/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Nikita Popov <nikic@crx.net>                                |
   |          Bob Weinand <bobwei9@hotmail.com>                           |
   +----------------------------------------------------------------------+
*/

#include "crex.h"
#include "crex_API.h"
#include "crex_interfaces.h"
#include "crex_exceptions.h"
#include "crex_generators.h"
#include "crex_closures.h"
#include "crex_generators_arginfo.h"
#include "crex_observer.h"

CREX_API crex_class_entry *crex_ce_generator;
CREX_API crex_class_entry *crex_ce_ClosedGeneratorException;
static crex_object_handlers crex_generator_handlers;

static crex_object *crex_generator_create(crex_class_entry *class_type);

CREX_API void crex_generator_restore_call_stack(crex_generator *generator) /* {{{ */
{
	crex_execute_data *call, *new_call, *prev_call = NULL;

	call = generator->frozen_call_stack;
	do {
		new_call = crex_vm_stack_push_call_frame(
			(CREX_CALL_INFO(call) & ~CREX_CALL_ALLOCATED),
			call->func,
			CREX_CALL_NUM_ARGS(call),
			C_PTR(call->This));
		memcpy(((zval*)new_call) + CREX_CALL_FRAME_SLOT, ((zval*)call) + CREX_CALL_FRAME_SLOT, CREX_CALL_NUM_ARGS(call) * sizeof(zval));
		new_call->extra_named_params = call->extra_named_params;
		new_call->prev_execute_data = prev_call;
		prev_call = new_call;

		call = call->prev_execute_data;
	} while (call);
	generator->execute_data->call = prev_call;
	efree(generator->frozen_call_stack);
	generator->frozen_call_stack = NULL;
}
/* }}} */

CREX_API crex_execute_data* crex_generator_freeze_call_stack(crex_execute_data *execute_data) /* {{{ */
{
	size_t used_stack;
	crex_execute_data *call, *new_call, *prev_call = NULL;
	zval *stack;

	/* calculate required stack size */
	used_stack = 0;
	call = EX(call);
	do {
		used_stack += CREX_CALL_FRAME_SLOT + CREX_CALL_NUM_ARGS(call);
		call = call->prev_execute_data;
	} while (call);

	stack = emalloc(used_stack * sizeof(zval));

	/* save stack, linking frames in reverse order */
	call = EX(call);
	do {
		size_t frame_size = CREX_CALL_FRAME_SLOT + CREX_CALL_NUM_ARGS(call);

		new_call = (crex_execute_data*)(stack + used_stack - frame_size);
		memcpy(new_call, call, frame_size * sizeof(zval));
		used_stack -= frame_size;
		new_call->prev_execute_data = prev_call;
		prev_call = new_call;

		new_call = call->prev_execute_data;
		crex_vm_stack_free_call_frame(call);
		call = new_call;
	} while (call);

	execute_data->call = NULL;
	CREX_ASSERT(prev_call == (crex_execute_data*)stack);

	return prev_call;
}
/* }}} */

static crex_execute_data* crex_generator_revert_call_stack(crex_execute_data *call)
{
	crex_execute_data *prev = NULL;

	do {
		crex_execute_data *next = call->prev_execute_data;
		call->prev_execute_data = prev;
		prev = call;
		call = next;
	} while (call);

	return prev;
}

static void crex_generator_cleanup_unfinished_execution(
		crex_generator *generator, crex_execute_data *execute_data, uint32_t catch_op_num) /* {{{ */
{
	crex_op_array *op_array = &execute_data->func->op_array;
	if (execute_data->opline != op_array->opcodes) {
		/* -1 required because we want the last run opcode, not the next to-be-run one. */
		uint32_t op_num = execute_data->opline - op_array->opcodes - 1;

		if (UNEXPECTED(generator->frozen_call_stack)) {
			/* Temporarily restore generator->execute_data if it has been NULLed out already. */
			crex_execute_data *save_ex = generator->execute_data;
			generator->execute_data = execute_data;
			crex_generator_restore_call_stack(generator);
			generator->execute_data = save_ex;
		}

		crex_cleanup_unfinished_execution(execute_data, op_num, catch_op_num);
	}
}
/* }}} */

