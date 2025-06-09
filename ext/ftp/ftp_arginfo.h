/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 072486274a3361dee3655cfd046a293cfb8a2757 */

CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_ftp_connect, 0, 1, FTP\\Connection, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, port, IS_LONG, 0, "21")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "90")
CREX_END_ARG_INFO()

#if defined(HAVE_FTP_SSL)
CREX_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_ftp_ssl_connect, 0, 1, FTP\\Connection, MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, port, IS_LONG, 0, "21")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "90")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_login, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, username, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_pwd, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_cdup, 0, 1, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_chdir, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_exec, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_raw, 0, 2, IS_ARRAY, 1)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, command, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_mkdir, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_ftp_rmdir arginfo_ftp_chdir

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_chmod, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, permissions, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_alloc, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, response, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_nlist, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_rawlist, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, recursive, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

#define arginfo_ftp_mlsd arginfo_ftp_nlist

#define arginfo_ftp_systype arginfo_ftp_pwd

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_fget, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_nb_fget, 0, 3, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_pasv, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_get, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, local_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_nb_get, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, local_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_nb_continue, 0, 1, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_fput, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_nb_fput, 0, 3, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_INFO(0, stream)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_put, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, local_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_append, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, local_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_nb_put, 0, 3, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, remote_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, local_filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "FTP_BINARY")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_size, 0, 2, IS_LONG, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_ftp_mdtm arginfo_ftp_size

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_rename, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, from, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_delete, 0, 2, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_ftp_site arginfo_ftp_exec

#define arginfo_ftp_close arginfo_ftp_cdup

#define arginfo_ftp_quit arginfo_ftp_cdup

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ftp_set_option, 0, 3, _IS_BOOL, 0)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
	CREX_ARG_INFO(0, value)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ftp_get_option, 0, 2, MAY_BE_LONG|MAY_BE_BOOL)
	CREX_ARG_OBJ_INFO(0, ftp, FTP\\Connection, 0)
	CREX_ARG_TYPE_INFO(0, option, IS_LONG, 0)
CREX_END_ARG_INFO()


CREX_FUNCTION(ftp_connect);
#if defined(HAVE_FTP_SSL)
CREX_FUNCTION(ftp_ssl_connect);
#endif
CREX_FUNCTION(ftp_login);
CREX_FUNCTION(ftp_pwd);
CREX_FUNCTION(ftp_cdup);
CREX_FUNCTION(ftp_chdir);
CREX_FUNCTION(ftp_exec);
CREX_FUNCTION(ftp_raw);
CREX_FUNCTION(ftp_mkdir);
CREX_FUNCTION(ftp_rmdir);
CREX_FUNCTION(ftp_chmod);
CREX_FUNCTION(ftp_alloc);
CREX_FUNCTION(ftp_nlist);
CREX_FUNCTION(ftp_rawlist);
CREX_FUNCTION(ftp_mlsd);
CREX_FUNCTION(ftp_systype);
CREX_FUNCTION(ftp_fget);
CREX_FUNCTION(ftp_nb_fget);
CREX_FUNCTION(ftp_pasv);
CREX_FUNCTION(ftp_get);
CREX_FUNCTION(ftp_nb_get);
CREX_FUNCTION(ftp_nb_continue);
CREX_FUNCTION(ftp_fput);
CREX_FUNCTION(ftp_nb_fput);
CREX_FUNCTION(ftp_put);
CREX_FUNCTION(ftp_append);
CREX_FUNCTION(ftp_nb_put);
CREX_FUNCTION(ftp_size);
CREX_FUNCTION(ftp_mdtm);
CREX_FUNCTION(ftp_rename);
CREX_FUNCTION(ftp_delete);
CREX_FUNCTION(ftp_site);
CREX_FUNCTION(ftp_close);
CREX_FUNCTION(ftp_set_option);
CREX_FUNCTION(ftp_get_option);


