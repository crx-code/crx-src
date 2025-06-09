/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: c01ba2e6ccaad6db28cc6f42d679dc753be57187 */

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_EmptyIterator_current, 0, 0, IS_NEVER, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_EmptyIterator_next, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_EmptyIterator_key arginfo_class_EmptyIterator_current

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_EmptyIterator_valid, 0, 0, IS_FALSE, 0)
CREX_END_ARG_INFO()

#define arginfo_class_EmptyIterator_rewind arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CallbackFilterIterator___main, 0, 0, 2)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CallbackFilterIterator_accept, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveCallbackFilterIterator___main, 0, 0, 2)
	CREX_ARG_OBJ_INFO(0, iterator, RecursiveIterator, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveCallbackFilterIterator_hasChildren arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveCallbackFilterIterator_getChildren, 0, 0, RecursiveCallbackFilterIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveIterator_hasChildren arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveIterator_getChildren, 0, 0, RecursiveIterator, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveIteratorIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Traversable, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "RecursiveIteratorIterator::LEAVES_ONLY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveIteratorIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_RecursiveIteratorIterator_valid arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveIteratorIterator_key, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveIteratorIterator_current arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_RecursiveIteratorIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveIteratorIterator_getDepth, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveIteratorIterator_getSubIterator, 0, 0, RecursiveIterator, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveIteratorIterator_getInnerIterator, 0, 0, RecursiveIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveIteratorIterator_beginIteration arginfo_class_EmptyIterator_next

#define arginfo_class_RecursiveIteratorIterator_endIteration arginfo_class_EmptyIterator_next

#define arginfo_class_RecursiveIteratorIterator_callHasChildren arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_RecursiveIteratorIterator_callGetChildren arginfo_class_RecursiveIterator_getChildren

#define arginfo_class_RecursiveIteratorIterator_beginChildren arginfo_class_EmptyIterator_next

#define arginfo_class_RecursiveIteratorIterator_endChildren arginfo_class_EmptyIterator_next

#define arginfo_class_RecursiveIteratorIterator_nextElement arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveIteratorIterator_setMaxDepth, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxDepth, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_RecursiveIteratorIterator_getMaxDepth, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_OuterIterator_getInnerIterator, 0, 0, Iterator, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_IteratorIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Traversable, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_IteratorIterator_getInnerIterator arginfo_class_OuterIterator_getInnerIterator

#define arginfo_class_IteratorIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_IteratorIterator_valid arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_IteratorIterator_key arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_IteratorIterator_current arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_IteratorIterator_next arginfo_class_EmptyIterator_next

#define arginfo_class_FilterIterator_accept arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_INFO_EX(arginfo_class_FilterIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_FilterIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_FilterIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveFilterIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, RecursiveIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveFilterIterator_hasChildren arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveFilterIterator_getChildren, 0, 0, RecursiveFilterIterator, 1)
CREX_END_ARG_INFO()

#define arginfo_class_ParentIterator___main arginfo_class_RecursiveFilterIterator___main

#define arginfo_class_ParentIterator_accept arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SeekableIterator_seek, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_LimitIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limit, IS_LONG, 0, "-1")
CREX_END_ARG_INFO()

#define arginfo_class_LimitIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_LimitIterator_valid arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_LimitIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_LimitIterator_seek, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_LimitIterator_getPosition arginfo_class_RecursiveIteratorIterator_getDepth

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CachingIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "CachingIterator::CALL_TOSTRING")
CREX_END_ARG_INFO()

#define arginfo_class_CachingIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_CachingIterator_valid arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_CachingIterator_next arginfo_class_EmptyIterator_next

#define arginfo_class_CachingIterator_hasNext arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CachingIterator_getFlags arginfo_class_RecursiveIteratorIterator_getDepth

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_setFlags, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_offsetGet, 0, 1, IS_MIXED, 0)
	CREX_ARG_INFO(0, key)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, key)
	CREX_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, key)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, key)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CachingIterator_getCache, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CachingIterator_count arginfo_class_RecursiveIteratorIterator_getDepth

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveCachingIterator___main, 0, 0, 1)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "RecursiveCachingIterator::CALL_TOSTRING")
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveCachingIterator_hasChildren arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveCachingIterator_getChildren, 0, 0, RecursiveCachingIterator, 1)
CREX_END_ARG_INFO()

#define arginfo_class_NoRewindIterator___main arginfo_class_FilterIterator___main