CREX_API void crex_generator_close(crex_generator *generator, bool finished_execution) /* {{{ */
{
	if (EXPECTED(generator->execute_data)) {
		crex_execute_data *execute_data = generator->execute_data;
		/* Null out execute_data early, to prevent double frees if GC runs while we're
		 * already cleaning up execute_data. */
		generator->execute_data = NULL;

		if (EX_CALL_INFO() & CREX_CALL_HAS_SYMBOL_TABLE) {
			crex_clean_and_cache_symbol_table(execute_data->symbol_table);
		}
		/* always free the CV's, in the symtable are only not-free'd IS_INDIRECT's */
		crex_free_compiled_variables(execute_data);
		if (EX_CALL_INFO() & CREX_CALL_HAS_EXTRA_NAMED_PARAMS) {
			crex_free_extra_named_params(execute_data->extra_named_params);
		}

		if (EX_CALL_INFO() & CREX_CALL_RELEASE_THIS) {
			OBJ_RELEASE(C_OBJ(execute_data->This));
		}

		/* A fatal error / die occurred during the generator execution.
		 * Trying to clean up the stack may not be safe in this case. */
		if (UNEXPECTED(CG(unclean_shutdown))) {
			generator->execute_data = NULL;
			return;
		}

		crex_vm_stack_free_extra_args(execute_data);

		/* Some cleanups are only necessary if the generator was closed
		 * before it could finish execution (reach a return statement). */
		if (UNEXPECTED(!finished_execution)) {
			crex_generator_cleanup_unfinished_execution(generator, execute_data, 0);
		}

		/* Free closure object */
		if (EX_CALL_INFO() & CREX_CALL_CLOSURE) {
			OBJ_RELEASE(CREX_CLOSURE_OBJECT(EX(func)));
		}

		efree(execute_data);
	}
}
/* }}} */

static void crex_generator_remove_child(crex_generator_node *node, crex_generator *child)
{
	CREX_ASSERT(node->children >= 1);
	if (node->children == 1) {
		node->child.single = NULL;
	} else {
		HashTable *ht = node->child.ht;
		crex_hash_index_del(ht, (crex_ulong) child);
		if (node->children == 2) {
			crex_generator *other_child;
			CREX_HASH_FOREACH_PTR(ht, other_child) {
				node->child.single = other_child;
				break;
			} CREX_HASH_FOREACH_END();
			crex_hash_destroy(ht);
			efree(ht);
		}
	}
	node->children--;
}

static crex_always_inline crex_generator *clear_link_to_leaf(crex_generator *generator) {
	CREX_ASSERT(!generator->node.parent);
	crex_generator *leaf = generator->node.ptr.leaf;
	if (leaf) {
		leaf->node.ptr.root = NULL;
		generator->node.ptr.leaf = NULL;
		return leaf;
	}
	return NULL;
}

static crex_always_inline void clear_link_to_root(crex_generator *generator) {
	CREX_ASSERT(generator->node.parent);
	if (generator->node.ptr.root) {
		generator->node.ptr.root->node.ptr.leaf = NULL;
		generator->node.ptr.root = NULL;
	}
}

static void crex_generator_dtor_storage(crex_object *object) /* {{{ */
{
	crex_generator *generator = (crex_generator*) object;
	crex_execute_data *ex = generator->execute_data;
	uint32_t op_num, try_catch_offset;
	int i;

	/* Generator is running in a suspended fiber.
	 * Will be dtor during fiber dtor */
	if (crex_generator_get_current(generator)->flags & CREX_GENERATOR_IN_FIBER) {
		/* Prevent finally blocks from yielding */
		generator->flags |= CREX_GENERATOR_FORCED_CLOSE;
		return;
	}

	/* leave yield from mode to properly allow finally execution */
	if (UNEXPECTED(C_TYPE(generator->values) != IS_UNDEF)) {
		zval_ptr_dtor(&generator->values);
		ZVAL_UNDEF(&generator->values);
	}

	crex_generator *parent = generator->node.parent;
	if (parent) {
		crex_generator_remove_child(&parent->node, generator);
		clear_link_to_root(generator);
		generator->node.parent = NULL;
		OBJ_RELEASE(&parent->std);
	} else {
		clear_link_to_leaf(generator);
	}

	if (EXPECTED(!ex) || EXPECTED(!(ex->func->op_array.fn_flags & CREX_ACC_HAS_FINALLY_BLOCK))
			|| CG(unclean_shutdown)) {
		crex_generator_close(generator, 0);
		return;
	}

	/* -1 required because we want the last run opcode, not the
	 * next to-be-run one. */
	op_num = ex->opline - ex->func->op_array.opcodes - 1;
	try_catch_offset = -1;

	/* Find the innermost try/catch that we are inside of. */
	for (i = 0; i < ex->func->op_array.last_try_catch; i++) {
		crex_try_catch_element *try_catch = &ex->func->op_array.try_catch_array[i];
		if (op_num < try_catch->try_op) {
			break;
		}
		if (op_num < try_catch->catch_op || op_num < try_catch->finally_end) {
			try_catch_offset = i;
		}
	}

	/* Walk try/catch/finally structures upwards, performing the necessary actions. */
	while (try_catch_offset != (uint32_t) -1) {
		crex_try_catch_element *try_catch = &ex->func->op_array.try_catch_array[try_catch_offset];

		if (op_num < try_catch->finally_op) {
			/* Go to finally block */
			zval *fast_call =
				CREX_CALL_VAR(ex, ex->func->op_array.opcodes[try_catch->finally_end].op1.var);

			crex_generator_cleanup_unfinished_execution(generator, ex, try_catch->finally_op);
			crex_object *old_exception = EG(exception);
			const crex_op *old_opline_before_exception = EG(opline_before_exception);
			EG(exception) = NULL;
			C_OBJ_P(fast_call) = NULL;
			C_OPLINE_NUM_P(fast_call) = (uint32_t)-1;

			ex->opline = &ex->func->op_array.opcodes[try_catch->finally_op];
			generator->flags |= CREX_GENERATOR_FORCED_CLOSE;
			crex_generator_resume(generator);

			if (old_exception) {
				EG(opline_before_exception) = old_opline_before_exception;
				if (EG(exception)) {
					crex_exception_set_previous(EG(exception), old_exception);
				} else {
					EG(exception) = old_exception;
				}
			}

			/* TODO: If we hit another yield inside try/finally,
			 * should we also jump to the next finally block? */
			break;
		} else if (op_num < try_catch->finally_end) {
			zval *fast_call =
				CREX_CALL_VAR(ex, ex->func->op_array.opcodes[try_catch->finally_end].op1.var);
			/* Clean up incomplete return statement */
			if (C_OPLINE_NUM_P(fast_call) != (uint32_t) -1) {
				crex_op *retval_op = &ex->func->op_array.opcodes[C_OPLINE_NUM_P(fast_call)];
				if (retval_op->op2_type & (IS_TMP_VAR | IS_VAR)) {
					zval_ptr_dtor(CREX_CALL_VAR(ex, retval_op->op2.var));
				}
			}
			/* Clean up backed-up exception */
			if (C_OBJ_P(fast_call)) {
				OBJ_RELEASE(C_OBJ_P(fast_call));
			}
		}

		try_catch_offset--;
	}

	crex_generator_close(generator, 0);
}
/* }}} */