static const crex_function_entry ext_functions[] = {
	CREX_FE(ftp_connect, arginfo_ftp_connect)
#if defined(HAVE_FTP_SSL)
	CREX_FE(ftp_ssl_connect, arginfo_ftp_ssl_connect)
#endif
	CREX_FE(ftp_login, arginfo_ftp_login)
	CREX_FE(ftp_pwd, arginfo_ftp_pwd)
	CREX_FE(ftp_cdup, arginfo_ftp_cdup)
	CREX_FE(ftp_chdir, arginfo_ftp_chdir)
	CREX_FE(ftp_exec, arginfo_ftp_exec)
	CREX_FE(ftp_raw, arginfo_ftp_raw)
	CREX_FE(ftp_mkdir, arginfo_ftp_mkdir)
	CREX_FE(ftp_rmdir, arginfo_ftp_rmdir)
	CREX_FE(ftp_chmod, arginfo_ftp_chmod)
	CREX_FE(ftp_alloc, arginfo_ftp_alloc)
	CREX_FE(ftp_nlist, arginfo_ftp_nlist)
	CREX_FE(ftp_rawlist, arginfo_ftp_rawlist)
	CREX_FE(ftp_mlsd, arginfo_ftp_mlsd)
	CREX_FE(ftp_systype, arginfo_ftp_systype)
	CREX_FE(ftp_fget, arginfo_ftp_fget)
	CREX_FE(ftp_nb_fget, arginfo_ftp_nb_fget)
	CREX_FE(ftp_pasv, arginfo_ftp_pasv)
	CREX_FE(ftp_get, arginfo_ftp_get)
	CREX_FE(ftp_nb_get, arginfo_ftp_nb_get)
	CREX_FE(ftp_nb_continue, arginfo_ftp_nb_continue)
	CREX_FE(ftp_fput, arginfo_ftp_fput)
	CREX_FE(ftp_nb_fput, arginfo_ftp_nb_fput)
	CREX_FE(ftp_put, arginfo_ftp_put)
	CREX_FE(ftp_append, arginfo_ftp_append)
	CREX_FE(ftp_nb_put, arginfo_ftp_nb_put)
	CREX_FE(ftp_size, arginfo_ftp_size)
	CREX_FE(ftp_mdtm, arginfo_ftp_mdtm)
	CREX_FE(ftp_rename, arginfo_ftp_rename)
	CREX_FE(ftp_delete, arginfo_ftp_delete)
	CREX_FE(ftp_site, arginfo_ftp_site)
	CREX_FE(ftp_close, arginfo_ftp_close)
	CREX_FALIAS(ftp_quit, ftp_close, arginfo_ftp_quit)
	CREX_FE(ftp_set_option, arginfo_ftp_set_option)
	CREX_FE(ftp_get_option, arginfo_ftp_get_option)
	CREX_FE_END
};


static const crex_function_entry class_FTP_Connection_methods[] = {
	CREX_FE_END
};

static void register_ftp_symbols(int module_number)
{
	REGISTER_LONG_CONSTANT("FTP_ASCII", FTPTYPE_ASCII, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_TEXT", FTPTYPE_ASCII, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_BINARY", FTPTYPE_IMAGE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_IMAGE", FTPTYPE_IMAGE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_AUTORESUME", CRX_FTP_AUTORESUME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_TIMEOUT_SEC", CRX_FTP_OPT_TIMEOUT_SEC, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_AUTOSEEK", CRX_FTP_OPT_AUTOSEEK, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_USEPASVADDRESS", CRX_FTP_OPT_USEPASVADDRESS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_FAILED", CRX_FTP_FAILED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_FINISHED", CRX_FTP_FINISHED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FTP_MOREDATA", CRX_FTP_MOREDATA, CONST_PERSISTENT);


	crex_add_parameter_attribute(crex_hash_str_find_ptr(CG(function_table), "ftp_login", sizeof("ftp_login") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
}

static crex_class_entry *register_class_FTP_Connection(void)
{
	crex_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "FTP", "Connection", class_FTP_Connection_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_FINAL|CREX_ACC_NO_DYNAMIC_PROPERTIES|CREX_ACC_NOT_SERIALIZABLE;

	return class_entry;
}