#define arginfo_class_NoRewindIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_NoRewindIterator_valid arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_NoRewindIterator_key arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_NoRewindIterator_current arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_NoRewindIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_INFO_EX(arginfo_class_AppendIterator___main, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_AppendIterator_append, 0, 1, IS_VOID, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_AppendIterator_rewind arginfo_class_EmptyIterator_next

#define arginfo_class_AppendIterator_valid arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_AppendIterator_current arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_AppendIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_AppendIterator_getIteratorIndex, 0, 0, IS_LONG, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_AppendIterator_getArrayIterator, 0, 0, ArrayIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_InfiniteIterator___main arginfo_class_FilterIterator___main

#define arginfo_class_InfiniteIterator_next arginfo_class_EmptyIterator_next

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RegexIterator___main, 0, 0, 2)
	CREX_ARG_OBJ_INFO(0, iterator, Iterator, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "RegexIterator::MATCH")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pregFlags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_RegexIterator_accept arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_RegexIterator_getMode arginfo_class_RecursiveIteratorIterator_getDepth

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RegexIterator_setMode, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RegexIterator_getFlags arginfo_class_RecursiveIteratorIterator_getDepth

#define arginfo_class_RegexIterator_setFlags arginfo_class_CachingIterator_setFlags

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RegexIterator_getRegex, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RegexIterator_getPregFlags arginfo_class_RecursiveIteratorIterator_getDepth

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RegexIterator_setPregFlags, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, pregFlags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveRegexIterator___main, 0, 0, 2)
	CREX_ARG_OBJ_INFO(0, iterator, RecursiveIterator, 0)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "RecursiveRegexIterator::MATCH")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pregFlags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveRegexIterator_accept arginfo_class_CallbackFilterIterator_accept

#define arginfo_class_RecursiveRegexIterator_hasChildren arginfo_class_CallbackFilterIterator_accept

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveRegexIterator_getChildren, 0, 0, RecursiveRegexIterator, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveTreeIterator___main, 0, 0, 1)
	CREX_ARG_INFO(0, iterator)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "RecursiveTreeIterator::BYPASS_KEY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cachingIteratorFlags, IS_LONG, 0, "CachingIterator::CATCH_GET_CHILD")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "RecursiveTreeIterator::SELF_FIRST")
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveTreeIterator_key arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_RecursiveTreeIterator_current arginfo_class_RecursiveIteratorIterator_key

#define arginfo_class_RecursiveTreeIterator_getPrefix arginfo_class_RegexIterator_getRegex

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveTreeIterator_setPostfix, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, postfix, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveTreeIterator_setPrefixPart, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, part, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveTreeIterator_getEntry arginfo_class_RegexIterator_getRegex

#define arginfo_class_RecursiveTreeIterator_getPostfix arginfo_class_RegexIterator_getRegex