static void crex_generator_free_storage(crex_object *object) /* {{{ */
{
	crex_generator *generator = (crex_generator*) object;

	crex_generator_close(generator, 0);

	/* we can't immediately free them in crex_generator_close() else yield from won't be able to fetch it */
	zval_ptr_dtor(&generator->value);
	zval_ptr_dtor(&generator->key);

	if (EXPECTED(!C_ISUNDEF(generator->retval))) {
		zval_ptr_dtor(&generator->retval);
	}

	if (UNEXPECTED(generator->node.children > 1)) {
		crex_hash_destroy(generator->node.child.ht);
		efree(generator->node.child.ht);
	}

	crex_object_std_dtor(&generator->std);
}
/* }}} */

static HashTable *crex_generator_get_gc(crex_object *object, zval **table, int *n) /* {{{ */
{
	crex_generator *generator = (crex_generator*)object;
	crex_execute_data *execute_data = generator->execute_data;
	crex_execute_data *call = NULL;

	if (!execute_data) {
		/* If the generator has been closed, it can only hold on to three values: The value, key
		 * and retval. These three zvals are stored sequentially starting at &generator->value. */
		*table = &generator->value;
		*n = 3;
		return NULL;
	}

	if (generator->flags & CREX_GENERATOR_CURRENTLY_RUNNING) {
		/* If the generator is currently running, we certainly won't be able to GC any values it
		 * holds on to. The execute_data state might be inconsistent during execution (e.g. because
		 * GC has been triggered in the middle of a variable reassignment), so we should not try
		 * to inspect it here. */
		*table = NULL;
		*n = 0;
		return NULL;
	}


	crex_get_gc_buffer *gc_buffer = crex_get_gc_buffer_create();
	crex_get_gc_buffer_add_zval(gc_buffer, &generator->value);
	crex_get_gc_buffer_add_zval(gc_buffer, &generator->key);
	crex_get_gc_buffer_add_zval(gc_buffer, &generator->retval);
	crex_get_gc_buffer_add_zval(gc_buffer, &generator->values);

	if (UNEXPECTED(generator->frozen_call_stack)) {
		/* The frozen stack is linked in reverse order */
		call = crex_generator_revert_call_stack(generator->frozen_call_stack);
	}

	crex_unfinished_execution_gc_ex(execute_data, call, gc_buffer, true);

	if (UNEXPECTED(generator->frozen_call_stack)) {
		crex_generator_revert_call_stack(call);
	}

	if (generator->node.parent) {
		crex_get_gc_buffer_add_obj(gc_buffer, &generator->node.parent->std);
	}

	crex_get_gc_buffer_use(gc_buffer, table, n);
	if (EX_CALL_INFO() & CREX_CALL_HAS_SYMBOL_TABLE) {
		return execute_data->symbol_table;
	} else {
		return NULL;
	}
}
/* }}} */

