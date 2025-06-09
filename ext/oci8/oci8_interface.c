/*
   +----------------------------------------------------------------------+
   | Copyright (c) The CRX Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the CRX license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.crx.net/license/3_01.txt                                 |
   | If you did not receive a copy of the CRX license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crx.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stig Sæther Bakken <ssb@crx.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |                                                                      |
   | Collection support by Andy Sautins <asautins@veripost.net>           |
   | Temporary LOB support by David Benson <dbenson@mancala.com>          |
   | ZTS per process OCIPLogon by Harald Radi <harald.radi@nme.at>        |
   |                                                                      |
   | Redesigned by: Antony Dovgal <antony@crex.com>                       |
   |                Andi Gutmans <andi@crx.net>                           |
   |                Wez Furlong <wez@omniti.com>                          |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "crx.h"
#include "ext/standard/info.h"
#include "crx_ini.h"

#ifdef HAVE_OCI8

#include "crx_oci8.h"
#include "crx_oci8_int.h"

#ifndef OCI_STMT_CALL
#define OCI_STMT_CALL 10
#endif

#define ERROR_ARG_POS(arg_num) (getThis() ? (arg_num-1) : (arg_num))

/* {{{ Register a callback function for Oracle Transparent Application Failover (TAF) */
CRX_FUNCTION(oci_register_taf_callback)
{
	zval *z_connection;
	crx_oci_connection *connection;
	crex_fcall_info fci;
	crex_fcall_info_cache fcc;
	zval *callback = NULL;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r|f!", &z_connection, &fci, &fcc) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	/* If callable passed, assign callback zval so that it can be passed to crx_oci_register_taf_callback() */
	if (CREX_FCI_INITIALIZED(fci)) {
		callback = &fci.function_name;
	}

	if (crx_oci_register_taf_callback(connection, callback) == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{    Unregister a callback function for Oracle Transparent Application Failover (TAF) */
CRX_FUNCTION(oci_unregister_taf_callback)
{
	zval *z_connection;
	crx_oci_connection *connection;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &z_connection) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (crx_oci_unregister_taf_callback(connection) == 0) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Define a CRX variable to an Oracle column by name */
/* if you want to define a LOB/CLOB etc make sure you allocate it via OCINewDescriptor BEFORE defining!!! */
CRX_FUNCTION(oci_define_by_name)
{
	zval *stmt, *var;
	char *name;
	size_t name_len;
	crex_long type = 0;
	crx_oci_statement *statement;
	crx_oci_define *define;
	crex_string *zvtmp;

	CREX_PARSE_PARAMETERS_START(3, 4)
		C_PARAM_RESOURCE(stmt)
		C_PARAM_STRING(name, name_len)
		C_PARAM_ZVAL(var)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(type)
	CREX_PARSE_PARAMETERS_END();

	if (!name_len) {
		crex_argument_value_error(2, "cannot be empty");
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_STATEMENT(stmt, statement);

	if (statement->defines == NULL) {
		ALLOC_HASHTABLE(statement->defines);
		crex_hash_init(statement->defines, 13, NULL, crx_oci_define_hash_dtor, 0);
	}
	else if (crex_hash_str_exists(statement->defines, (const char *)name, name_len)) {
		RETURN_FALSE;
	}

	define = ecalloc(1,sizeof(crx_oci_define));

	/* if (crex_hash_add(statement->defines, name, name_len, define, sizeof(crx_oci_define), (void **)&tmp_define) == SUCCESS) { */
	zvtmp = crex_string_init(name, name_len, 0);
	if ((define = crex_hash_add_new_ptr(statement->defines, zvtmp, define)) != NULL) {
		crex_string_release_ex(zvtmp, 0);
	} else {
		efree(define);
		crex_string_release_ex(zvtmp, 0);
		RETURN_FALSE;
	}

	define->name = (text*) ecalloc(1, name_len+1);
	memcpy(define->name, name, name_len);
	define->name[name_len] = '\0';
	define->name_len = (ub4) name_len;
	define->type = (ub4) type;
	CREX_ASSERT(C_ISREF_P(var));
	ZVAL_COPY(&define->val, var);

	RETURN_TRUE;
}
/* }}} */

/* {{{ Bind a CRX variable to an Oracle placeholder by name */
/* if you want to bind a LOB/CLOB etc make sure you allocate it via OCINewDescriptor BEFORE binding!!! */
CRX_FUNCTION(oci_bind_by_name)
{
	ub2	bind_type = SQLT_CHR; /* unterminated string */
	size_t name_len;
	crex_long maxlen = -1, type = 0;
	char *name;
	zval *z_statement;
	zval *bind_var = NULL;
	crx_oci_statement *statement;

	CREX_PARSE_PARAMETERS_START(3, 5)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_STRING(name, name_len)
		C_PARAM_ZVAL(bind_var)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(maxlen)
		C_PARAM_LONG(type)
	CREX_PARSE_PARAMETERS_END();

	if (type) {
		bind_type = (ub2) type;
	}

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_bind_by_name(statement, name, name_len, bind_var, maxlen, bind_type)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Bind a CRX array to an Oracle PL/SQL type by name */
CRX_FUNCTION(oci_bind_array_by_name)
{
	size_t name_len;
	crex_long max_item_len = -1;
	crex_long max_array_len = 0;
	crex_long type = SQLT_AFC;
	char *name;
	zval *z_statement;
	zval *bind_var = NULL;
	crx_oci_statement *statement;

	CREX_PARSE_PARAMETERS_START(4, 6)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_STRING(name, name_len)
		C_PARAM_ZVAL(bind_var)
		C_PARAM_LONG(max_array_len)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(max_item_len)
		C_PARAM_LONG(type)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (CREX_NUM_ARGS() == 5 && max_item_len <= 0) {
		max_item_len = -1;
	}

	if (max_array_len <= 0) {
		crex_argument_value_error(4, "must be greater than 0");
		RETURN_THROWS();
	}

	if (crx_oci_bind_array_by_name(statement, name, (sb4) name_len, bind_var, max_array_len, max_item_len, type)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Deletes large object description */
CRX_FUNCTION(oci_free_descriptor)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	crex_list_close(descriptor->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Saves a large object */
CRX_FUNCTION(oci_lob_save)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	char *data;
	size_t data_len;
	crex_long offset = 0;
	ub4 bytes_written;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os|l", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &offset) == FAILURE) {
		RETURN_THROWS();
	}

	if (offset < 0) {
		crex_argument_value_error(ERROR_ARG_POS(3), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_write(descriptor, (ub4) offset, data, (ub4) data_len, &bytes_written)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Loads file into a LOB */
CRX_FUNCTION(oci_lob_import)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	char *filename;
	size_t filename_len;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Op", &z_descriptor, oci_lob_class_entry_ptr, &filename, &filename_len) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_import(descriptor, filename)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Loads a large object */
CRX_FUNCTION(oci_lob_load)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	char *buffer = NULL;
	ub4 buffer_len;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_read(descriptor, -1, 0, &buffer, &buffer_len)) {
		RETURN_FALSE;
	}
	if (buffer_len > 0) {
		crex_string *ret = crex_string_init(buffer, buffer_len, 0);
		if (buffer)
			efree(buffer);
		RETURN_STR(ret);
	}
	else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Reads particular part of a large object */
CRX_FUNCTION(oci_lob_read)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	crex_long length;
	char *buffer;
	ub4 buffer_len;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ol", &z_descriptor, oci_lob_class_entry_ptr, &length) == FAILURE) {
		RETURN_THROWS();
	}

	if (length <= 0) {
		crex_argument_value_error(ERROR_ARG_POS(2), "must be greater than 0");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_read(descriptor, length, descriptor->lob_current_position, &buffer, &buffer_len)) {
		RETURN_FALSE;
	}
	if (buffer_len > 0) {
		crex_string *ret = crex_string_init(buffer, buffer_len, 0);
		efree(buffer);
		RETURN_STR(ret);
	}
	else {
		RETURN_EMPTY_STRING();
	}
}
/* }}} */

/* {{{ Checks if EOF is reached */
CRX_FUNCTION(oci_lob_eof)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	ub4 lob_length;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (!crx_oci_lob_get_length(descriptor, &lob_length)) {
		if (lob_length == descriptor->lob_current_position) {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tells LOB pointer position */
CRX_FUNCTION(oci_lob_tell)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	RETURN_LONG(descriptor->lob_current_position);
}
/* }}} */

/* {{{ Rewind pointer of a LOB */
CRX_FUNCTION(oci_lob_rewind)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	descriptor->lob_current_position = 0;

	RETURN_TRUE;
}
/* }}} */

/* {{{ Moves the pointer of a LOB */
CRX_FUNCTION(oci_lob_seek)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	crex_long offset, whence = CRX_OCI_SEEK_SET;
	ub4 lob_length;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ol|l", &z_descriptor, oci_lob_class_entry_ptr, &offset, &whence) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_get_length(descriptor, &lob_length)) {
		RETURN_FALSE;
	}

	switch(whence) {
		case CRX_OCI_SEEK_CUR:
			descriptor->lob_current_position += (ub4) offset;
			break;
		case CRX_OCI_SEEK_END:
			if ((descriptor->lob_size + offset) >= 0) {
				descriptor->lob_current_position = descriptor->lob_size + (ub4) offset;
			}
			else {
				descriptor->lob_current_position = 0;
			}
			break;
		case CRX_OCI_SEEK_SET:
		default:
				descriptor->lob_current_position = (offset > 0) ? (ub4) offset : 0;
			break;
	}
	if (descriptor->lob_current_position > UB4MAXVAL) {
		crx_error_docref(NULL, E_WARNING, "Invalid offset or LOB position");
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns size of a large object */
CRX_FUNCTION(oci_lob_size)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	ub4 lob_length;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_get_length(descriptor, &lob_length)) {
		RETURN_FALSE;
	}
	RETURN_LONG(lob_length);
}
/* }}} */

/* {{{ Writes data to current position of a LOB */
CRX_FUNCTION(oci_lob_write)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	size_t data_len;
	crex_long write_len;
	bool write_len_is_null = 1;
	ub4 bytes_written;
	char *data;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os|l!", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &write_len, &write_len_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	if (!write_len_is_null) {
		data_len = MIN((crex_long) data_len, write_len);
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (data_len <= 0) {
		RETURN_LONG(0);
	}

	if (crx_oci_lob_write(descriptor, descriptor->lob_current_position, data, (ub4) data_len, &bytes_written)) {
		RETURN_FALSE;
	}
	RETURN_LONG(bytes_written);
}
/* }}} */

/* {{{ Appends data from a LOB to another LOB */
CRX_FUNCTION(oci_lob_append)
{
	zval *tmp_dest, *tmp_from, *z_descriptor_dest, *z_descriptor_from;
	crx_oci_descriptor *descriptor_dest, *descriptor_from;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "OO", &z_descriptor_dest, oci_lob_class_entry_ptr, &z_descriptor_from, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp_dest = crex_hash_str_find(C_OBJPROP_P(z_descriptor_dest), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}

	if ((tmp_from = crex_hash_str_find(C_OBJPROP_P(z_descriptor_from), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_dest, descriptor_dest);
	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_from, descriptor_from);

	if (crx_oci_lob_append(descriptor_dest, descriptor_from)) {
		RETURN_FALSE;
	}
	/* XXX should we increase lob_size here ? */
	RETURN_TRUE;
}
/* }}} */

/* {{{ Truncates a LOB */
CRX_FUNCTION(oci_lob_truncate)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	crex_long trim_length = 0;
	ub4 ub_trim_length;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O|l", &z_descriptor, oci_lob_class_entry_ptr, &trim_length) == FAILURE) {
		RETURN_THROWS();
	}

	if (trim_length < 0) {
		crex_argument_value_error(ERROR_ARG_POS(2), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	ub_trim_length = (ub4) trim_length;
	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_truncate(descriptor, ub_trim_length)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Erases a specified portion of the internal LOB, starting at a specified offset */
CRX_FUNCTION(oci_lob_erase)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	ub4 bytes_erased;
	crex_long offset, length;
	bool offset_is_null = 1, length_is_null = 1;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O|l!l!", &z_descriptor, oci_lob_class_entry_ptr, &offset, &offset_is_null, &length, &length_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	if (offset_is_null) {
		offset = -1;
	} else if (offset < 0) {
		crex_argument_value_error(ERROR_ARG_POS(2), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (length_is_null) {
		length = -1;
	} else if (length < 0) {
		crex_argument_value_error(ERROR_ARG_POS(3), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_erase(descriptor, offset, (ub4) length, &bytes_erased)) {
		RETURN_FALSE;
	}
	RETURN_LONG(bytes_erased);
}
/* }}} */

/* {{{ Flushes the LOB buffer */
CRX_FUNCTION(oci_lob_flush)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	crex_long flush_flag = 0;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O|l", &z_descriptor, oci_lob_class_entry_ptr, &flush_flag) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (descriptor->buffering == CRX_OCI_LOB_BUFFER_DISABLED) {
		/* buffering wasn't enabled, there is nothing to flush */
		RETURN_FALSE;
	}

	if (crx_oci_lob_flush(descriptor, flush_flag)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Enables/disables buffering for a LOB */
CRX_FUNCTION(ocisetbufferinglob)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	bool flag;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ob", &z_descriptor, oci_lob_class_entry_ptr, &flag) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_set_buffering(descriptor, flag)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Returns current state of buffering for a LOB */
CRX_FUNCTION(ocigetbufferinglob)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (descriptor->buffering != CRX_OCI_LOB_BUFFER_DISABLED) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Copies data from a LOB to another LOB */
CRX_FUNCTION(oci_lob_copy)
{
	zval *tmp_dest, *tmp_from, *z_descriptor_dest, *z_descriptor_from;
	crx_oci_descriptor *descriptor_dest, *descriptor_from;
	crex_long length;
	bool length_is_null = 1;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO|l!", &z_descriptor_dest, oci_lob_class_entry_ptr, &z_descriptor_from, oci_lob_class_entry_ptr, &length, &length_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	if (length_is_null) {
		length = -1;
	} else if (length < 0) {
		crex_argument_value_error(3, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if ((tmp_dest = crex_hash_str_find(C_OBJPROP_P(z_descriptor_dest), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}

	if ((tmp_from = crex_hash_str_find(C_OBJPROP_P(z_descriptor_from), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_dest, descriptor_dest);
	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_from, descriptor_from);

	if (crx_oci_lob_copy(descriptor_dest, descriptor_from, length)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Tests to see if two LOB/FILE locators are equal */
CRX_FUNCTION(oci_lob_is_equal)
{
	zval *tmp_first, *tmp_second, *z_descriptor_first, *z_descriptor_second;
	crx_oci_descriptor *descriptor_first, *descriptor_second;
	boolean is_equal;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "OO", &z_descriptor_first, oci_lob_class_entry_ptr, &z_descriptor_second, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp_first = crex_hash_str_find(C_OBJPROP_P(z_descriptor_first), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}

	if ((tmp_second = crex_hash_str_find(C_OBJPROP_P(z_descriptor_second), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_first, descriptor_first);
	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp_second, descriptor_second);

	if (crx_oci_lob_is_equal(descriptor_first, descriptor_second, &is_equal)) {
		RETURN_FALSE;
	}

	if (is_equal == TRUE) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Writes a large object into a file */
CRX_FUNCTION(oci_lob_export)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	char *filename;
	char *buffer;
	size_t filename_len;
	crex_long start, length, block_length;
	bool start_is_null = 1, length_is_null = 1;
	crx_stream *stream;
	ub4 lob_length;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Op|l!l!", &z_descriptor, oci_lob_class_entry_ptr, &filename, &filename_len, &start, &start_is_null, &length, &length_is_null) == FAILURE) {
		RETURN_THROWS();
	}

	if (start_is_null) {
		start = -1;
	} else if (start < 0) {
		crex_argument_value_error(ERROR_ARG_POS(3), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (length_is_null) {
		length = -1;
	} else if (length < 0) {
		crex_argument_value_error(ERROR_ARG_POS(4), "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_get_length(descriptor, &lob_length)) {
		RETURN_FALSE;
	}

	if (start == -1) {
		start = 0;
	}

	if (length == -1) {
		length = lob_length - descriptor->lob_current_position;
	}

	if (lob_length == 0) {
		length = 0;
	}

	if (length == 0) {
		/* nothing to write, fail silently */
		RETURN_FALSE;
	}

	if (crx_check_open_basedir(filename)) {
		RETURN_FALSE;
	}

	stream = crx_stream_open_wrapper_ex(filename, "w", REPORT_ERRORS, NULL, NULL);

	block_length = CRX_OCI_LOB_BUFFER_SIZE;
	if (block_length > length) {
		block_length = length;
	}

	while(length > 0) {
		ub4 tmp_bytes_read = 0;
		if (crx_oci_lob_read(descriptor, block_length, start, &buffer, &tmp_bytes_read)) {
			crx_stream_close(stream);
			RETURN_FALSE;
		}
		if (tmp_bytes_read && !crx_stream_write(stream, buffer, tmp_bytes_read)) {
			crx_stream_close(stream);
			if (buffer)
				efree(buffer);
			RETURN_FALSE;
		}
		if (buffer) {
			efree(buffer);
		}

		length -= tmp_bytes_read;
		descriptor->lob_current_position += tmp_bytes_read;
		start += tmp_bytes_read;

		if (block_length > length) {
			block_length = length;
		}
	}

	crx_stream_close(stream);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Writes temporary blob */
CRX_METHOD(OCILob, writeTemporary)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;
	char *data;
	size_t data_len;
	crex_long type = OCI_TEMP_CLOB;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os|l", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &type) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_write_tmp(descriptor, type, data, (int) data_len)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Closes lob descriptor */
CRX_METHOD(OCILob, close)
{
	zval *tmp, *z_descriptor;
	crx_oci_descriptor *descriptor;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_DESCRIPTOR(tmp, descriptor);

	if (crx_oci_lob_close(descriptor)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Initialize a new empty descriptor LOB/FILE (LOB is default) */
CRX_FUNCTION(oci_new_descriptor)
{
	zval *z_connection;
	crx_oci_connection *connection;
	crx_oci_descriptor *descriptor;
	crex_long type = OCI_DTYPE_LOB;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r|l", &z_connection, &type) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	/* crx_oci_lob_create() checks type */
	descriptor = crx_oci_lob_create(connection, type);

	if (!descriptor) {
		RETURN_NULL();
	}

	object_init_ex(return_value, oci_lob_class_entry_ptr);
	add_property_resource(return_value, "descriptor", descriptor->id);
}
/* }}} */

/* {{{ Rollback the current context */
CRX_FUNCTION(oci_rollback)
{
	zval *z_connection;
	crx_oci_connection *connection;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_connection)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (connection->descriptors) {
		crx_oci_connection_descriptors_free(connection);
	}

	if (crx_oci_connection_rollback(connection)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Commit the current context */
CRX_FUNCTION(oci_commit)
{
	zval *z_connection;
	crx_oci_connection *connection;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_connection)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (connection->descriptors) {
		crx_oci_connection_descriptors_free(connection);
	}

	if (crx_oci_connection_commit(connection)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Tell the name of a column */
CRX_FUNCTION(oci_field_name)
{
	crx_oci_out_column *column;

	if ( ( column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_STRINGL(column->name, column->name_len);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tell the maximum data size of a column */
CRX_FUNCTION(oci_field_size)
{
	crx_oci_out_column *column;

	if ( ( column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		/* Handle data type of LONG */
		if (column->data_type == SQLT_LNG){
			RETURN_LONG(column->storage_size4);
		}
		RETURN_LONG(column->data_size);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tell the scale of a column */
CRX_FUNCTION(oci_field_scale)
{
	crx_oci_out_column *column;

	if ( ( column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_LONG(column->scale);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tell the precision of a column */
CRX_FUNCTION(oci_field_precision)
{
	crx_oci_out_column *column;

	if ( ( column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_LONG(column->precision);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tell the data type of a column */
CRX_FUNCTION(oci_field_type)
{
	crx_oci_out_column *column;

	column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);

	if (!column) {
		RETURN_FALSE;
	}

	switch (column->data_type) {
#ifdef SQLT_TIMESTAMP
		case SQLT_TIMESTAMP:
			RETVAL_STRING("TIMESTAMP");
			break;
#endif
#ifdef SQLT_TIMESTAMP_TZ
		case SQLT_TIMESTAMP_TZ:
			RETVAL_STRING("TIMESTAMP WITH TIMEZONE");
			break;
#endif
#ifdef SQLT_TIMESTAMP_LTZ
		case SQLT_TIMESTAMP_LTZ:
			RETVAL_STRING("TIMESTAMP WITH LOCAL TIMEZONE");
			break;
#endif
#ifdef SQLT_INTERVAL_YM
		case SQLT_INTERVAL_YM:
			RETVAL_STRING("INTERVAL YEAR TO MONTH");
			break;
#endif
#ifdef SQLT_INTERVAL_DS
		case SQLT_INTERVAL_DS:
			RETVAL_STRING("INTERVAL DAY TO SECOND");
			break;
#endif
		case SQLT_DAT:
			RETVAL_STRING("DATE");
			break;
		case SQLT_NUM:
			RETVAL_STRING("NUMBER");
			break;
		case SQLT_LNG:
			RETVAL_STRING("LONG");
			break;
		case SQLT_BIN:
			RETVAL_STRING("RAW");
			break;
		case SQLT_LBI:
			RETVAL_STRING("LONG RAW");
			break;
		case SQLT_CHR:
			RETVAL_STRING("VARCHAR2");
			break;
		case SQLT_RSET:
			RETVAL_STRING("REFCURSOR");
			break;
		case SQLT_AFC:
			RETVAL_STRING("CHAR");
			break;
		case SQLT_BLOB:
			RETVAL_STRING("BLOB");
			break;
		case SQLT_CLOB:
			RETVAL_STRING("CLOB");
			break;
		case SQLT_BFILE:
			RETVAL_STRING("BFILE");
			break;
		case SQLT_RDD:
			RETVAL_STRING("ROWID");
			break;
		default:
			RETVAL_LONG(column->data_type);
	}
}
/* }}} */

/* {{{ Tell the raw oracle data type of a column */
CRX_FUNCTION(oci_field_type_raw)
{
	crx_oci_out_column *column;

	column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
	if (column) {
		RETURN_LONG(column->data_type);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Tell whether a field in the current row is NULL */
CRX_FUNCTION(oci_field_is_null)
{
	crx_oci_out_column *column;

	if ( ( column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		if (column->indicator == -1) {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Execute a parsed statement */
CRX_FUNCTION(oci_execute)
{
	zval *z_statement;
	crx_oci_statement *statement;
	crex_long mode = OCI_COMMIT_ON_SUCCESS;

	CREX_PARSE_PARAMETERS_START(1, 2)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(mode)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_statement_execute(statement, (ub4) mode)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Cancel reading from a cursor */
CRX_FUNCTION(oci_cancel)
{
	zval *z_statement;
	crx_oci_statement *statement;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "r", &z_statement) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_statement_cancel(statement)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Prepare a new row of data for reading */
CRX_FUNCTION(oci_fetch)
{
	zval *z_statement;
	crx_oci_statement *statement;
	ub4 nrows = 1; /* only one row at a time is supported for now */

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_statement_fetch(statement, nrows)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Fetch a row of result data into an array */
CRX_FUNCTION(ocifetchinto)
{
	crx_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_OCI_NUM, 3);
}
/* }}} */

/* {{{ Fetch all rows of result data into an array */
CRX_FUNCTION(oci_fetch_all)
{
	zval *z_statement, *array;
	zval element, tmp;
	crx_oci_statement *statement;
	crx_oci_out_column **columns;
	zval **outarrs;
	ub4 nrows = 1;
	int i;
	crex_long rows = 0, flags = CRX_OCI_FETCHSTATEMENT_BY_COLUMN | CRX_OCI_ASSOC, skip = 0, maxrows = -1;

	CREX_PARSE_PARAMETERS_START(2, 5)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_ZVAL(array)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(skip)
		C_PARAM_LONG(maxrows)
		C_PARAM_LONG(flags)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	while (skip--) {
		if (crx_oci_statement_fetch(statement, nrows)) {
			crex_try_array_init(array);
			RETURN_LONG(0);
		}
	}

	if (flags & CRX_OCI_FETCHSTATEMENT_BY_ROW) {
		/* Fetch by Row: array will contain one sub-array per query row */
		array = crex_try_array_init(array);
		if (!array) {
			RETURN_THROWS();
		}

		columns = safe_emalloc(statement->ncolumns, sizeof(crx_oci_out_column *), 0);
		for (i = 0; i < statement->ncolumns; i++) {
			columns[ i ] = crx_oci_statement_get_column(statement, i + 1, NULL, 0);
		}

		while (!crx_oci_statement_fetch(statement, nrows)) {
			zval row;

			array_init_size(&row, statement->ncolumns);

			for (i = 0; i < statement->ncolumns; i++) {
				crx_oci_column_to_zval(columns[ i ], &element, CRX_OCI_RETURN_LOBS);

				if (flags & CRX_OCI_NUM) {
					crex_hash_next_index_insert(C_ARRVAL(row), &element);
				} else { /* default to ASSOC */
					crex_string *zvtmp;
					zvtmp = crex_string_init(columns[ i ]->name, columns[ i ]->name_len, 0);
					crex_symtable_update(C_ARRVAL(row), zvtmp, &element);
					crex_string_release_ex(zvtmp, 0);
				}
			}

			crex_hash_next_index_insert(C_ARRVAL_P(array), &row);
			rows++;

			if (maxrows != -1 && rows == maxrows) {
				crx_oci_statement_cancel(statement);
				break;
			}
		}
		efree(columns);

	} else { /* default to BY_COLUMN */
		/* Fetch by columns: array will contain one sub-array per query column */
		array = crex_try_array_init_size(array, statement->ncolumns);
		if (!array) {
			RETURN_THROWS();
		}

		columns = safe_emalloc(statement->ncolumns, sizeof(crx_oci_out_column *), 0);
		outarrs = safe_emalloc(statement->ncolumns, sizeof(zval*), 0);

		if (flags & CRX_OCI_NUM) {
			for (i = 0; i < statement->ncolumns; i++) {
				columns[ i ] = crx_oci_statement_get_column(statement, i + 1, NULL, 0);

				array_init(&tmp);
				outarrs[ i ] = crex_hash_next_index_insert(C_ARRVAL_P(array), &tmp);
			}
		} else { /* default to ASSOC */
			for (i = 0; i < statement->ncolumns; i++) {
				crex_string *zvtmp;
				columns[ i ] = crx_oci_statement_get_column(statement, i + 1, NULL, 0);

				array_init(&tmp);
				zvtmp = crex_string_init(columns[ i ]->name, columns[ i ]->name_len, 0);
				outarrs[ i ] = crex_symtable_update(C_ARRVAL_P(array), zvtmp, &tmp);
				crex_string_release_ex(zvtmp, 0);
			}
		}

		while (!crx_oci_statement_fetch(statement, nrows)) {
			for (i = 0; i < statement->ncolumns; i++) {
				crx_oci_column_to_zval(columns[ i ], &element, CRX_OCI_RETURN_LOBS);
				crex_hash_index_update(C_ARRVAL_P(outarrs[ i ]), rows, &element);
			}

			rows++;

			if (maxrows != -1 && rows == maxrows) {
				crx_oci_statement_cancel(statement);
				break;
			}
		}

		efree(columns);
		efree(outarrs);
	}

	RETURN_LONG(rows);
}
/* }}} */

/* {{{ Fetch a result row as an object */
CRX_FUNCTION(oci_fetch_object)
{
	crx_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_OCI_ASSOC | CRX_OCI_RETURN_NULLS, 2);

	if (C_TYPE_P(return_value) == IS_ARRAY) {
		object_and_properties_init(return_value, CREX_STANDARD_CLASS_DEF_PTR, C_ARRVAL_P(return_value));
	}
}
/* }}} */

/* {{{ Fetch a result row as an enumerated array */
CRX_FUNCTION(oci_fetch_row)
{
	crx_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_OCI_NUM | CRX_OCI_RETURN_NULLS, 1);
}
/* }}} */

/* {{{ Fetch a result row as an associative array */
CRX_FUNCTION(oci_fetch_assoc)
{
	crx_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_OCI_ASSOC | CRX_OCI_RETURN_NULLS, 1);
}
/* }}} */

/* {{{ Fetch a result row as an array */
CRX_FUNCTION(oci_fetch_array)
{
	crx_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, CRX_OCI_BOTH | CRX_OCI_RETURN_NULLS, 2);
}
/* }}} */

/* {{{ Free all resources associated with a statement */
CRX_FUNCTION(oci_free_statement)
{
	zval *z_statement;
	crx_oci_statement *statement;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	crex_list_close(statement->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Disconnect from database */
CRX_FUNCTION(oci_close)
{
	/* oci_close for pconnect (if old_oci_close_semantics not set) would
	 * release the connection back to the client-side session pool (and to the
	 * server-side pool if Database Resident Connection Pool is being used).
	 * Subsequent pconnects in the same script are not guaranteed to get the
	 * same database session.
	 */

	zval *z_connection;
	crx_oci_connection *connection;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_connection)
	CREX_PARSE_PARAMETERS_END();

	if (OCI_G(old_oci_close_semantics)) {
		/* do nothing to keep BC */
		RETURN_NULL();
	}

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);
	if (GC_REFCOUNT(connection->id) == 2) { /* CHANGED VERSION::CRX7
											   Changed the refCount to 2 since
											   internally Crex engine increments
											   RefCount value by 1 */
		/* Unregister Oracle TAF */
		crx_oci_unregister_taf_callback(connection);

		crex_list_close(connection->id);
	}

	/* ZVAL_NULL(z_connection); */

	RETURN_TRUE;
}
/* }}} */

/* {{{ Connect to an Oracle database and log on. Returns a new session. */
CRX_FUNCTION(oci_new_connect)
{
	crx_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ Connect to an Oracle database and log on. Returns a new session. */
CRX_FUNCTION(oci_connect)
{
	crx_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ Connect to an Oracle database using a persistent connection and log on. Returns a new session. */
CRX_FUNCTION(oci_pconnect)
{
	crx_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
}
/* }}} */

/* {{{ Return the last error of stmt|connection|global. If no error happened returns false. */
CRX_FUNCTION(oci_error)
{
	zval *arg = NULL;
	crx_oci_statement *statement;
	crx_oci_connection *connection;
	text errbuf[CRX_OCI_ERRBUF_LEN];
	sb4 errcode = 0;
	dvoid *errh = NULL;
	ub2 error_offset = 0;
	text *sqltext = NULL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_RESOURCE_OR_NULL(arg)
	CREX_PARSE_PARAMETERS_END();

	if (arg) {
		statement = (crx_oci_statement *) crex_fetch_resource_ex(arg, NULL, le_statement);
		if (statement) {
			errh = statement->err;
			errcode = statement->errcode;

			if (crx_oci_fetch_sqltext_offset(statement, &sqltext, &error_offset)) {
				RETURN_FALSE;
			}
			goto go_out;
		}

		connection = (crx_oci_connection *) crex_fetch_resource_ex(arg, NULL, le_connection);
		if (connection) {
			errh = connection->err;
			errcode = connection->errcode;
			goto go_out;
		}

		connection = (crx_oci_connection *) crex_fetch_resource_ex(arg, NULL, le_pconnection);
		if (connection) {
			errh = connection->err;
			errcode = connection->errcode;
			goto go_out;
		}
	} else {
		errh = OCI_G(err);
		errcode = OCI_G(errcode);
	}

go_out:
	if (errcode == 0) { /* no error set in the handle */
		RETURN_FALSE;
	}

	if (!errh) {
		crx_error_docref(NULL, E_WARNING, "oci_error: unable to find error handle");
		RETURN_FALSE;
	}

	errcode = crx_oci_fetch_errmsg(errh, errbuf, sizeof(errbuf));

	if (errcode) {
		array_init(return_value);
		add_assoc_long(return_value, "code", errcode);
		/* TODO: avoid reallocation ??? */
		add_assoc_string(return_value, "message", (char*) errbuf);
		add_assoc_long(return_value, "offset", error_offset);
		add_assoc_string(return_value, "sqltext", sqltext ? (char *) sqltext : "");
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return the number of result columns in a statement */
CRX_FUNCTION(oci_num_fields)
{
	zval *z_statement;
	crx_oci_statement *statement;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	RETURN_LONG(statement->ncolumns);
}
/* }}} */

/* {{{ Parse a SQL or PL/SQL statement and return a statement resource */
CRX_FUNCTION(oci_parse)
{
	zval *z_connection;
	crx_oci_connection *connection;
	crx_oci_statement *statement;
	char *query;
	size_t query_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(query, query_len)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	statement = crx_oci_statement_create(connection, query, (int) query_len);

	if (statement) {
		RETURN_RES(statement->id);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Sets the number of rows to be prefetched on execute to prefetch_rows for stmt */
CRX_FUNCTION(oci_set_prefetch)
{
	zval *z_statement;
	crx_oci_statement *statement;
	crex_long size;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_LONG(size)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (size < 0) {
		crex_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	if (crx_oci_statement_set_prefetch(statement, (ub4)size)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the amount of LOB data to be prefetched when OCI LOB locators are fetched */
CRX_FUNCTION(oci_set_prefetch_lob)
{
	zval *z_statement;
	crex_long prefetch_lob_size;
#if (OCI_MAJOR_VERSION > 12 || (OCI_MAJOR_VERSION == 12 && OCI_MINOR_VERSION >= 2))
	crx_oci_statement *statement;
#endif	

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_statement)
		C_PARAM_LONG(prefetch_lob_size)
		CREX_PARSE_PARAMETERS_END();

#if (OCI_MAJOR_VERSION > 12 || (OCI_MAJOR_VERSION == 12 && OCI_MINOR_VERSION >= 2))
	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (prefetch_lob_size < 0) {
		crex_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	statement->prefetch_lob_size = (ub4) prefetch_lob_size;
	RETURN_TRUE;
#else
	/* Although the LOB prefetch feature was available in some earlier Oracle
	 * version, I don't consider it stable until 12.2 */
	crx_error_docref(NULL, E_NOTICE, "Unsupported with this version of Oracle Client");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ Sets the client identifier attribute on the connection */
CRX_FUNCTION(oci_set_client_identifier)
{
	zval *z_connection;
	crx_oci_connection *connection;
	char *client_id;
	size_t client_id_len;
	sword errstatus;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(client_id, client_id_len)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, ((dvoid *) connection->session, (ub4) OCI_HTYPE_SESSION, (dvoid *) client_id, (ub4) client_id_len, (ub4) OCI_ATTR_CLIENT_IDENTIFIER, connection->err));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		RETURN_FALSE;
	}

#ifdef HAVE_OCI8_DTRACE
	/* The alternatives to storing client_id like done below are
	   i) display it in a probe here in oci_set_client_identifier and
	   let the user D script correlate the connection address probe
	   argument and the client_id. This would likely require user D
	   script variables, which would use kernel memory.
	   ii) call OCIAttrGet for each probe definition that uses
	   client_id. This would be slower than storing it.
	*/

	if (connection->client_id) {
		pefree(connection->client_id, connection->is_persistent);
	}

	if (client_id) {
		/* this long winded copy allows compatibility with older CRX versions */
		connection->client_id = (char *)pemalloc(client_id_len+1, connection->is_persistent);
		memcpy(connection->client_id, client_id, client_id_len);
		connection->client_id[client_id_len] = '\0';
	} else {
		connection->client_id = NULL;
	}
#endif /* HAVE_OCI8_DTRACE */

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the edition attribute for all subsequent connections created */
CRX_FUNCTION(oci_set_edition)
{
	char *edition;
	size_t edition_len;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_STRING(edition, edition_len)
	CREX_PARSE_PARAMETERS_END();

	if (OCI_G(edition)) {
		efree(OCI_G(edition));
	}

	if (edition) {
		OCI_G(edition) = (char *)safe_emalloc(edition_len+1, sizeof(char), 0);
		memcpy(OCI_G(edition), edition, edition_len);
		OCI_G(edition)[edition_len] = '\0';
	} else {
		OCI_G(edition) = NULL;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the module attribute on the connection for end-to-end tracing */
CRX_FUNCTION(oci_set_module_name)
{
	zval *z_connection;
	char *module;
	size_t module_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(module, module_len)
	CREX_PARSE_PARAMETERS_END();

	crx_oci_connection *connection;
	sword errstatus;

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, ((dvoid *) connection->session, (ub4) OCI_HTYPE_SESSION, (dvoid *) module, (ub4) module_len, (ub4) OCI_ATTR_MODULE, connection->err));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the action attribute on the connection for end-to-end tracing */
CRX_FUNCTION(oci_set_action)
{
	zval *z_connection;
	char *action;
	size_t action_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(action, action_len)
	CREX_PARSE_PARAMETERS_END();

	crx_oci_connection *connection;
	sword errstatus;

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, ((dvoid *) connection->session, (ub4) OCI_HTYPE_SESSION, (dvoid *) action, (ub4) action_len, (ub4) OCI_ATTR_ACTION, connection->err));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the client info attribute on the connection  for end-to-end tracing */
CRX_FUNCTION(oci_set_client_info)
{
	zval *z_connection;
	char *client_info;
	size_t client_info_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(client_info, client_info_len)
	CREX_PARSE_PARAMETERS_END();

	crx_oci_connection *connection;
	sword errstatus;

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(errstatus, OCIAttrSet, ((dvoid *) connection->session, (ub4) OCI_HTYPE_SESSION, (dvoid *) client_info, (ub4) client_info_len, (ub4) OCI_ATTR_CLIENT_INFO, connection->err));

	if (errstatus != OCI_SUCCESS) {
		connection->errcode = crx_oci_error(connection->err, errstatus);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ Sets the "DB operation" on the connection for Oracle end-to-end tracing.
   For history, see Oracle bug 16695981 */
CRX_FUNCTION(oci_set_db_operation)
{
	zval *z_connection;
	char *dbop_name;
	size_t dbop_name_len;

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_STRING(dbop_name, dbop_name_len)
	CREX_PARSE_PARAMETERS_END();

#if (OCI_MAJOR_VERSION > 11)
	crx_oci_connection *connection;

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(OCI_G(errcode), OCIAttrSet, ((dvoid *) connection->session, (ub4) OCI_HTYPE_SESSION, (dvoid *) dbop_name, (ub4) dbop_name_len, (ub4) OCI_ATTR_DBOP, OCI_G(err)));

	if (OCI_G(errcode) != OCI_SUCCESS) {
		crx_oci_error(OCI_G(err), OCI_G(errcode));
		RETURN_FALSE;
	}
	RETURN_TRUE;
#else
	crx_error_docref(NULL, E_NOTICE, "Unsupported attribute type");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ */
CRX_FUNCTION(oci_set_call_timeout)
{
	zval *z_connection;
	crex_long call_timeout;  // milliseconds

	CREX_PARSE_PARAMETERS_START(2, 2)
		C_PARAM_RESOURCE(z_connection)
		C_PARAM_LONG(call_timeout)
	CREX_PARSE_PARAMETERS_END();

#if (OCI_MAJOR_VERSION >= 18)
	crx_oci_connection *connection;

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	CRX_OCI_CALL_RETURN(OCI_G(errcode), OCIAttrSet, ((dvoid *) connection->svc, (ub4) OCI_HTYPE_SVCCTX, (ub4 *) &call_timeout, 0, OCI_ATTR_CALL_TIMEOUT, OCI_G(err)));

	if (OCI_G(errcode) != OCI_SUCCESS) {
		crx_oci_error(OCI_G(err), OCI_G(errcode));
		RETURN_FALSE;
	}
	RETURN_TRUE;
#else
	crx_error_docref(NULL, E_NOTICE, "Unsupported with this version of Oracle Client");
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ Changes the password of an account */
CRX_FUNCTION(oci_password_change)
{
	zval *z_connection;
	char *user, *pass_old, *pass_new, *dbname;
	size_t user_len, pass_old_len, pass_new_len, dbname_len;
	crx_oci_connection *connection;

	if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "rsss", &z_connection, &user, &user_len, &pass_old, &pass_old_len, &pass_new, &pass_new_len) == SUCCESS) {
		CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

		if (!user_len) {
			crex_argument_value_error(2, "cannot be empty");
			RETURN_THROWS();
		}
		if (!pass_old_len) {
			crex_argument_value_error(3, "cannot be empty");
			RETURN_THROWS();
		}
		if (!pass_new_len) {
			crex_argument_value_error(4, "cannot be empty");
			RETURN_THROWS();
		}

		if (crx_oci_password_change(connection, user, (int) user_len, pass_old, (int) pass_old_len, pass_new, (int) pass_new_len)) {
			RETURN_FALSE;
		}
		RETURN_TRUE;
	} else if (crex_parse_parameters_ex(CREX_PARSE_PARAMS_QUIET, CREX_NUM_ARGS(), "ssss", &dbname, &dbname_len, &user, &user_len, &pass_old, &pass_old_len, &pass_new, &pass_new_len) == SUCCESS) {

		if (!user_len) {
			crex_argument_value_error(2, "cannot be empty");
			RETURN_THROWS();
		}
		if (!pass_old_len) {
			crex_argument_value_error(3, "cannot be empty");
			RETURN_THROWS();
		}
		if (!pass_new_len) {
			crex_argument_value_error(4, "cannot be empty");
			RETURN_THROWS();
		}

		connection = crx_oci_do_connect_ex(user, (int) user_len, pass_old, (int) pass_old_len, pass_new, (int) pass_new_len, dbname, (int) dbname_len, NULL, OCI_DEFAULT, 0, 0);
		if (!connection) {
			RETURN_FALSE;
		}
		RETURN_RES(connection->id);
	}
	WRONG_PARAM_COUNT;
}
/* }}} */

/* {{{ Return a new cursor (Statement-Handle) - use this to bind ref-cursors! */
CRX_FUNCTION(oci_new_cursor)
{
	zval *z_connection;
	crx_oci_connection *connection;
	crx_oci_statement *statement;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_connection)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	statement = crx_oci_statement_create(connection, NULL, 0);

	if (statement) {
		RETURN_RES(statement->id);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ Return a single column of result data */
CRX_FUNCTION(oci_result)
{
	crx_oci_out_column *column;

	column = crx_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
	if(column) {
		crx_oci_column_to_zval(column, return_value, 0);
	}
	else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return a string containing runtime client library version information */
CRX_FUNCTION(oci_client_version)
{
	char version[256];
	CREX_PARSE_PARAMETERS_NONE();

	crx_oci_client_get_version(version, sizeof(version));
	RETURN_STRING(version);
}
/* }}} */

/* {{{ Return a string containing server version information */
CRX_FUNCTION(oci_server_version)
{
	zval *z_connection;
	crx_oci_connection *connection;
	char version[256];
	crex_string *ret;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_connection)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (crx_oci_server_get_version(connection, version, sizeof(version))) {
		RETURN_FALSE;
	}

	ret = crex_string_init(version, strlen(version), 0);
	RETURN_STR(ret);
}
/* }}} */

/* {{{ Return the query type of an OCI statement */
CRX_FUNCTION(oci_statement_type)
{
	zval *z_statement;
	crx_oci_statement *statement;
	ub2 type;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_statement_get_type(statement, &type)) {
		RETURN_FALSE;
	}

	switch (type) {
		case OCI_STMT_SELECT:
			RETVAL_STRING("SELECT");
			break;
		case OCI_STMT_UPDATE:
			RETVAL_STRING("UPDATE");
			break;
		case OCI_STMT_DELETE:
			RETVAL_STRING("DELETE");
			break;
		case OCI_STMT_INSERT:
			RETVAL_STRING("INSERT");
			break;
		case OCI_STMT_CREATE:
			RETVAL_STRING("CREATE");
			break;
		case OCI_STMT_DROP:
			RETVAL_STRING("DROP");
			break;
		case OCI_STMT_ALTER:
			RETVAL_STRING("ALTER");
			break;
		case OCI_STMT_BEGIN:
			RETVAL_STRING("BEGIN");
			break;
		case OCI_STMT_DECLARE:
			RETVAL_STRING("DECLARE");
			break;
		case OCI_STMT_CALL:
			RETVAL_STRING("CALL");
			break;
		default:
			RETVAL_STRING("UNKNOWN");
	}
}
/* }}} */

/* {{{ Return the row count of an OCI statement */
CRX_FUNCTION(oci_num_rows)
{
	zval *z_statement;
	crx_oci_statement *statement;
	ub4 rowcount;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (crx_oci_statement_get_numrows(statement, &rowcount)) {
		RETURN_FALSE;
	}
	RETURN_LONG(rowcount);
}
/* }}} */

/* {{{ Deletes collection object*/
CRX_FUNCTION(oci_free_collection)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	crex_list_close(collection->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Append an object to the collection */
CRX_FUNCTION(oci_collection_append)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	char *value;
	size_t value_len;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Os", &z_collection, oci_coll_class_entry_ptr, &value, &value_len) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_append(collection, value, (int) value_len)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Retrieve the value at collection index ndx */
CRX_FUNCTION(oci_collection_element_get)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	crex_long element_index;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ol", &z_collection, oci_coll_class_entry_ptr, &element_index) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_element_get(collection, element_index, return_value)) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Assign a collection from another existing collection */
CRX_FUNCTION(oci_collection_assign)
{
	zval *tmp_dest, *tmp_from, *z_collection_dest, *z_collection_from;
	crx_oci_collection *collection_dest, *collection_from;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "OO", &z_collection_dest, oci_coll_class_entry_ptr, &z_collection_from, oci_coll_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp_dest = crex_hash_str_find(C_OBJPROP_P(z_collection_dest), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property. The first argument should be valid collection object");
		RETURN_FALSE;
	}

	if ((tmp_from = crex_hash_str_find(C_OBJPROP_P(z_collection_from), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property. The second argument should be valid collection object");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp_dest, collection_dest);
	CRX_OCI_ZVAL_TO_COLLECTION(tmp_from, collection_from);

	if (crx_oci_collection_assign(collection_dest, collection_from)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Assign element val to collection at index ndx */
CRX_FUNCTION(oci_collection_element_assign)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	size_t value_len;
	crex_long element_index;
	char *value;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ols", &z_collection, oci_coll_class_entry_ptr, &element_index, &value, &value_len) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_element_set(collection, element_index, value, (int) value_len)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Return the size of a collection */
CRX_FUNCTION(oci_collection_size)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	sb4 size = 0;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_size(collection, &size)) {
		RETURN_FALSE;
	}
	RETURN_LONG(size);
}
/* }}} */

/* {{{ Return the max value of a collection. For a varray this is the maximum length of the array */
CRX_FUNCTION(oci_collection_max)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	crex_long max;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_max(collection, &max)) {
		RETURN_FALSE;
	}
	RETURN_LONG(max);
}
/* }}} */

/* {{{ Trim num elements from the end of a collection */
CRX_FUNCTION(oci_collection_trim)
{
	zval *tmp, *z_collection;
	crx_oci_collection *collection;
	crex_long trim_size;

	if (crex_parse_method_parameters(CREX_NUM_ARGS(), getThis(), "Ol", &z_collection, oci_coll_class_entry_ptr, &trim_size) == FAILURE) {
		RETURN_THROWS();
	}

	if ((tmp = crex_hash_str_find(C_OBJPROP_P(z_collection), "collection", sizeof("collection")-1)) == NULL) {
		crx_error_docref(NULL, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}

	CRX_OCI_ZVAL_TO_COLLECTION(tmp, collection);

	if (crx_oci_collection_trim(collection, trim_size)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ Initialize a new collection */
CRX_FUNCTION(oci_new_collection)
{
	zval *z_connection;
	crx_oci_connection *connection;
	crx_oci_collection *collection;
	char *tdo, *schema = NULL;
	size_t tdo_len, schema_len = 0;

	if (crex_parse_parameters(CREX_NUM_ARGS(), "rs|s!", &z_connection, &tdo, &tdo_len, &schema, &schema_len) == FAILURE) {
		RETURN_THROWS();
	}

	CRX_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if ( (collection = crx_oci_collection_create(connection, tdo, (int) tdo_len, schema, (int) schema_len)) ) {
		object_init_ex(return_value, oci_coll_class_entry_ptr);
		add_property_resource(return_value, "collection", collection->id);
	}
	else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Get the next statement resource from an Oracle 12c PL/SQL Implicit Result Set */
CRX_FUNCTION(oci_get_implicit_resultset)
{
	zval *z_statement;
	crx_oci_statement *statement;
	crx_oci_statement *imp_statement;

	CREX_PARSE_PARAMETERS_START(1, 1)
		C_PARAM_RESOURCE(z_statement)
	CREX_PARSE_PARAMETERS_END();

	CRX_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	imp_statement = crx_oci_get_implicit_resultset(statement);

	if (imp_statement) {
		if (crx_oci_statement_execute(imp_statement, (ub4)OCI_DEFAULT))
			RETURN_FALSE;
		RETURN_RES(imp_statement->id);
	}
	RETURN_FALSE;
}

/* }}} */

#endif /* HAVE_OCI8 */