CREX_METHOD(EmptyIterator, current);
CREX_METHOD(EmptyIterator, next);
CREX_METHOD(EmptyIterator, key);
CREX_METHOD(EmptyIterator, valid);
CREX_METHOD(EmptyIterator, rewind);
CREX_METHOD(CallbackFilterIterator, __main);
CREX_METHOD(CallbackFilterIterator, accept);
CREX_METHOD(RecursiveCallbackFilterIterator, __main);
CREX_METHOD(RecursiveFilterIterator, hasChildren);
CREX_METHOD(RecursiveCallbackFilterIterator, getChildren);
CREX_METHOD(RecursiveIteratorIterator, __main);
CREX_METHOD(RecursiveIteratorIterator, rewind);
CREX_METHOD(RecursiveIteratorIterator, valid);
CREX_METHOD(RecursiveIteratorIterator, key);
CREX_METHOD(RecursiveIteratorIterator, current);
CREX_METHOD(RecursiveIteratorIterator, next);
CREX_METHOD(RecursiveIteratorIterator, getDepth);
CREX_METHOD(RecursiveIteratorIterator, getSubIterator);
CREX_METHOD(RecursiveIteratorIterator, getInnerIterator);
CREX_METHOD(RecursiveIteratorIterator, beginIteration);
CREX_METHOD(RecursiveIteratorIterator, endIteration);
CREX_METHOD(RecursiveIteratorIterator, callHasChildren);
CREX_METHOD(RecursiveIteratorIterator, callGetChildren);
CREX_METHOD(RecursiveIteratorIterator, beginChildren);
CREX_METHOD(RecursiveIteratorIterator, endChildren);
CREX_METHOD(RecursiveIteratorIterator, nextElement);
CREX_METHOD(RecursiveIteratorIterator, setMaxDepth);
CREX_METHOD(RecursiveIteratorIterator, getMaxDepth);
CREX_METHOD(IteratorIterator, __main);
CREX_METHOD(IteratorIterator, getInnerIterator);
CREX_METHOD(IteratorIterator, rewind);
CREX_METHOD(IteratorIterator, valid);
CREX_METHOD(IteratorIterator, key);
CREX_METHOD(IteratorIterator, current);
CREX_METHOD(IteratorIterator, next);
CREX_METHOD(FilterIterator, __main);
CREX_METHOD(FilterIterator, rewind);
CREX_METHOD(FilterIterator, next);
CREX_METHOD(RecursiveFilterIterator, __main);
CREX_METHOD(RecursiveFilterIterator, getChildren);
CREX_METHOD(ParentIterator, __main);
CREX_METHOD(LimitIterator, __main);
CREX_METHOD(LimitIterator, rewind);
CREX_METHOD(LimitIterator, valid);
CREX_METHOD(LimitIterator, next);
CREX_METHOD(LimitIterator, seek);
CREX_METHOD(LimitIterator, getPosition);
CREX_METHOD(CachingIterator, __main);
CREX_METHOD(CachingIterator, rewind);
CREX_METHOD(CachingIterator, valid);
CREX_METHOD(CachingIterator, next);
CREX_METHOD(CachingIterator, hasNext);
CREX_METHOD(CachingIterator, __toString);
CREX_METHOD(CachingIterator, getFlags);
CREX_METHOD(CachingIterator, setFlags);
CREX_METHOD(CachingIterator, offsetGet);
CREX_METHOD(CachingIterator, offsetSet);
CREX_METHOD(CachingIterator, offsetUnset);
CREX_METHOD(CachingIterator, offsetExists);
CREX_METHOD(CachingIterator, getCache);
CREX_METHOD(CachingIterator, count);
CREX_METHOD(RecursiveCachingIterator, __main);
CREX_METHOD(RecursiveCachingIterator, hasChildren);
CREX_METHOD(RecursiveCachingIterator, getChildren);
CREX_METHOD(NoRewindIterator, __main);
CREX_METHOD(NoRewindIterator, rewind);
CREX_METHOD(NoRewindIterator, valid);
CREX_METHOD(NoRewindIterator, key);
CREX_METHOD(NoRewindIterator, current);
CREX_METHOD(NoRewindIterator, next);
CREX_METHOD(AppendIterator, __main);
CREX_METHOD(AppendIterator, append);
CREX_METHOD(AppendIterator, rewind);
CREX_METHOD(AppendIterator, valid);
CREX_METHOD(AppendIterator, current);
CREX_METHOD(AppendIterator, next);
CREX_METHOD(AppendIterator, getIteratorIndex);
CREX_METHOD(AppendIterator, getArrayIterator);
CREX_METHOD(InfiniteIterator, __main);
CREX_METHOD(InfiniteIterator, next);
CREX_METHOD(RegexIterator, __main);
CREX_METHOD(RegexIterator, accept);
CREX_METHOD(RegexIterator, getMode);
CREX_METHOD(RegexIterator, setMode);
CREX_METHOD(RegexIterator, getFlags);
CREX_METHOD(RegexIterator, setFlags);
CREX_METHOD(RegexIterator, getRegex);
CREX_METHOD(RegexIterator, getPregFlags);
CREX_METHOD(RegexIterator, setPregFlags);
CREX_METHOD(RecursiveRegexIterator, __main);
CREX_METHOD(RecursiveRegexIterator, accept);
CREX_METHOD(RecursiveRegexIterator, getChildren);
CREX_METHOD(RecursiveTreeIterator, __main);
CREX_METHOD(RecursiveTreeIterator, key);
CREX_METHOD(RecursiveTreeIterator, current);
CREX_METHOD(RecursiveTreeIterator, getPrefix);
CREX_METHOD(RecursiveTreeIterator, setPostfix);
CREX_METHOD(RecursiveTreeIterator, setPrefixPart);
CREX_METHOD(RecursiveTreeIterator, getEntry);
CREX_METHOD(RecursiveTreeIterator, getPostfix);