static crex_object *crex_generator_create(crex_class_entry *class_type) /* {{{ */
{
	crex_generator *generator = emalloc(sizeof(crex_generator));
	memset(generator, 0, sizeof(crex_generator));

	/* The key will be incremented on first use, so it'll start at 0 */
	generator->largest_used_integer_key = -1;

	ZVAL_UNDEF(&generator->retval);
	ZVAL_UNDEF(&generator->values);

	/* By default we have a tree of only one node */
	generator->node.parent = NULL;
	generator->node.children = 0;
	generator->node.ptr.root = NULL;

	crex_object_std_init(&generator->std, class_type);
	return (crex_object*)generator;
}
/* }}} */

static CREX_COLD crex_function *crex_generator_get_constructor(crex_object *object) /* {{{ */
{
	crex_throw_error(NULL, "The \"Generator\" class is reserved for internal use and cannot be manually instantiated");

	return NULL;
}
/* }}} */

CREX_API crex_execute_data *crex_generator_check_placeholder_frame(crex_execute_data *ptr)
{
	if (!ptr->func && C_TYPE(ptr->This) == IS_OBJECT) {
		if (C_OBJCE(ptr->This) == crex_ce_generator) {
			crex_generator *generator = (crex_generator *) C_OBJ(ptr->This);
			crex_execute_data *prev = ptr->prev_execute_data;
			CREX_ASSERT(generator->node.parent && "Placeholder only used with delegation");
			while (generator->node.parent->node.parent) {
				generator->execute_data->prev_execute_data = prev;
				prev = generator->execute_data;
				generator = generator->node.parent;
			}
			generator->execute_data->prev_execute_data = prev;
			ptr = generator->execute_data;
		}
	}
	return ptr;
}

static void crex_generator_throw_exception(crex_generator *generator, zval *exception)
{
	crex_execute_data *original_execute_data = EG(current_execute_data);

	/* Throw the exception in the context of the generator. Decrementing the opline
	 * to pretend the exception happened during the YIELD opcode. */
	EG(current_execute_data) = generator->execute_data;
	generator->execute_data->opline--;

	if (exception) {
		crex_throw_exception_object(exception);
	} else {
		crex_rethrow_exception(EG(current_execute_data));
	}

	/* if we don't stop an array/iterator yield from, the exception will only reach the generator after the values were all iterated over */
	if (UNEXPECTED(C_TYPE(generator->values) != IS_UNDEF)) {
		zval_ptr_dtor(&generator->values);
		ZVAL_UNDEF(&generator->values);
	}

	generator->execute_data->opline++;
	EG(current_execute_data) = original_execute_data;
}

static void crex_generator_add_child(crex_generator *generator, crex_generator *child)
{
	crex_generator_node *node = &generator->node;

	if (node->children == 0) {
		node->child.single = child;
	} else {
		if (node->children == 1) {
			HashTable *ht = emalloc(sizeof(HashTable));
			crex_hash_init(ht, 0, NULL, NULL, 0);
			crex_hash_index_add_new_ptr(ht,
				(crex_ulong) node->child.single, node->child.single);
			node->child.ht = ht;
		}

		crex_hash_index_add_new_ptr(node->child.ht, (crex_ulong) child, child);
	}

	++node->children;
}

void crex_generator_yield_from(crex_generator *generator, crex_generator *from)
{
	CREX_ASSERT(!generator->node.parent && "Already has parent?");
	crex_generator *leaf = clear_link_to_leaf(generator);
	if (leaf && !from->node.parent && !from->node.ptr.leaf) {
		from->node.ptr.leaf = leaf;
		leaf->node.ptr.root = from;
	}
	generator->node.parent = from;
	crex_generator_add_child(from, generator);
	generator->flags |= CREX_GENERATOR_DO_INIT;
}

CREX_API crex_generator *crex_generator_update_root(crex_generator *generator)
{
	crex_generator *root = generator->node.parent;
	while (root->node.parent) {
		root = root->node.parent;
	}

	clear_link_to_leaf(root);
	root->node.ptr.leaf = generator;
	generator->node.ptr.root = root;
	return root;
}

static crex_generator *get_new_root(crex_generator *generator, crex_generator *root)
{
	while (!root->execute_data && root->node.children == 1) {
		root = root->node.child.single;
	}

	if (root->execute_data) {
		return root;
	}

	/* We have reached a multi-child node haven't found the root yet. We don't know which
	 * child to follow, so perform the search from the other direction instead. */
	while (generator->node.parent->execute_data) {
		generator = generator->node.parent;
	}

	return generator;
}

CREX_API crex_generator *crex_generator_update_current(crex_generator *generator)
{
	crex_generator *old_root = generator->node.ptr.root;
	CREX_ASSERT(!old_root->execute_data && "Nothing to update?");

	crex_generator *new_root = get_new_root(generator, old_root);

	CREX_ASSERT(old_root->node.ptr.leaf == generator);
	generator->node.ptr.root = new_root;
	new_root->node.ptr.leaf = generator;
	old_root->node.ptr.leaf = NULL;

	crex_generator *new_root_parent = new_root->node.parent;
	CREX_ASSERT(new_root_parent);
	crex_generator_remove_child(&new_root_parent->node, new_root);

	if (EXPECTED(EG(exception) == NULL) && EXPECTED((OBJ_FLAGS(&generator->std) & IS_OBJ_DESTRUCTOR_CALLED) == 0)) {
		crex_op *yield_from = (crex_op *) new_root->execute_data->opline - 1;

		if (yield_from->opcode == CREX_YIELD_FROM) {
			if (C_ISUNDEF(new_root_parent->retval)) {
				/* Throw the exception in the context of the generator */
				crex_execute_data *original_execute_data = EG(current_execute_data);
				EG(current_execute_data) = new_root->execute_data;

				if (new_root == generator) {
					new_root->execute_data->prev_execute_data = original_execute_data;
				} else {
					new_root->execute_data->prev_execute_data = &generator->execute_fake;
					generator->execute_fake.prev_execute_data = original_execute_data;
				}

				/* CREX_YIELD(_FROM) already advance, so decrement opline to throw from correct place */
				new_root->execute_data->opline--;
				crex_throw_exception(crex_ce_ClosedGeneratorException, "Generator yielded from aborted, no return value available", 0);

				EG(current_execute_data) = original_execute_data;

				if (!((old_root ? old_root : generator)->flags & CREX_GENERATOR_CURRENTLY_RUNNING)) {
					new_root->node.parent = NULL;
					OBJ_RELEASE(&new_root_parent->std);
					crex_generator_resume(generator);
					return crex_generator_get_current(generator);
				}
			} else {
				zval_ptr_dtor(&new_root->value);
				ZVAL_COPY(&new_root->value, &new_root_parent->value);
				ZVAL_COPY(CREX_CALL_VAR(new_root->execute_data, yield_from->result.var), &new_root_parent->retval);
			}
		}
	}

	new_root->node.parent = NULL;
	OBJ_RELEASE(&new_root_parent->std);

	return new_root;
}

static crex_result crex_generator_get_next_delegated_value(crex_generator *generator) /* {{{ */
{
	--generator->execute_data->opline;

	zval *value;
	if (C_TYPE(generator->values) == IS_ARRAY) {
		HashTable *ht = C_ARR(generator->values);
		HashPosition pos = C_FE_POS(generator->values);

		if (HT_IS_PACKED(ht)) {
			do {
				if (UNEXPECTED(pos >= ht->nNumUsed)) {
					/* Reached end of array */
					goto failure;
				}

				value = &ht->arPacked[pos];
				pos++;
			} while (C_ISUNDEF_P(value));

			zval_ptr_dtor(&generator->value);
			ZVAL_COPY(&generator->value, value);

			zval_ptr_dtor(&generator->key);
			ZVAL_LONG(&generator->key, pos - 1);
		} else {
			Bucket *p;

			do {
				if (UNEXPECTED(pos >= ht->nNumUsed)) {
					/* Reached end of array */
					goto failure;
				}

				p = &ht->arData[pos];
				value = &p->val;
				pos++;
			} while (C_ISUNDEF_P(value));

			zval_ptr_dtor(&generator->value);
			ZVAL_COPY(&generator->value, value);

			zval_ptr_dtor(&generator->key);
			if (p->key) {
				ZVAL_STR_COPY(&generator->key, p->key);
			} else {
				ZVAL_LONG(&generator->key, p->h);
			}
		}
		C_FE_POS(generator->values) = pos;
	} else {
		crex_object_iterator *iter = (crex_object_iterator *) C_OBJ(generator->values);

		if (iter->index++ > 0) {
			iter->funcs->move_forward(iter);
			if (UNEXPECTED(EG(exception) != NULL)) {
				goto failure;
			}
		}

		if (iter->funcs->valid(iter) == FAILURE) {
			/* reached end of iteration */
			goto failure;
		}

		value = iter->funcs->get_current_data(iter);
		if (UNEXPECTED(EG(exception) != NULL) || UNEXPECTED(!value)) {
			goto failure;
		}

		zval_ptr_dtor(&generator->value);
		ZVAL_COPY(&generator->value, value);

		zval_ptr_dtor(&generator->key);
		if (iter->funcs->get_current_key) {
			iter->funcs->get_current_key(iter, &generator->key);
			if (UNEXPECTED(EG(exception) != NULL)) {
				ZVAL_UNDEF(&generator->key);
				goto failure;
			}
		} else {
			ZVAL_LONG(&generator->key, iter->index);
		}
	}

	++generator->execute_data->opline;
	return SUCCESS;

failure:
	zval_ptr_dtor(&generator->values);
	ZVAL_UNDEF(&generator->values);

	++generator->execute_data->opline;
	return FAILURE;
}
/* }}} */