static const crex_function_entry class_EmptyIterator_methods[] = {
	CREX_ME(EmptyIterator, current, arginfo_class_EmptyIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(EmptyIterator, next, arginfo_class_EmptyIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(EmptyIterator, key, arginfo_class_EmptyIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(EmptyIterator, valid, arginfo_class_EmptyIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(EmptyIterator, rewind, arginfo_class_EmptyIterator_rewind, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CallbackFilterIterator_methods[] = {
	CREX_ME(CallbackFilterIterator, __main, arginfo_class_CallbackFilterIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(CallbackFilterIterator, accept, arginfo_class_CallbackFilterIterator_accept, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveCallbackFilterIterator_methods[] = {
	CREX_ME(RecursiveCallbackFilterIterator, __main, arginfo_class_RecursiveCallbackFilterIterator___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(RecursiveFilterIterator, hasChildren, hasChildren, arginfo_class_RecursiveCallbackFilterIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveCallbackFilterIterator, getChildren, arginfo_class_RecursiveCallbackFilterIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveIterator_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(RecursiveIterator, hasChildren, arginfo_class_RecursiveIterator_hasChildren, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ABSTRACT_ME_WITH_FLAGS(RecursiveIterator, getChildren, arginfo_class_RecursiveIterator_getChildren, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveIteratorIterator_methods[] = {
	CREX_ME(RecursiveIteratorIterator, __main, arginfo_class_RecursiveIteratorIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, rewind, arginfo_class_RecursiveIteratorIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, valid, arginfo_class_RecursiveIteratorIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, key, arginfo_class_RecursiveIteratorIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, current, arginfo_class_RecursiveIteratorIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, next, arginfo_class_RecursiveIteratorIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, getDepth, arginfo_class_RecursiveIteratorIterator_getDepth, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, getSubIterator, arginfo_class_RecursiveIteratorIterator_getSubIterator, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, getInnerIterator, arginfo_class_RecursiveIteratorIterator_getInnerIterator, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, beginIteration, arginfo_class_RecursiveIteratorIterator_beginIteration, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, endIteration, arginfo_class_RecursiveIteratorIterator_endIteration, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, callHasChildren, arginfo_class_RecursiveIteratorIterator_callHasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, callGetChildren, arginfo_class_RecursiveIteratorIterator_callGetChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, beginChildren, arginfo_class_RecursiveIteratorIterator_beginChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, endChildren, arginfo_class_RecursiveIteratorIterator_endChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, nextElement, arginfo_class_RecursiveIteratorIterator_nextElement, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, setMaxDepth, arginfo_class_RecursiveIteratorIterator_setMaxDepth, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveIteratorIterator, getMaxDepth, arginfo_class_RecursiveIteratorIterator_getMaxDepth, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_OuterIterator_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(OuterIterator, getInnerIterator, arginfo_class_OuterIterator_getInnerIterator, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_IteratorIterator_methods[] = {
	CREX_ME(IteratorIterator, __main, arginfo_class_IteratorIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, getInnerIterator, arginfo_class_IteratorIterator_getInnerIterator, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, rewind, arginfo_class_IteratorIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, valid, arginfo_class_IteratorIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, key, arginfo_class_IteratorIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, current, arginfo_class_IteratorIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(IteratorIterator, next, arginfo_class_IteratorIterator_next, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_FilterIterator_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(FilterIterator, accept, arginfo_class_FilterIterator_accept, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_ME(FilterIterator, __main, arginfo_class_FilterIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(FilterIterator, rewind, arginfo_class_FilterIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(FilterIterator, next, arginfo_class_FilterIterator_next, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveFilterIterator_methods[] = {
	CREX_ME(RecursiveFilterIterator, __main, arginfo_class_RecursiveFilterIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveFilterIterator, hasChildren, arginfo_class_RecursiveFilterIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveFilterIterator, getChildren, arginfo_class_RecursiveFilterIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_ParentIterator_methods[] = {
	CREX_ME(ParentIterator, __main, arginfo_class_ParentIterator___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(RecursiveFilterIterator, accept, hasChildren, arginfo_class_ParentIterator_accept, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SeekableIterator_methods[] = {
	CREX_ABSTRACT_ME_WITH_FLAGS(SeekableIterator, seek, arginfo_class_SeekableIterator_seek, CREX_ACC_PUBLIC|CREX_ACC_ABSTRACT)
	CREX_FE_END
};


static const crex_function_entry class_LimitIterator_methods[] = {
	CREX_ME(LimitIterator, __main, arginfo_class_LimitIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(LimitIterator, rewind, arginfo_class_LimitIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(LimitIterator, valid, arginfo_class_LimitIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(LimitIterator, next, arginfo_class_LimitIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(LimitIterator, seek, arginfo_class_LimitIterator_seek, CREX_ACC_PUBLIC)
	CREX_ME(LimitIterator, getPosition, arginfo_class_LimitIterator_getPosition, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_CachingIterator_methods[] = {
	CREX_ME(CachingIterator, __main, arginfo_class_CachingIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, rewind, arginfo_class_CachingIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, valid, arginfo_class_CachingIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, next, arginfo_class_CachingIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, hasNext, arginfo_class_CachingIterator_hasNext, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, __toString, arginfo_class_CachingIterator___toString, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, getFlags, arginfo_class_CachingIterator_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, setFlags, arginfo_class_CachingIterator_setFlags, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, offsetGet, arginfo_class_CachingIterator_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, offsetSet, arginfo_class_CachingIterator_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, offsetUnset, arginfo_class_CachingIterator_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, offsetExists, arginfo_class_CachingIterator_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, getCache, arginfo_class_CachingIterator_getCache, CREX_ACC_PUBLIC)
	CREX_ME(CachingIterator, count, arginfo_class_CachingIterator_count, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveCachingIterator_methods[] = {
	CREX_ME(RecursiveCachingIterator, __main, arginfo_class_RecursiveCachingIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveCachingIterator, hasChildren, arginfo_class_RecursiveCachingIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveCachingIterator, getChildren, arginfo_class_RecursiveCachingIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_NoRewindIterator_methods[] = {
	CREX_ME(NoRewindIterator, __main, arginfo_class_NoRewindIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(NoRewindIterator, rewind, arginfo_class_NoRewindIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(NoRewindIterator, valid, arginfo_class_NoRewindIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(NoRewindIterator, key, arginfo_class_NoRewindIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(NoRewindIterator, current, arginfo_class_NoRewindIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(NoRewindIterator, next, arginfo_class_NoRewindIterator_next, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_AppendIterator_methods[] = {
	CREX_ME(AppendIterator, __main, arginfo_class_AppendIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, append, arginfo_class_AppendIterator_append, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, rewind, arginfo_class_AppendIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, valid, arginfo_class_AppendIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, current, arginfo_class_AppendIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, next, arginfo_class_AppendIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, getIteratorIndex, arginfo_class_AppendIterator_getIteratorIndex, CREX_ACC_PUBLIC)
	CREX_ME(AppendIterator, getArrayIterator, arginfo_class_AppendIterator_getArrayIterator, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_InfiniteIterator_methods[] = {
	CREX_ME(InfiniteIterator, __main, arginfo_class_InfiniteIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(InfiniteIterator, next, arginfo_class_InfiniteIterator_next, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RegexIterator_methods[] = {
	CREX_ME(RegexIterator, __main, arginfo_class_RegexIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, accept, arginfo_class_RegexIterator_accept, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, getMode, arginfo_class_RegexIterator_getMode, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, setMode, arginfo_class_RegexIterator_setMode, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, getFlags, arginfo_class_RegexIterator_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, setFlags, arginfo_class_RegexIterator_setFlags, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, getRegex, arginfo_class_RegexIterator_getRegex, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, getPregFlags, arginfo_class_RegexIterator_getPregFlags, CREX_ACC_PUBLIC)
	CREX_ME(RegexIterator, setPregFlags, arginfo_class_RegexIterator_setPregFlags, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveRegexIterator_methods[] = {
	CREX_ME(RecursiveRegexIterator, __main, arginfo_class_RecursiveRegexIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveRegexIterator, accept, arginfo_class_RecursiveRegexIterator_accept, CREX_ACC_PUBLIC)
	CREX_MALIAS(RecursiveFilterIterator, hasChildren, hasChildren, arginfo_class_RecursiveRegexIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveRegexIterator, getChildren, arginfo_class_RecursiveRegexIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveTreeIterator_methods[] = {
	CREX_ME(RecursiveTreeIterator, __main, arginfo_class_RecursiveTreeIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, key, arginfo_class_RecursiveTreeIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, current, arginfo_class_RecursiveTreeIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, getPrefix, arginfo_class_RecursiveTreeIterator_getPrefix, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, setPostfix, arginfo_class_RecursiveTreeIterator_setPostfix, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, setPrefixPart, arginfo_class_RecursiveTreeIterator_setPrefixPart, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, getEntry, arginfo_class_RecursiveTreeIterator_getEntry, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveTreeIterator, getPostfix, arginfo_class_RecursiveTreeIterator_getPostfix, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_EmptyIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "EmptyIterator", class_EmptyIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}

static crex_class_entry *register_class_CallbackFilterIterator(crex_class_entry *class_entry_FilterIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CallbackFilterIterator", class_CallbackFilterIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FilterIterator);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveCallbackFilterIterator(crex_class_entry *class_entry_CallbackFilterIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveCallbackFilterIterator", class_RecursiveCallbackFilterIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_CallbackFilterIterator);
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveIterator", class_RecursiveIterator_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveIteratorIterator(crex_class_entry *class_entry_OuterIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveIteratorIterator", class_RecursiveIteratorIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_OuterIterator);

	zval const_LEAVES_ONLY_value;
	ZVAL_LONG(&const_LEAVES_ONLY_value, RIT_LEAVES_ONLY);
	crex_string *const_LEAVES_ONLY_name = crex_string_init_interned("LEAVES_ONLY", sizeof("LEAVES_ONLY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_LEAVES_ONLY_name, &const_LEAVES_ONLY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_LEAVES_ONLY_name);

	zval const_SELF_FIRST_value;
	ZVAL_LONG(&const_SELF_FIRST_value, RIT_SELF_FIRST);
	crex_string *const_SELF_FIRST_name = crex_string_init_interned("SELF_FIRST", sizeof("SELF_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SELF_FIRST_name, &const_SELF_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SELF_FIRST_name);

	zval const_CHILD_FIRST_value;
	ZVAL_LONG(&const_CHILD_FIRST_value, RIT_CHILD_FIRST);
	crex_string *const_CHILD_FIRST_name = crex_string_init_interned("CHILD_FIRST", sizeof("CHILD_FIRST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CHILD_FIRST_name, &const_CHILD_FIRST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CHILD_FIRST_name);

	zval const_CATCH_GET_CHILD_value;
	ZVAL_LONG(&const_CATCH_GET_CHILD_value, RIT_CATCH_GET_CHILD);
	crex_string *const_CATCH_GET_CHILD_name = crex_string_init_interned("CATCH_GET_CHILD", sizeof("CATCH_GET_CHILD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CATCH_GET_CHILD_name, &const_CATCH_GET_CHILD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CATCH_GET_CHILD_name);

	return class_entry;
}

static crex_class_entry *register_class_OuterIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "OuterIterator", class_OuterIterator_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}

static crex_class_entry *register_class_IteratorIterator(crex_class_entry *class_entry_OuterIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "IteratorIterator", class_IteratorIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_OuterIterator);

	return class_entry;
}

static crex_class_entry *register_class_FilterIterator(crex_class_entry *class_entry_IteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FilterIterator", class_FilterIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);
	class_entry->ce_flags |= CREX_ACC_ABSTRACT;

	return class_entry;
}

static crex_class_entry *register_class_RecursiveFilterIterator(crex_class_entry *class_entry_FilterIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveFilterIterator", class_RecursiveFilterIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FilterIterator);
	class_entry->ce_flags |= CREX_ACC_ABSTRACT;
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	return class_entry;
}

static crex_class_entry *register_class_ParentIterator(crex_class_entry *class_entry_RecursiveFilterIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ParentIterator", class_ParentIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RecursiveFilterIterator);

	return class_entry;
}

static crex_class_entry *register_class_SeekableIterator(crex_class_entry *class_entry_Iterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SeekableIterator", class_SeekableIterator_methods);
	class_entry = crex_register_internal_interface(&ce);
	crex_class_implements(class_entry, 1, class_entry_Iterator);

	return class_entry;
}

static crex_class_entry *register_class_LimitIterator(crex_class_entry *class_entry_IteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "LimitIterator", class_LimitIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);

	return class_entry;
}

static crex_class_entry *register_class_CachingIterator(crex_class_entry *class_entry_IteratorIterator, crex_class_entry *class_entry_ArrayAccess, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_Stringable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CachingIterator", class_CachingIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);
	crex_class_implements(class_entry, 3, class_entry_ArrayAccess, class_entry_Countable, class_entry_Stringable);

	zval const_CALL_TOSTRING_value;
	ZVAL_LONG(&const_CALL_TOSTRING_value, CIT_CALL_TOSTRING);
	crex_string *const_CALL_TOSTRING_name = crex_string_init_interned("CALL_TOSTRING", sizeof("CALL_TOSTRING") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CALL_TOSTRING_name, &const_CALL_TOSTRING_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CALL_TOSTRING_name);

	zval const_CATCH_GET_CHILD_value;
	ZVAL_LONG(&const_CATCH_GET_CHILD_value, CIT_CATCH_GET_CHILD);
	crex_string *const_CATCH_GET_CHILD_name = crex_string_init_interned("CATCH_GET_CHILD", sizeof("CATCH_GET_CHILD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CATCH_GET_CHILD_name, &const_CATCH_GET_CHILD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CATCH_GET_CHILD_name);

	zval const_TOSTRING_USE_KEY_value;
	ZVAL_LONG(&const_TOSTRING_USE_KEY_value, CIT_TOSTRING_USE_KEY);
	crex_string *const_TOSTRING_USE_KEY_name = crex_string_init_interned("TOSTRING_USE_KEY", sizeof("TOSTRING_USE_KEY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TOSTRING_USE_KEY_name, &const_TOSTRING_USE_KEY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TOSTRING_USE_KEY_name);

	zval const_TOSTRING_USE_CURRENT_value;
	ZVAL_LONG(&const_TOSTRING_USE_CURRENT_value, CIT_TOSTRING_USE_CURRENT);
	crex_string *const_TOSTRING_USE_CURRENT_name = crex_string_init_interned("TOSTRING_USE_CURRENT", sizeof("TOSTRING_USE_CURRENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TOSTRING_USE_CURRENT_name, &const_TOSTRING_USE_CURRENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TOSTRING_USE_CURRENT_name);

	zval const_TOSTRING_USE_INNER_value;
	ZVAL_LONG(&const_TOSTRING_USE_INNER_value, CIT_TOSTRING_USE_INNER);
	crex_string *const_TOSTRING_USE_INNER_name = crex_string_init_interned("TOSTRING_USE_INNER", sizeof("TOSTRING_USE_INNER") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_TOSTRING_USE_INNER_name, &const_TOSTRING_USE_INNER_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_TOSTRING_USE_INNER_name);

	zval const_FULL_CACHE_value;
	ZVAL_LONG(&const_FULL_CACHE_value, CIT_FULL_CACHE);
	crex_string *const_FULL_CACHE_name = crex_string_init_interned("FULL_CACHE", sizeof("FULL_CACHE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FULL_CACHE_name, &const_FULL_CACHE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FULL_CACHE_name);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveCachingIterator(crex_class_entry *class_entry_CachingIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveCachingIterator", class_RecursiveCachingIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_CachingIterator);
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	return class_entry;
}

static crex_class_entry *register_class_NoRewindIterator(crex_class_entry *class_entry_IteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "NoRewindIterator", class_NoRewindIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);

	return class_entry;
}

static crex_class_entry *register_class_AppendIterator(crex_class_entry *class_entry_IteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "AppendIterator", class_AppendIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);

	return class_entry;
}

static crex_class_entry *register_class_InfiniteIterator(crex_class_entry *class_entry_IteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "InfiniteIterator", class_InfiniteIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_IteratorIterator);

	return class_entry;
}

static crex_class_entry *register_class_RegexIterator(crex_class_entry *class_entry_FilterIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RegexIterator", class_RegexIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FilterIterator);

	zval const_USE_KEY_value;
	ZVAL_LONG(&const_USE_KEY_value, REGIT_USE_KEY);
	crex_string *const_USE_KEY_name = crex_string_init_interned("USE_KEY", sizeof("USE_KEY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_USE_KEY_name, &const_USE_KEY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_USE_KEY_name);

	zval const_INVERT_MATCH_value;
	ZVAL_LONG(&const_INVERT_MATCH_value, REGIT_INVERTED);
	crex_string *const_INVERT_MATCH_name = crex_string_init_interned("INVERT_MATCH", sizeof("INVERT_MATCH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_INVERT_MATCH_name, &const_INVERT_MATCH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_INVERT_MATCH_name);

	zval const_MATCH_value;
	ZVAL_LONG(&const_MATCH_value, REGIT_MODE_MATCH);
	crex_string *const_MATCH_name = crex_string_init_interned("MATCH", sizeof("MATCH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_MATCH_name, &const_MATCH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_MATCH_name);

	zval const_GET_MATCH_value;
	ZVAL_LONG(&const_GET_MATCH_value, REGIT_MODE_GET_MATCH);
	crex_string *const_GET_MATCH_name = crex_string_init_interned("GET_MATCH", sizeof("GET_MATCH") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_GET_MATCH_name, &const_GET_MATCH_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_GET_MATCH_name);

	zval const_ALL_MATCHES_value;
	ZVAL_LONG(&const_ALL_MATCHES_value, REGIT_MODE_ALL_MATCHES);
	crex_string *const_ALL_MATCHES_name = crex_string_init_interned("ALL_MATCHES", sizeof("ALL_MATCHES") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_ALL_MATCHES_name, &const_ALL_MATCHES_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_ALL_MATCHES_name);

	zval const_SPLIT_value;
	ZVAL_LONG(&const_SPLIT_value, REGIT_MODE_SPLIT);
	crex_string *const_SPLIT_name = crex_string_init_interned("SPLIT", sizeof("SPLIT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SPLIT_name, &const_SPLIT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SPLIT_name);

	zval const_REPLACE_value;
	ZVAL_LONG(&const_REPLACE_value, REGIT_MODE_REPLACE);
	crex_string *const_REPLACE_name = crex_string_init_interned("REPLACE", sizeof("REPLACE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_REPLACE_name, &const_REPLACE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_REPLACE_name);

	zval property_replacement_default_value;
	ZVAL_NULL(&property_replacement_default_value);
	crex_string *property_replacement_name = crex_string_init("replacement", sizeof("replacement") - 1, 1);
	crex_declare_typed_property(class_entry, property_replacement_name, &property_replacement_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING|MAY_BE_NULL));
	crex_string_release(property_replacement_name);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveRegexIterator(crex_class_entry *class_entry_RegexIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveRegexIterator", class_RecursiveRegexIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RegexIterator);
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveTreeIterator(crex_class_entry *class_entry_RecursiveIteratorIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveTreeIterator", class_RecursiveTreeIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RecursiveIteratorIterator);

	zval const_BYPASS_CURRENT_value;
	ZVAL_LONG(&const_BYPASS_CURRENT_value, RTIT_BYPASS_CURRENT);
	crex_string *const_BYPASS_CURRENT_name = crex_string_init_interned("BYPASS_CURRENT", sizeof("BYPASS_CURRENT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BYPASS_CURRENT_name, &const_BYPASS_CURRENT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BYPASS_CURRENT_name);

	zval const_BYPASS_KEY_value;
	ZVAL_LONG(&const_BYPASS_KEY_value, RTIT_BYPASS_KEY);
	crex_string *const_BYPASS_KEY_name = crex_string_init_interned("BYPASS_KEY", sizeof("BYPASS_KEY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_BYPASS_KEY_name, &const_BYPASS_KEY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_BYPASS_KEY_name);

	zval const_PREFIX_LEFT_value;
	ZVAL_LONG(&const_PREFIX_LEFT_value, 0);
	crex_string *const_PREFIX_LEFT_name = crex_string_init_interned("PREFIX_LEFT", sizeof("PREFIX_LEFT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_LEFT_name, &const_PREFIX_LEFT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_LEFT_name);

	zval const_PREFIX_MID_HAS_NEXT_value;
	ZVAL_LONG(&const_PREFIX_MID_HAS_NEXT_value, 1);
	crex_string *const_PREFIX_MID_HAS_NEXT_name = crex_string_init_interned("PREFIX_MID_HAS_NEXT", sizeof("PREFIX_MID_HAS_NEXT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_MID_HAS_NEXT_name, &const_PREFIX_MID_HAS_NEXT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_MID_HAS_NEXT_name);

	zval const_PREFIX_MID_LAST_value;
	ZVAL_LONG(&const_PREFIX_MID_LAST_value, 2);
	crex_string *const_PREFIX_MID_LAST_name = crex_string_init_interned("PREFIX_MID_LAST", sizeof("PREFIX_MID_LAST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_MID_LAST_name, &const_PREFIX_MID_LAST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_MID_LAST_name);

	zval const_PREFIX_END_HAS_NEXT_value;
	ZVAL_LONG(&const_PREFIX_END_HAS_NEXT_value, 3);
	crex_string *const_PREFIX_END_HAS_NEXT_name = crex_string_init_interned("PREFIX_END_HAS_NEXT", sizeof("PREFIX_END_HAS_NEXT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_END_HAS_NEXT_name, &const_PREFIX_END_HAS_NEXT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_END_HAS_NEXT_name);

	zval const_PREFIX_END_LAST_value;
	ZVAL_LONG(&const_PREFIX_END_LAST_value, 4);
	crex_string *const_PREFIX_END_LAST_name = crex_string_init_interned("PREFIX_END_LAST", sizeof("PREFIX_END_LAST") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_END_LAST_name, &const_PREFIX_END_LAST_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_END_LAST_name);

	zval const_PREFIX_RIGHT_value;
	ZVAL_LONG(&const_PREFIX_RIGHT_value, 5);
	crex_string *const_PREFIX_RIGHT_name = crex_string_init_interned("PREFIX_RIGHT", sizeof("PREFIX_RIGHT") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_PREFIX_RIGHT_name, &const_PREFIX_RIGHT_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_PREFIX_RIGHT_name);

	return class_entry;
}