CREX_API void crex_generator_resume(crex_generator *orig_generator) /* {{{ */
{
	crex_generator *generator = crex_generator_get_current(orig_generator);

	/* The generator is already closed, thus can't resume */
	if (UNEXPECTED(!generator->execute_data)) {
		return;
	}

try_again:
	if (generator->flags & CREX_GENERATOR_CURRENTLY_RUNNING) {
		crex_throw_error(NULL, "Cannot resume an already running generator");
		return;
	}

	if (UNEXPECTED((orig_generator->flags & CREX_GENERATOR_DO_INIT) != 0 && !C_ISUNDEF(generator->value))) {
		/* We must not advance Generator if we yield from a Generator being currently run */
		orig_generator->flags &= ~CREX_GENERATOR_DO_INIT;
		return;
	}

	/* Drop the AT_FIRST_YIELD flag */
	orig_generator->flags &= ~CREX_GENERATOR_AT_FIRST_YIELD;

	/* Backup executor globals */
	crex_execute_data *original_execute_data = EG(current_execute_data);
	uint32_t original_jit_trace_num = EG(jit_trace_num);

	/* Set executor globals */
	EG(current_execute_data) = generator->execute_data;
	EG(jit_trace_num) = 0;

	/* We want the backtrace to look as if the generator function was
	 * called from whatever method we are current running (e.g. next()).
	 * So we have to link generator call frame with caller call frame. */
	if (generator == orig_generator) {
		generator->execute_data->prev_execute_data = original_execute_data;
	} else {
		/* We need some execute_data placeholder in stacktrace to be replaced
		 * by the real stack trace when needed */
		generator->execute_data->prev_execute_data = &orig_generator->execute_fake;
		orig_generator->execute_fake.prev_execute_data = original_execute_data;
	}

	/* Ensure this is run after executor_data swap to have a proper stack trace */
	if (UNEXPECTED(!C_ISUNDEF(generator->values))) {
		if (EXPECTED(crex_generator_get_next_delegated_value(generator) == SUCCESS)) {
			/* Restore executor globals */
			EG(current_execute_data) = original_execute_data;
			EG(jit_trace_num) = original_jit_trace_num;

			orig_generator->flags &= ~CREX_GENERATOR_DO_INIT;
			return;
		}
		/* If there are no more delegated values, resume the generator
		 * after the "yield from" expression. */
	}

	if (UNEXPECTED(generator->frozen_call_stack)) {
		/* Restore frozen call-stack */
		crex_generator_restore_call_stack(generator);
	}

	/* Resume execution */
	generator->flags |= CREX_GENERATOR_CURRENTLY_RUNNING
						| (EG(active_fiber) ? CREX_GENERATOR_IN_FIBER : 0);
	if (!CREX_OBSERVER_ENABLED) {
		crex_execute_ex(generator->execute_data);
	} else {
		crex_observer_generator_resume(generator->execute_data);
		crex_execute_ex(generator->execute_data);
		if (generator->execute_data) {
			/* On the final return, this will be called from CREX_GENERATOR_RETURN */
			crex_observer_fcall_end(generator->execute_data, &generator->value);
		}
	}
	generator->flags &= ~(CREX_GENERATOR_CURRENTLY_RUNNING | CREX_GENERATOR_IN_FIBER);

	generator->frozen_call_stack = NULL;
	if (EXPECTED(generator->execute_data) &&
		UNEXPECTED(generator->execute_data->call)) {
		/* Frize call-stack */
		generator->frozen_call_stack = crex_generator_freeze_call_stack(generator->execute_data);
	}

	/* Restore executor globals */
	EG(current_execute_data) = original_execute_data;
	EG(jit_trace_num) = original_jit_trace_num;

	/* If an exception was thrown in the generator we have to internally
	 * rethrow it in the parent scope.
	 * In case we did yield from, the Exception must be rethrown into
	 * its calling frame (see above in if (check_yield_from). */
	if (UNEXPECTED(EG(exception) != NULL)) {
		if (generator == orig_generator) {
			crex_generator_close(generator, 0);
			if (!EG(current_execute_data)) {
				crex_throw_exception_internal(NULL);
			} else if (EG(current_execute_data)->func &&
					CREX_USER_CODE(EG(current_execute_data)->func->common.type)) {
				crex_rethrow_exception(EG(current_execute_data));
			}
		} else {
			generator = crex_generator_get_current(orig_generator);
			crex_generator_throw_exception(generator, NULL);
			orig_generator->flags &= ~CREX_GENERATOR_DO_INIT;
			goto try_again;
		}
	}

	/* yield from was used, try another resume. */
	if (UNEXPECTED((generator != orig_generator && !C_ISUNDEF(generator->retval)) || (generator->execute_data && (generator->execute_data->opline - 1)->opcode == CREX_YIELD_FROM))) {
		generator = crex_generator_get_current(orig_generator);
		goto try_again;
	}

	orig_generator->flags &= ~CREX_GENERATOR_DO_INIT;
}
/* }}} */

static inline void crex_generator_ensure_initialized(crex_generator *generator) /* {{{ */
{
	if (UNEXPECTED(C_TYPE(generator->value) == IS_UNDEF) && EXPECTED(generator->execute_data) && EXPECTED(generator->node.parent == NULL)) {
		crex_generator_resume(generator);
		generator->flags |= CREX_GENERATOR_AT_FIRST_YIELD;
	}
}
/* }}} */

static inline void crex_generator_rewind(crex_generator *generator) /* {{{ */
{
	crex_generator_ensure_initialized(generator);

	if (!(generator->flags & CREX_GENERATOR_AT_FIRST_YIELD)) {
		crex_throw_exception(NULL, "Cannot rewind a generator that was already run", 0);
	}
}
/* }}} */

/* {{{ Rewind the generator */
CREX_METHOD(Generator, rewind)
{
	crex_generator *generator;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_rewind(generator);
}
/* }}} */

/* {{{ Check whether the generator is valid */
CREX_METHOD(Generator, valid)
{
	crex_generator *generator;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	crex_generator_get_current(generator);

	RETURN_BOOL(EXPECTED(generator->execute_data != NULL));
}
/* }}} */

/* {{{ Get the current value */
CREX_METHOD(Generator, current)
{
	crex_generator *generator, *root;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	root = crex_generator_get_current(generator);
	if (EXPECTED(generator->execute_data != NULL && C_TYPE(root->value) != IS_UNDEF)) {
		RETURN_COPY_DEREF(&root->value);
	}
}
/* }}} */

/* {{{ Get the current key */
CREX_METHOD(Generator, key)
{
	crex_generator *generator, *root;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	root = crex_generator_get_current(generator);
	if (EXPECTED(generator->execute_data != NULL && C_TYPE(root->key) != IS_UNDEF)) {
		RETURN_COPY_DEREF(&root->key);
	}
}
/* }}} */

/* {{{ Advances the generator */
CREX_METHOD(Generator, next)
{
	crex_generator *generator;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	crex_generator_resume(generator);
}
/* }}} */

/* {{{ Sends a value to the generator */
CREX_METHOD(Generator, send)
{
	zval *value;
	crex_generator *generator, *root;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_ZVAL(value)
	CREX_PARSE_PARAMETERS_END();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	/* The generator is already closed, thus can't send anything */
	if (UNEXPECTED(!generator->execute_data)) {
		return;
	}

	root = crex_generator_get_current(generator);
	/* Put sent value in the target VAR slot, if it is used */
	if (root->send_target && !(root->flags & CREX_GENERATOR_CURRENTLY_RUNNING)) {
		ZVAL_COPY(root->send_target, value);
	}

	crex_generator_resume(generator);

	root = crex_generator_get_current(generator);
	if (EXPECTED(generator->execute_data)) {
		RETURN_COPY_DEREF(&root->value);
	}
}
/* }}} */

/* {{{ Throws an exception into the generator */
CREX_METHOD(Generator, throw)
{
	zval *exception;
	crex_generator *generator;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_OBJECT_OF_CLASS(exception, crex_ce_throwable);
	CREX_PARSE_PARAMETERS_END();

	C_TRY_ADDREF_P(exception);

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);

	if (generator->execute_data) {
		crex_generator *root = crex_generator_get_current(generator);

		crex_generator_throw_exception(root, exception);

		crex_generator_resume(generator);

		root = crex_generator_get_current(generator);
		if (generator->execute_data) {
			RETURN_COPY_DEREF(&root->value);
		}
	} else {
		/* If the generator is already closed throw the exception in the
		 * current context */
		crex_throw_exception_object(exception);
	}
}
/* }}} */

/* {{{ Retrieves the return value of the generator */
CREX_METHOD(Generator, getReturn)
{
	crex_generator *generator;

	CREX_PARSE_PARAMETERS_NONE();

	generator = (crex_generator *) C_OBJ_P(CREX_THIS);

	crex_generator_ensure_initialized(generator);
	if (UNEXPECTED(EG(exception))) {
		return;
	}

	if (C_ISUNDEF(generator->retval)) {
		/* Generator hasn't returned yet -> error! */
		crex_throw_exception(NULL,
			"Cannot get return value of a generator that hasn't returned", 0);
		return;
	}

	ZVAL_COPY(return_value, &generator->retval);
}
/* }}} */

/* get_iterator implementation */

static void crex_generator_iterator_dtor(crex_object_iterator *iterator) /* {{{ */
{
	zval_ptr_dtor(&iterator->data);
}
/* }}} */

static int crex_generator_iterator_valid(crex_object_iterator *iterator) /* {{{ */
{
	crex_generator *generator = (crex_generator*)C_OBJ(iterator->data);

	crex_generator_ensure_initialized(generator);

	crex_generator_get_current(generator);

	return generator->execute_data ? SUCCESS : FAILURE;
}
/* }}} */

static zval *crex_generator_iterator_get_data(crex_object_iterator *iterator) /* {{{ */
{
	crex_generator *generator = (crex_generator*)C_OBJ(iterator->data), *root;

	crex_generator_ensure_initialized(generator);

	root = crex_generator_get_current(generator);

	return &root->value;
}
/* }}} */

static void crex_generator_iterator_get_key(crex_object_iterator *iterator, zval *key) /* {{{ */
{
	crex_generator *generator = (crex_generator*)C_OBJ(iterator->data), *root;

	crex_generator_ensure_initialized(generator);

	root = crex_generator_get_current(generator);

	if (EXPECTED(C_TYPE(root->key) != IS_UNDEF)) {
		zval *zv = &root->key;

		ZVAL_COPY_DEREF(key, zv);
	} else {
		ZVAL_NULL(key);
	}
}
/* }}} */

static void crex_generator_iterator_move_forward(crex_object_iterator *iterator) /* {{{ */
{
	crex_generator *generator = (crex_generator*)C_OBJ(iterator->data);

	crex_generator_ensure_initialized(generator);

	crex_generator_resume(generator);
}
/* }}} */

static void crex_generator_iterator_rewind(crex_object_iterator *iterator) /* {{{ */
{
	crex_generator *generator = (crex_generator*)C_OBJ(iterator->data);

	crex_generator_rewind(generator);
}
/* }}} */

static HashTable *crex_generator_iterator_get_gc(
		crex_object_iterator *iterator, zval **table, int *n)
{
	*table = &iterator->data;
	*n = 1;
	return NULL;
}

static const crex_object_iterator_funcs crex_generator_iterator_functions = {
	crex_generator_iterator_dtor,
	crex_generator_iterator_valid,
	crex_generator_iterator_get_data,
	crex_generator_iterator_get_key,
	crex_generator_iterator_move_forward,
	crex_generator_iterator_rewind,
	NULL,
	crex_generator_iterator_get_gc,
};

/* by_ref is int due to Iterator API */
crex_object_iterator *crex_generator_get_iterator(crex_class_entry *ce, zval *object, int by_ref) /* {{{ */
{
	crex_object_iterator *iterator;
	crex_generator *generator = (crex_generator*)C_OBJ_P(object);

	if (!generator->execute_data) {
		crex_throw_exception(NULL, "Cannot traverse an already closed generator", 0);
		return NULL;
	}

	if (UNEXPECTED(by_ref) && !(generator->execute_data->func->op_array.fn_flags & CREX_ACC_RETURN_REFERENCE)) {
		crex_throw_exception(NULL, "You can only iterate a generator by-reference if it declared that it yields by-reference", 0);
		return NULL;
	}

	iterator = emalloc(sizeof(crex_object_iterator));
	crex_iterator_init(iterator);

	iterator->funcs = &crex_generator_iterator_functions;
	ZVAL_OBJ_COPY(&iterator->data, C_OBJ_P(object));

	return iterator;
}
/* }}} */

void crex_register_generator_ce(void) /* {{{ */
{
	crex_ce_generator = register_class_Generator(crex_ce_iterator);
	crex_ce_generator->create_object = crex_generator_create;
	/* get_iterator has to be assigned *after* implementing the interface */
	crex_ce_generator->get_iterator = crex_generator_get_iterator;
	crex_ce_generator->default_object_handlers = &crex_generator_handlers;

	memcpy(&crex_generator_handlers, &std_object_handlers, sizeof(crex_object_handlers));
	crex_generator_handlers.free_obj = crex_generator_free_storage;
	crex_generator_handlers.dtor_obj = crex_generator_dtor_storage;
	crex_generator_handlers.get_gc = crex_generator_get_gc;
	crex_generator_handlers.clone_obj = NULL;
	crex_generator_handlers.get_constructor = crex_generator_get_constructor;

	crex_ce_ClosedGeneratorException = register_class_ClosedGeneratorException(crex_ce_exception);
}
/* }}} */
