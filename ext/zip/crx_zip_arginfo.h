/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 95564c667a51a548f5d43025c90546b991970ddd */

CREX_BEGIN_ARG_INFO_EX(arginfo_zip_open, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_zip_close, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, zip)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_zip_read, 0, 0, 1)
	CREX_ARG_INFO(0, zip)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_zip_entry_open, 0, 2, _IS_BOOL, 0)
	CREX_ARG_INFO(0, zip_dp)
	CREX_ARG_INFO(0, zip_entry)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"rb\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_zip_entry_close, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, zip_entry)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zip_entry_read, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, zip_entry)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, len, IS_LONG, 0, "1024")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zip_entry_name, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_INFO(0, zip_entry)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_zip_entry_compressedsize, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_INFO(0, zip_entry)
CREX_END_ARG_INFO()

#define arginfo_zip_entry_filesize arginfo_zip_entry_compressedsize

#define arginfo_zip_entry_compressionmethod arginfo_zip_entry_name

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_open, 0, 1, MAY_BE_BOOL|MAY_BE_LONG)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setPassword, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, password, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_close, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_getStatusString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_clearError, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_addEmptyDir, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, dirname, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_addFromString, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, content, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ZipArchive::FL_OVERWRITE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_addFile, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filepath, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, entryname, IS_STRING, 0, "\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "ZipArchive::LENGTH_TO_END")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "ZipArchive::FL_OVERWRITE")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_replaceFile, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filepath, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, start, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "ZipArchive::LENGTH_TO_END")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_addGlob, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_addPattern, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, path, IS_STRING, 0, "\".\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_renameIndex, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, new_name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_renameName, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, new_name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setArchiveComment, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, comment, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_getArchiveComment, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setArchiveFlag, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, flag, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, value, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_getArchiveFlag, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, flag, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setCommentIndex, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, comment, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setCommentName, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, comment, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(HAVE_SET_MTIME)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setMtimeIndex, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_SET_MTIME)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setMtimeName, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, timestamp, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_getCommentIndex, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_getCommentName, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_deleteIndex, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_deleteName, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_statName, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_statIndex, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_locateName, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_ZipArchive_getNameIndex arginfo_class_ZipArchive_getCommentIndex

#define arginfo_class_ZipArchive_unchangeArchive arginfo_class_ZipArchive_close

#define arginfo_class_ZipArchive_unchangeAll arginfo_class_ZipArchive_close

#define arginfo_class_ZipArchive_unchangeIndex arginfo_class_ZipArchive_deleteIndex

#define arginfo_class_ZipArchive_unchangeName arginfo_class_ZipArchive_deleteName

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_extractTo, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, pathto, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, files, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_getFromName, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, len, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_ZipArchive_getFromIndex, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, len, IS_LONG, 0, "0")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ZipArchive_getStreamIndex, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ZipArchive_getStreamName, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_ZipArchive_getStream, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
CREX_END_ARG_INFO()

#if defined(ZIP_OPSYS_DEFAULT)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setExternalAttributesName, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, opsys, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(ZIP_OPSYS_DEFAULT)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setExternalAttributesIndex, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, opsys, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, attr, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(ZIP_OPSYS_DEFAULT)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_getExternalAttributesName, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_INFO(1, opsys)
	CREX_ARG_INFO(1, attr)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

#if defined(ZIP_OPSYS_DEFAULT)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_getExternalAttributesIndex, 0, 3, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_INFO(1, opsys)
	CREX_ARG_INFO(1, attr)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setCompressionName, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compflags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setCompressionIndex, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compflags, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#if defined(HAVE_ENCRYPTION)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setEncryptionName, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, password, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_ENCRYPTION)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_setEncryptionIndex, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, password, IS_STRING, 1, "null")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_PROGRESS_CALLBACK)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_registerProgressCallback, 0, 2, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, rate, IS_DOUBLE, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_CANCEL_CALLBACK)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_registerCancelCallback, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_METHOD_SUPPORTED)
CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ZipArchive_isCompressionMethodSupported, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, method, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enc, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_METHOD_SUPPORTED)
#define arginfo_class_ZipArchive_isEncryptionMethodSupported arginfo_class_ZipArchive_isCompressionMethodSupported
#endif


CREX_FUNCTION(zip_open);
CREX_FUNCTION(zip_close);
CREX_FUNCTION(zip_read);
CREX_FUNCTION(zip_entry_open);
CREX_FUNCTION(zip_entry_close);
CREX_FUNCTION(zip_entry_read);
CREX_FUNCTION(zip_entry_name);
CREX_FUNCTION(zip_entry_compressedsize);
CREX_FUNCTION(zip_entry_filesize);
CREX_FUNCTION(zip_entry_compressionmethod);
CREX_METHOD(ZipArchive, open);
CREX_METHOD(ZipArchive, setPassword);
CREX_METHOD(ZipArchive, close);
CREX_METHOD(ZipArchive, count);
CREX_METHOD(ZipArchive, getStatusString);
CREX_METHOD(ZipArchive, clearError);
CREX_METHOD(ZipArchive, addEmptyDir);
CREX_METHOD(ZipArchive, addFromString);
CREX_METHOD(ZipArchive, addFile);
CREX_METHOD(ZipArchive, replaceFile);
CREX_METHOD(ZipArchive, addGlob);
CREX_METHOD(ZipArchive, addPattern);
CREX_METHOD(ZipArchive, renameIndex);
CREX_METHOD(ZipArchive, renameName);
CREX_METHOD(ZipArchive, setArchiveComment);
CREX_METHOD(ZipArchive, getArchiveComment);
CREX_METHOD(ZipArchive, setArchiveFlag);
CREX_METHOD(ZipArchive, getArchiveFlag);
CREX_METHOD(ZipArchive, setCommentIndex);
CREX_METHOD(ZipArchive, setCommentName);
#if defined(HAVE_SET_MTIME)
CREX_METHOD(ZipArchive, setMtimeIndex);
#endif
#if defined(HAVE_SET_MTIME)
CREX_METHOD(ZipArchive, setMtimeName);
#endif
CREX_METHOD(ZipArchive, getCommentIndex);
CREX_METHOD(ZipArchive, getCommentName);
CREX_METHOD(ZipArchive, deleteIndex);
CREX_METHOD(ZipArchive, deleteName);
CREX_METHOD(ZipArchive, statName);
CREX_METHOD(ZipArchive, statIndex);
CREX_METHOD(ZipArchive, locateName);
CREX_METHOD(ZipArchive, getNameIndex);
CREX_METHOD(ZipArchive, unchangeArchive);
CREX_METHOD(ZipArchive, unchangeAll);
CREX_METHOD(ZipArchive, unchangeIndex);
CREX_METHOD(ZipArchive, unchangeName);
CREX_METHOD(ZipArchive, extractTo);
CREX_METHOD(ZipArchive, getFromName);
CREX_METHOD(ZipArchive, getFromIndex);
CREX_METHOD(ZipArchive, getStreamIndex);
CREX_METHOD(ZipArchive, getStreamName);
CREX_METHOD(ZipArchive, getStream);
#if defined(ZIP_OPSYS_DEFAULT)
CREX_METHOD(ZipArchive, setExternalAttributesName);
#endif
#if defined(ZIP_OPSYS_DEFAULT)
CREX_METHOD(ZipArchive, setExternalAttributesIndex);
#endif
#if defined(ZIP_OPSYS_DEFAULT)
CREX_METHOD(ZipArchive, getExternalAttributesName);
#endif
#if defined(ZIP_OPSYS_DEFAULT)
CREX_METHOD(ZipArchive, getExternalAttributesIndex);
#endif
CREX_METHOD(ZipArchive, setCompressionName);
CREX_METHOD(ZipArchive, setCompressionIndex);
#if defined(HAVE_ENCRYPTION)
CREX_METHOD(ZipArchive, setEncryptionName);
#endif
#if defined(HAVE_ENCRYPTION)
CREX_METHOD(ZipArchive, setEncryptionIndex);
#endif
#if defined(HAVE_PROGRESS_CALLBACK)
CREX_METHOD(ZipArchive, registerProgressCallback);
#endif
#if defined(HAVE_CANCEL_CALLBACK)
CREX_METHOD(ZipArchive, registerCancelCallback);
#endif
#if defined(HAVE_METHOD_SUPPORTED)
CREX_METHOD(ZipArchive, isCompressionMethodSupported);
#endif
#if defined(HAVE_METHOD_SUPPORTED)
CREX_METHOD(ZipArchive, isEncryptionMethodSupported);
#endif


static const crex_function_entry ext_functions[] = {
	CREX_DEP_FE(zip_open, arginfo_zip_open)
	CREX_DEP_FE(zip_close, arginfo_zip_close)
	CREX_DEP_FE(zip_read, arginfo_zip_read)
	CREX_DEP_FE(zip_entry_open, arginfo_zip_entry_open)
	CREX_DEP_FE(zip_entry_close, arginfo_zip_entry_close)
	CREX_DEP_FE(zip_entry_read, arginfo_zip_entry_read)
	CREX_DEP_FE(zip_entry_name, arginfo_zip_entry_name)
	CREX_DEP_FE(zip_entry_compressedsize, arginfo_zip_entry_compressedsize)
	CREX_DEP_FE(zip_entry_filesize, arginfo_zip_entry_filesize)
	CREX_DEP_FE(zip_entry_compressionmethod, arginfo_zip_entry_compressionmethod)
	CREX_FE_END
};


static const crex_function_entry class_ZipArchive_methods[] = {
	CREX_ME(ZipArchive, open, arginfo_class_ZipArchive_open, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setPassword, arginfo_class_ZipArchive_setPassword, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, close, arginfo_class_ZipArchive_close, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, count, arginfo_class_ZipArchive_count, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getStatusString, arginfo_class_ZipArchive_getStatusString, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, clearError, arginfo_class_ZipArchive_clearError, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, addEmptyDir, arginfo_class_ZipArchive_addEmptyDir, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, addFromString, arginfo_class_ZipArchive_addFromString, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, addFile, arginfo_class_ZipArchive_addFile, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, replaceFile, arginfo_class_ZipArchive_replaceFile, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, addGlob, arginfo_class_ZipArchive_addGlob, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, addPattern, arginfo_class_ZipArchive_addPattern, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, renameIndex, arginfo_class_ZipArchive_renameIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, renameName, arginfo_class_ZipArchive_renameName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setArchiveComment, arginfo_class_ZipArchive_setArchiveComment, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getArchiveComment, arginfo_class_ZipArchive_getArchiveComment, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setArchiveFlag, arginfo_class_ZipArchive_setArchiveFlag, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getArchiveFlag, arginfo_class_ZipArchive_getArchiveFlag, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setCommentIndex, arginfo_class_ZipArchive_setCommentIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setCommentName, arginfo_class_ZipArchive_setCommentName, CREX_ACC_PUBLIC)
#if defined(HAVE_SET_MTIME)
	CREX_ME(ZipArchive, setMtimeIndex, arginfo_class_ZipArchive_setMtimeIndex, CREX_ACC_PUBLIC)
#endif
#if defined(HAVE_SET_MTIME)
	CREX_ME(ZipArchive, setMtimeName, arginfo_class_ZipArchive_setMtimeName, CREX_ACC_PUBLIC)
#endif
	CREX_ME(ZipArchive, getCommentIndex, arginfo_class_ZipArchive_getCommentIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getCommentName, arginfo_class_ZipArchive_getCommentName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, deleteIndex, arginfo_class_ZipArchive_deleteIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, deleteName, arginfo_class_ZipArchive_deleteName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, statName, arginfo_class_ZipArchive_statName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, statIndex, arginfo_class_ZipArchive_statIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, locateName, arginfo_class_ZipArchive_locateName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getNameIndex, arginfo_class_ZipArchive_getNameIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, unchangeArchive, arginfo_class_ZipArchive_unchangeArchive, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, unchangeAll, arginfo_class_ZipArchive_unchangeAll, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, unchangeIndex, arginfo_class_ZipArchive_unchangeIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, unchangeName, arginfo_class_ZipArchive_unchangeName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, extractTo, arginfo_class_ZipArchive_extractTo, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getFromName, arginfo_class_ZipArchive_getFromName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getFromIndex, arginfo_class_ZipArchive_getFromIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getStreamIndex, arginfo_class_ZipArchive_getStreamIndex, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getStreamName, arginfo_class_ZipArchive_getStreamName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, getStream, arginfo_class_ZipArchive_getStream, CREX_ACC_PUBLIC)
#if defined(ZIP_OPSYS_DEFAULT)
	CREX_ME(ZipArchive, setExternalAttributesName, arginfo_class_ZipArchive_setExternalAttributesName, CREX_ACC_PUBLIC)
#endif
#if defined(ZIP_OPSYS_DEFAULT)
	CREX_ME(ZipArchive, setExternalAttributesIndex, arginfo_class_ZipArchive_setExternalAttributesIndex, CREX_ACC_PUBLIC)
#endif
#if defined(ZIP_OPSYS_DEFAULT)
	CREX_ME(ZipArchive, getExternalAttributesName, arginfo_class_ZipArchive_getExternalAttributesName, CREX_ACC_PUBLIC)
#endif
#if defined(ZIP_OPSYS_DEFAULT)
	CREX_ME(ZipArchive, getExternalAttributesIndex, arginfo_class_ZipArchive_getExternalAttributesIndex, CREX_ACC_PUBLIC)
#endif
	CREX_ME(ZipArchive, setCompressionName, arginfo_class_ZipArchive_setCompressionName, CREX_ACC_PUBLIC)
	CREX_ME(ZipArchive, setCompressionIndex, arginfo_class_ZipArchive_setCompressionIndex, CREX_ACC_PUBLIC)
#if defined(HAVE_ENCRYPTION)
	CREX_ME(ZipArchive, setEncryptionName, arginfo_class_ZipArchive_setEncryptionName, CREX_ACC_PUBLIC)
#endif
#if defined(HAVE_ENCRYPTION)
	CREX_ME(ZipArchive, setEncryptionIndex, arginfo_class_ZipArchive_setEncryptionIndex, CREX_ACC_PUBLIC)
#endif
#if defined(HAVE_PROGRESS_CALLBACK)
	CREX_ME(ZipArchive, registerProgressCallback, arginfo_class_ZipArchive_registerProgressCallback, CREX_ACC_PUBLIC)
#endif
#if defined(HAVE_CANCEL_CALLBACK)
	CREX_ME(ZipArchive, registerCancelCallback, arginfo_class_ZipArchive_registerCancelCallback, CREX_ACC_PUBLIC)
#endif
#if defined(HAVE_METHOD_SUPPORTED)
	CREX_ME(ZipArchive, isCompressionMethodSupported, arginfo_class_ZipArchive_isCompressionMethodSupported, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
#if defined(HAVE_METHOD_SUPPORTED)
	CREX_ME(ZipArchive, isEncryptionMethodSupported, arginfo_class_ZipArchive_isEncryptionMethodSupported, CREX_ACC_PUBLIC|CREX_ACC_STATIC)
#endif
	CREX_FE_END
};

static crex_class_entry *register_class_ZipArchive(crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ZipArchive", class_ZipArchive_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	crex_class_implements(class_entry, 1, class_entry_Countable);

	zval const_CREATE_value;
	ZVAL_LONG(&const_CREATE_value, ZIP_CREATE);
	crex_string *const_CREATE_name = crex_string_init_interned("CREATE", sizeof("CREATE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CREATE_name, &const_CREATE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CREATE_name);

	zval const_EXCL_value;
	ZVAL_LONG(&const_EXCL_value, ZIP_EXCL);
	crex_string *const_EXCL_name = crex_string_init_interned("EXCL", sizeof("EXCL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EXCL_name, &const_EXCL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EXCL_name);

	zval const_CHECKCONS_value;
	ZVAL_LONG(&const_CHECKCONS_value, ZIP_CHECKCONS);
	crex_string *const_CHECKCONS_name = crex_string_init_interned("CHECKCONS", sizeof("CHECKCONS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CHECKCONS_name, &const_CHECKCONS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CHECKCONS_name);

	zval const_OVERWRITE_value;
	ZVAL_LONG(&const_OVERWRITE_value, ZIP_OVERWRITE);
	crex_string *const_OVERWRITE_name = crex_string_init_interned("OVERWRITE", sizeof("OVERWRITE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OVERWRITE_name, &const_OVERWRITE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OVERWRITE_name);
#if defined(ZIP_RDONLY)

	zval const_RDONLY_value;
	ZVAL_LONG(&const_RDONLY_value, ZIP_RDONLY);
	crex_string *const_RDONLY_name = crex_string_init_interned("RDONLY", sizeof("RDONLY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_RDONLY_name, &const_RDONLY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_RDONLY_name);
#endif

	zval const_FL_NOCASE_value;
	ZVAL_LONG(&const_FL_NOCASE_value, ZIP_FL_NOCASE);
	crex_string *const_FL_NOCASE_name = crex_string_init_interned("FL_NOCASE", sizeof("FL_NOCASE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_NOCASE_name, &const_FL_NOCASE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_NOCASE_name);

	zval const_FL_NODIR_value;
	ZVAL_LONG(&const_FL_NODIR_value, ZIP_FL_NODIR);
	crex_string *const_FL_NODIR_name = crex_string_init_interned("FL_NODIR", sizeof("FL_NODIR") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_NODIR_name, &const_FL_NODIR_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_NODIR_name);

	zval const_FL_COMPRESSED_value;
	ZVAL_LONG(&const_FL_COMPRESSED_value, ZIP_FL_COMPRESSED);
	crex_string *const_FL_COMPRESSED_name = crex_string_init_interned("FL_COMPRESSED", sizeof("FL_COMPRESSED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_COMPRESSED_name, &const_FL_COMPRESSED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_COMPRESSED_name);

	zval const_FL_UNCHANGED_value;
	ZVAL_LONG(&const_FL_UNCHANGED_value, ZIP_FL_UNCHANGED);
	crex_string *const_FL_UNCHANGED_name = crex_string_init_interned("FL_UNCHANGED", sizeof("FL_UNCHANGED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_UNCHANGED_name, &const_FL_UNCHANGED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_UNCHANGED_name);
#if defined(ZIP_FL_RECOMPRESS)

	zval const_FL_RECOMPRESS_value;
	ZVAL_LONG(&const_FL_RECOMPRESS_value, ZIP_FL_RECOMPRESS);
	crex_string *const_FL_RECOMPRESS_name = crex_string_init_interned("FL_RECOMPRESS", sizeof("FL_RECOMPRESS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_RECOMPRESS_name, &const_FL_RECOMPRESS_value, CREX_ACC_PUBLIC|CREX_ACC_DEPRECATED, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_RECOMPRESS_name);
#endif

	zval const_FL_ENCRYPTED_value;
	ZVAL_LONG(&const_FL_ENCRYPTED_value, ZIP_FL_ENCRYPTED);
	crex_string *const_FL_ENCRYPTED_name = crex_string_init_interned("FL_ENCRYPTED", sizeof("FL_ENCRYPTED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENCRYPTED_name, &const_FL_ENCRYPTED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENCRYPTED_name);

	zval const_FL_OVERWRITE_value;
	ZVAL_LONG(&const_FL_OVERWRITE_value, ZIP_FL_OVERWRITE);
	crex_string *const_FL_OVERWRITE_name = crex_string_init_interned("FL_OVERWRITE", sizeof("FL_OVERWRITE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_OVERWRITE_name, &const_FL_OVERWRITE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_OVERWRITE_name);

	zval const_FL_LOCAL_value;
	ZVAL_LONG(&const_FL_LOCAL_value, ZIP_FL_LOCAL);
	crex_string *const_FL_LOCAL_name = crex_string_init_interned("FL_LOCAL", sizeof("FL_LOCAL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_LOCAL_name, &const_FL_LOCAL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_LOCAL_name);

	zval const_FL_CENTRAL_value;
	ZVAL_LONG(&const_FL_CENTRAL_value, ZIP_FL_CENTRAL);
	crex_string *const_FL_CENTRAL_name = crex_string_init_interned("FL_CENTRAL", sizeof("FL_CENTRAL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_CENTRAL_name, &const_FL_CENTRAL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_CENTRAL_name);

	zval const_FL_ENC_GUESS_value;
	ZVAL_LONG(&const_FL_ENC_GUESS_value, ZIP_FL_ENC_GUESS);
	crex_string *const_FL_ENC_GUESS_name = crex_string_init_interned("FL_ENC_GUESS", sizeof("FL_ENC_GUESS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENC_GUESS_name, &const_FL_ENC_GUESS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENC_GUESS_name);

	zval const_FL_ENC_RAW_value;
	ZVAL_LONG(&const_FL_ENC_RAW_value, ZIP_FL_ENC_RAW);
	crex_string *const_FL_ENC_RAW_name = crex_string_init_interned("FL_ENC_RAW", sizeof("FL_ENC_RAW") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENC_RAW_name, &const_FL_ENC_RAW_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENC_RAW_name);

	zval const_FL_ENC_STRICT_value;
	ZVAL_LONG(&const_FL_ENC_STRICT_value, ZIP_FL_ENC_STRICT);
	crex_string *const_FL_ENC_STRICT_name = crex_string_init_interned("FL_ENC_STRICT", sizeof("FL_ENC_STRICT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENC_STRICT_name, &const_FL_ENC_STRICT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENC_STRICT_name);

	zval const_FL_ENC_UTF_8_value;
	ZVAL_LONG(&const_FL_ENC_UTF_8_value, ZIP_FL_ENC_UTF_8);
	crex_string *const_FL_ENC_UTF_8_name = crex_string_init_interned("FL_ENC_UTF_8", sizeof("FL_ENC_UTF_8") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENC_UTF_8_name, &const_FL_ENC_UTF_8_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENC_UTF_8_name);

	zval const_FL_ENC_CP437_value;
	ZVAL_LONG(&const_FL_ENC_CP437_value, ZIP_FL_ENC_CP437);
	crex_string *const_FL_ENC_CP437_name = crex_string_init_interned("FL_ENC_CP437", sizeof("FL_ENC_CP437") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_ENC_CP437_name, &const_FL_ENC_CP437_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_ENC_CP437_name);

	zval const_FL_OPEN_FILE_NOW_value;
	ZVAL_LONG(&const_FL_OPEN_FILE_NOW_value, ZIP_FL_OPEN_FILE_NOW);
	crex_string *const_FL_OPEN_FILE_NOW_name = crex_string_init_interned("FL_OPEN_FILE_NOW", sizeof("FL_OPEN_FILE_NOW") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_FL_OPEN_FILE_NOW_name, &const_FL_OPEN_FILE_NOW_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_FL_OPEN_FILE_NOW_name);

	zval const_CM_DEFAULT_value;
	ZVAL_LONG(&const_CM_DEFAULT_value, ZIP_CM_DEFAULT);
	crex_string *const_CM_DEFAULT_name = crex_string_init_interned("CM_DEFAULT", sizeof("CM_DEFAULT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_DEFAULT_name, &const_CM_DEFAULT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_DEFAULT_name);

	zval const_CM_STORE_value;
	ZVAL_LONG(&const_CM_STORE_value, ZIP_CM_STORE);
	crex_string *const_CM_STORE_name = crex_string_init_interned("CM_STORE", sizeof("CM_STORE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_STORE_name, &const_CM_STORE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_STORE_name);

	zval const_CM_SHRINK_value;
	ZVAL_LONG(&const_CM_SHRINK_value, ZIP_CM_SHRINK);
	crex_string *const_CM_SHRINK_name = crex_string_init_interned("CM_SHRINK", sizeof("CM_SHRINK") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_SHRINK_name, &const_CM_SHRINK_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_SHRINK_name);

	zval const_CM_REDUCE_1_value;
	ZVAL_LONG(&const_CM_REDUCE_1_value, ZIP_CM_REDUCE_1);
	crex_string *const_CM_REDUCE_1_name = crex_string_init_interned("CM_REDUCE_1", sizeof("CM_REDUCE_1") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_REDUCE_1_name, &const_CM_REDUCE_1_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_REDUCE_1_name);

	zval const_CM_REDUCE_2_value;
	ZVAL_LONG(&const_CM_REDUCE_2_value, ZIP_CM_REDUCE_2);
	crex_string *const_CM_REDUCE_2_name = crex_string_init_interned("CM_REDUCE_2", sizeof("CM_REDUCE_2") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_REDUCE_2_name, &const_CM_REDUCE_2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_REDUCE_2_name);

	zval const_CM_REDUCE_3_value;
	ZVAL_LONG(&const_CM_REDUCE_3_value, ZIP_CM_REDUCE_3);
	crex_string *const_CM_REDUCE_3_name = crex_string_init_interned("CM_REDUCE_3", sizeof("CM_REDUCE_3") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_REDUCE_3_name, &const_CM_REDUCE_3_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_REDUCE_3_name);

	zval const_CM_REDUCE_4_value;
	ZVAL_LONG(&const_CM_REDUCE_4_value, ZIP_CM_REDUCE_4);
	crex_string *const_CM_REDUCE_4_name = crex_string_init_interned("CM_REDUCE_4", sizeof("CM_REDUCE_4") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_REDUCE_4_name, &const_CM_REDUCE_4_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_REDUCE_4_name);

	zval const_CM_IMPLODE_value;
	ZVAL_LONG(&const_CM_IMPLODE_value, ZIP_CM_IMPLODE);
	crex_string *const_CM_IMPLODE_name = crex_string_init_interned("CM_IMPLODE", sizeof("CM_IMPLODE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_IMPLODE_name, &const_CM_IMPLODE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_IMPLODE_name);

	zval const_CM_DEFLATE_value;
	ZVAL_LONG(&const_CM_DEFLATE_value, ZIP_CM_DEFLATE);
	crex_string *const_CM_DEFLATE_name = crex_string_init_interned("CM_DEFLATE", sizeof("CM_DEFLATE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_DEFLATE_name, &const_CM_DEFLATE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_DEFLATE_name);

	zval const_CM_DEFLATE64_value;
	ZVAL_LONG(&const_CM_DEFLATE64_value, ZIP_CM_DEFLATE64);
	crex_string *const_CM_DEFLATE64_name = crex_string_init_interned("CM_DEFLATE64", sizeof("CM_DEFLATE64") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_DEFLATE64_name, &const_CM_DEFLATE64_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_DEFLATE64_name);

	zval const_CM_PKWARE_IMPLODE_value;
	ZVAL_LONG(&const_CM_PKWARE_IMPLODE_value, ZIP_CM_PKWARE_IMPLODE);
	crex_string *const_CM_PKWARE_IMPLODE_name = crex_string_init_interned("CM_PKWARE_IMPLODE", sizeof("CM_PKWARE_IMPLODE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_PKWARE_IMPLODE_name, &const_CM_PKWARE_IMPLODE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_PKWARE_IMPLODE_name);

	zval const_CM_BZIP2_value;
	ZVAL_LONG(&const_CM_BZIP2_value, ZIP_CM_BZIP2);
	crex_string *const_CM_BZIP2_name = crex_string_init_interned("CM_BZIP2", sizeof("CM_BZIP2") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_BZIP2_name, &const_CM_BZIP2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_BZIP2_name);

	zval const_CM_LZMA_value;
	ZVAL_LONG(&const_CM_LZMA_value, ZIP_CM_LZMA);
	crex_string *const_CM_LZMA_name = crex_string_init_interned("CM_LZMA", sizeof("CM_LZMA") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_LZMA_name, &const_CM_LZMA_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_LZMA_name);
#if defined(ZIP_CM_LZMA2)

	zval const_CM_LZMA2_value;
	ZVAL_LONG(&const_CM_LZMA2_value, ZIP_CM_LZMA2);
	crex_string *const_CM_LZMA2_name = crex_string_init_interned("CM_LZMA2", sizeof("CM_LZMA2") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_LZMA2_name, &const_CM_LZMA2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_LZMA2_name);
#endif
#if defined(ZIP_CM_ZSTD)

	zval const_CM_ZSTD_value;
	ZVAL_LONG(&const_CM_ZSTD_value, ZIP_CM_ZSTD);
	crex_string *const_CM_ZSTD_name = crex_string_init_interned("CM_ZSTD", sizeof("CM_ZSTD") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_ZSTD_name, &const_CM_ZSTD_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_ZSTD_name);
#endif
#if defined(ZIP_CM_XZ)

	zval const_CM_XC_value;
	ZVAL_LONG(&const_CM_XC_value, ZIP_CM_XZ);
	crex_string *const_CM_XC_name = crex_string_init_interned("CM_XZ", sizeof("CM_XZ") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_XC_name, &const_CM_XC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_XC_name);
#endif

	zval const_CM_TERSE_value;
	ZVAL_LONG(&const_CM_TERSE_value, ZIP_CM_TERSE);
	crex_string *const_CM_TERSE_name = crex_string_init_interned("CM_TERSE", sizeof("CM_TERSE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_TERSE_name, &const_CM_TERSE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_TERSE_name);

	zval const_CM_LZ77_value;
	ZVAL_LONG(&const_CM_LZ77_value, ZIP_CM_LZ77);
	crex_string *const_CM_LZ77_name = crex_string_init_interned("CM_LZ77", sizeof("CM_LZ77") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_LZ77_name, &const_CM_LZ77_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_LZ77_name);

	zval const_CM_WAVPACK_value;
	ZVAL_LONG(&const_CM_WAVPACK_value, ZIP_CM_WAVPACK);
	crex_string *const_CM_WAVPACK_name = crex_string_init_interned("CM_WAVPACK", sizeof("CM_WAVPACK") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_WAVPACK_name, &const_CM_WAVPACK_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_WAVPACK_name);

	zval const_CM_PPMD_value;
	ZVAL_LONG(&const_CM_PPMD_value, ZIP_CM_PPMD);
	crex_string *const_CM_PPMD_name = crex_string_init_interned("CM_PPMD", sizeof("CM_PPMD") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CM_PPMD_name, &const_CM_PPMD_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CM_PPMD_name);

	zval const_ER_OK_value;
	ZVAL_LONG(&const_ER_OK_value, ZIP_ER_OK);
	crex_string *const_ER_OK_name = crex_string_init_interned("ER_OK", sizeof("ER_OK") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_OK_name, &const_ER_OK_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_OK_name);

	zval const_ER_MULTIDISK_value;
	ZVAL_LONG(&const_ER_MULTIDISK_value, ZIP_ER_MULTIDISK);
	crex_string *const_ER_MULTIDISK_name = crex_string_init_interned("ER_MULTIDISK", sizeof("ER_MULTIDISK") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_MULTIDISK_name, &const_ER_MULTIDISK_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_MULTIDISK_name);

	zval const_ER_RENAME_value;
	ZVAL_LONG(&const_ER_RENAME_value, ZIP_ER_RENAME);
	crex_string *const_ER_RENAME_name = crex_string_init_interned("ER_RENAME", sizeof("ER_RENAME") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_RENAME_name, &const_ER_RENAME_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_RENAME_name);

	zval const_ER_CLOSE_value;
	ZVAL_LONG(&const_ER_CLOSE_value, ZIP_ER_CLOSE);
	crex_string *const_ER_CLOSE_name = crex_string_init_interned("ER_CLOSE", sizeof("ER_CLOSE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_CLOSE_name, &const_ER_CLOSE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_CLOSE_name);

	zval const_ER_SEEK_value;
	ZVAL_LONG(&const_ER_SEEK_value, ZIP_ER_SEEK);
	crex_string *const_ER_SEEK_name = crex_string_init_interned("ER_SEEK", sizeof("ER_SEEK") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_SEEK_name, &const_ER_SEEK_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_SEEK_name);

	zval const_ER_READ_value;
	ZVAL_LONG(&const_ER_READ_value, ZIP_ER_READ);
	crex_string *const_ER_READ_name = crex_string_init_interned("ER_READ", sizeof("ER_READ") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_READ_name, &const_ER_READ_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_READ_name);

	zval const_ER_WRITE_value;
	ZVAL_LONG(&const_ER_WRITE_value, ZIP_ER_WRITE);
	crex_string *const_ER_WRITE_name = crex_string_init_interned("ER_WRITE", sizeof("ER_WRITE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_WRITE_name, &const_ER_WRITE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_WRITE_name);

	zval const_ER_CRC_value;
	ZVAL_LONG(&const_ER_CRC_value, ZIP_ER_CRC);
	crex_string *const_ER_CRC_name = crex_string_init_interned("ER_CRC", sizeof("ER_CRC") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_CRC_name, &const_ER_CRC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_CRC_name);

	zval const_ER_ZIPCLOSED_value;
	ZVAL_LONG(&const_ER_ZIPCLOSED_value, ZIP_ER_ZIPCLOSED);
	crex_string *const_ER_ZIPCLOSED_name = crex_string_init_interned("ER_ZIPCLOSED", sizeof("ER_ZIPCLOSED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_ZIPCLOSED_name, &const_ER_ZIPCLOSED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_ZIPCLOSED_name);

	zval const_ER_NOENT_value;
	ZVAL_LONG(&const_ER_NOENT_value, ZIP_ER_NOENT);
	crex_string *const_ER_NOENT_name = crex_string_init_interned("ER_NOENT", sizeof("ER_NOENT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_NOENT_name, &const_ER_NOENT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_NOENT_name);

	zval const_ER_EXISTS_value;
	ZVAL_LONG(&const_ER_EXISTS_value, ZIP_ER_EXISTS);
	crex_string *const_ER_EXISTS_name = crex_string_init_interned("ER_EXISTS", sizeof("ER_EXISTS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_EXISTS_name, &const_ER_EXISTS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_EXISTS_name);

	zval const_ER_OPEN_value;
	ZVAL_LONG(&const_ER_OPEN_value, ZIP_ER_OPEN);
	crex_string *const_ER_OPEN_name = crex_string_init_interned("ER_OPEN", sizeof("ER_OPEN") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_OPEN_name, &const_ER_OPEN_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_OPEN_name);

	zval const_ER_TMPOPEN_value;
	ZVAL_LONG(&const_ER_TMPOPEN_value, ZIP_ER_TMPOPEN);
	crex_string *const_ER_TMPOPEN_name = crex_string_init_interned("ER_TMPOPEN", sizeof("ER_TMPOPEN") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_TMPOPEN_name, &const_ER_TMPOPEN_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_TMPOPEN_name);

	zval const_ER_ZLIB_value;
	ZVAL_LONG(&const_ER_ZLIB_value, ZIP_ER_ZLIB);
	crex_string *const_ER_ZLIB_name = crex_string_init_interned("ER_ZLIB", sizeof("ER_ZLIB") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_ZLIB_name, &const_ER_ZLIB_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_ZLIB_name);

	zval const_ER_MEMORY_value;
	ZVAL_LONG(&const_ER_MEMORY_value, ZIP_ER_MEMORY);
	crex_string *const_ER_MEMORY_name = crex_string_init_interned("ER_MEMORY", sizeof("ER_MEMORY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_MEMORY_name, &const_ER_MEMORY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_MEMORY_name);

	zval const_ER_CHANGED_value;
	ZVAL_LONG(&const_ER_CHANGED_value, ZIP_ER_CHANGED);
	crex_string *const_ER_CHANGED_name = crex_string_init_interned("ER_CHANGED", sizeof("ER_CHANGED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_CHANGED_name, &const_ER_CHANGED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_CHANGED_name);

	zval const_ER_COMPNOTSUPP_value;
	ZVAL_LONG(&const_ER_COMPNOTSUPP_value, ZIP_ER_COMPNOTSUPP);
	crex_string *const_ER_COMPNOTSUPP_name = crex_string_init_interned("ER_COMPNOTSUPP", sizeof("ER_COMPNOTSUPP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_COMPNOTSUPP_name, &const_ER_COMPNOTSUPP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_COMPNOTSUPP_name);

	zval const_ER_EOF_value;
	ZVAL_LONG(&const_ER_EOF_value, ZIP_ER_EOF);
	crex_string *const_ER_EOF_name = crex_string_init_interned("ER_EOF", sizeof("ER_EOF") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_EOF_name, &const_ER_EOF_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_EOF_name);

	zval const_ER_INVAL_value;
	ZVAL_LONG(&const_ER_INVAL_value, ZIP_ER_INVAL);
	crex_string *const_ER_INVAL_name = crex_string_init_interned("ER_INVAL", sizeof("ER_INVAL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_INVAL_name, &const_ER_INVAL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_INVAL_name);

	zval const_ER_NOZIP_value;
	ZVAL_LONG(&const_ER_NOZIP_value, ZIP_ER_NOZIP);
	crex_string *const_ER_NOZIP_name = crex_string_init_interned("ER_NOZIP", sizeof("ER_NOZIP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_NOZIP_name, &const_ER_NOZIP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_NOZIP_name);

	zval const_ER_INTERNAL_value;
	ZVAL_LONG(&const_ER_INTERNAL_value, ZIP_ER_INTERNAL);
	crex_string *const_ER_INTERNAL_name = crex_string_init_interned("ER_INTERNAL", sizeof("ER_INTERNAL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_INTERNAL_name, &const_ER_INTERNAL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_INTERNAL_name);

	zval const_ER_INCONS_value;
	ZVAL_LONG(&const_ER_INCONS_value, ZIP_ER_INCONS);
	crex_string *const_ER_INCONS_name = crex_string_init_interned("ER_INCONS", sizeof("ER_INCONS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_INCONS_name, &const_ER_INCONS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_INCONS_name);

	zval const_ER_REMOVE_value;
	ZVAL_LONG(&const_ER_REMOVE_value, ZIP_ER_REMOVE);
	crex_string *const_ER_REMOVE_name = crex_string_init_interned("ER_REMOVE", sizeof("ER_REMOVE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_REMOVE_name, &const_ER_REMOVE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_REMOVE_name);

	zval const_ER_DELETED_value;
	ZVAL_LONG(&const_ER_DELETED_value, ZIP_ER_DELETED);
	crex_string *const_ER_DELETED_name = crex_string_init_interned("ER_DELETED", sizeof("ER_DELETED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_DELETED_name, &const_ER_DELETED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_DELETED_name);

	zval const_ER_ENCRNOTSUPP_value;
	ZVAL_LONG(&const_ER_ENCRNOTSUPP_value, ZIP_ER_ENCRNOTSUPP);
	crex_string *const_ER_ENCRNOTSUPP_name = crex_string_init_interned("ER_ENCRNOTSUPP", sizeof("ER_ENCRNOTSUPP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_ENCRNOTSUPP_name, &const_ER_ENCRNOTSUPP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_ENCRNOTSUPP_name);

	zval const_ER_RDONLY_value;
	ZVAL_LONG(&const_ER_RDONLY_value, ZIP_ER_RDONLY);
	crex_string *const_ER_RDONLY_name = crex_string_init_interned("ER_RDONLY", sizeof("ER_RDONLY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_RDONLY_name, &const_ER_RDONLY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_RDONLY_name);

	zval const_ER_NOPASSWD_value;
	ZVAL_LONG(&const_ER_NOPASSWD_value, ZIP_ER_NOPASSWD);
	crex_string *const_ER_NOPASSWD_name = crex_string_init_interned("ER_NOPASSWD", sizeof("ER_NOPASSWD") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_NOPASSWD_name, &const_ER_NOPASSWD_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_NOPASSWD_name);

	zval const_ER_WRONGPASSWD_value;
	ZVAL_LONG(&const_ER_WRONGPASSWD_value, ZIP_ER_WRONGPASSWD);
	crex_string *const_ER_WRONGPASSWD_name = crex_string_init_interned("ER_WRONGPASSWD", sizeof("ER_WRONGPASSWD") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_WRONGPASSWD_name, &const_ER_WRONGPASSWD_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_WRONGPASSWD_name);
#if defined(ZIP_ER_OPNOTSUPP)

	zval const_ER_OPNOTSUPP_value;
	ZVAL_LONG(&const_ER_OPNOTSUPP_value, ZIP_ER_OPNOTSUPP);
	crex_string *const_ER_OPNOTSUPP_name = crex_string_init_interned("ER_OPNOTSUPP", sizeof("ER_OPNOTSUPP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_OPNOTSUPP_name, &const_ER_OPNOTSUPP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_OPNOTSUPP_name);
#endif
#if defined(ZIP_ER_INUSE)

	zval const_ER_INUSE_value;
	ZVAL_LONG(&const_ER_INUSE_value, ZIP_ER_INUSE);
	crex_string *const_ER_INUSE_name = crex_string_init_interned("ER_INUSE", sizeof("ER_INUSE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_INUSE_name, &const_ER_INUSE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_INUSE_name);
#endif
#if defined(ZIP_ER_TELL)

	zval const_ER_TELL_value;
	ZVAL_LONG(&const_ER_TELL_value, ZIP_ER_TELL);
	crex_string *const_ER_TELL_name = crex_string_init_interned("ER_TELL", sizeof("ER_TELL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_TELL_name, &const_ER_TELL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_TELL_name);
#endif
#if defined(ZIP_ER_COMPRESSED_DATA)

	zval const_ER_COMPRESSED_DATA_value;
	ZVAL_LONG(&const_ER_COMPRESSED_DATA_value, ZIP_ER_COMPRESSED_DATA);
	crex_string *const_ER_COMPRESSED_DATA_name = crex_string_init_interned("ER_COMPRESSED_DATA", sizeof("ER_COMPRESSED_DATA") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_COMPRESSED_DATA_name, &const_ER_COMPRESSED_DATA_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_COMPRESSED_DATA_name);
#endif
#if defined(ZIP_ER_CANCELLED)

	zval const_ER_CANCELLED_value;
	ZVAL_LONG(&const_ER_CANCELLED_value, ZIP_ER_CANCELLED);
	crex_string *const_ER_CANCELLED_name = crex_string_init_interned("ER_CANCELLED", sizeof("ER_CANCELLED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_CANCELLED_name, &const_ER_CANCELLED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_CANCELLED_name);
#endif
#if defined(ZIP_ER_DATA_LENGTH)

	zval const_ER_DATA_LENGTH_value;
	ZVAL_LONG(&const_ER_DATA_LENGTH_value, ZIP_ER_DATA_LENGTH);
	crex_string *const_ER_DATA_LENGTH_name = crex_string_init_interned("ER_DATA_LENGTH", sizeof("ER_DATA_LENGTH") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_DATA_LENGTH_name, &const_ER_DATA_LENGTH_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_DATA_LENGTH_name);
#endif
#if defined(ZIP_ER_NOT_ALLOWED)

	zval const_ER_NOT_ALLOWED_value;
	ZVAL_LONG(&const_ER_NOT_ALLOWED_value, ZIP_ER_NOT_ALLOWED);
	crex_string *const_ER_NOT_ALLOWED_name = crex_string_init_interned("ER_NOT_ALLOWED", sizeof("ER_NOT_ALLOWED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ER_NOT_ALLOWED_name, &const_ER_NOT_ALLOWED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ER_NOT_ALLOWED_name);
#endif
#if defined(ZIP_AFL_RDONLY)

	zval const_AFL_RDONLY_value;
	ZVAL_LONG(&const_AFL_RDONLY_value, ZIP_AFL_RDONLY);
	crex_string *const_AFL_RDONLY_name = crex_string_init_interned("AFL_RDONLY", sizeof("AFL_RDONLY") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_AFL_RDONLY_name, &const_AFL_RDONLY_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_AFL_RDONLY_name);
#endif
#if defined(ZIP_AFL_IS_TORRENTZIP)

	zval const_AFL_IS_TORRENTZIP_value;
	ZVAL_LONG(&const_AFL_IS_TORRENTZIP_value, ZIP_AFL_IS_TORRENTZIP);
	crex_string *const_AFL_IS_TORRENTZIP_name = crex_string_init_interned("AFL_IS_TORRENTZIP", sizeof("AFL_IS_TORRENTZIP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_AFL_IS_TORRENTZIP_name, &const_AFL_IS_TORRENTZIP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_AFL_IS_TORRENTZIP_name);
#endif
#if defined(ZIP_AFL_WANT_TORRENTZIP)

	zval const_AFL_WANT_TORRENTZIP_value;
	ZVAL_LONG(&const_AFL_WANT_TORRENTZIP_value, ZIP_AFL_WANT_TORRENTZIP);
	crex_string *const_AFL_WANT_TORRENTZIP_name = crex_string_init_interned("AFL_WANT_TORRENTZIP", sizeof("AFL_WANT_TORRENTZIP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_AFL_WANT_TORRENTZIP_name, &const_AFL_WANT_TORRENTZIP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_AFL_WANT_TORRENTZIP_name);
#endif
#if defined(ZIP_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE)

	zval const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_value;
	ZVAL_LONG(&const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_value, ZIP_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE);
	crex_string *const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_name = crex_string_init_interned("AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE", sizeof("AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_name, &const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_AFL_CREATE_OR_KEEP_FILE_FOR_EMPTY_ARCHIVE_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_DOS_value;
	ZVAL_LONG(&const_OPSYS_DOS_value, ZIP_OPSYS_DOS);
	crex_string *const_OPSYS_DOS_name = crex_string_init_interned("OPSYS_DOS", sizeof("OPSYS_DOS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_DOS_name, &const_OPSYS_DOS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_DOS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_AMIGA_value;
	ZVAL_LONG(&const_OPSYS_AMIGA_value, ZIP_OPSYS_AMIGA);
	crex_string *const_OPSYS_AMIGA_name = crex_string_init_interned("OPSYS_AMIGA", sizeof("OPSYS_AMIGA") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_AMIGA_name, &const_OPSYS_AMIGA_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_AMIGA_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_OPENVMS_value;
	ZVAL_LONG(&const_OPSYS_OPENVMS_value, ZIP_OPSYS_OPENVMS);
	crex_string *const_OPSYS_OPENVMS_name = crex_string_init_interned("OPSYS_OPENVMS", sizeof("OPSYS_OPENVMS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_OPENVMS_name, &const_OPSYS_OPENVMS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_OPENVMS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_UNIX_value;
	ZVAL_LONG(&const_OPSYS_UNIX_value, ZIP_OPSYS_UNIX);
	crex_string *const_OPSYS_UNIX_name = crex_string_init_interned("OPSYS_UNIX", sizeof("OPSYS_UNIX") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_UNIX_name, &const_OPSYS_UNIX_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_UNIX_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_VM_CMS_value;
	ZVAL_LONG(&const_OPSYS_VM_CMS_value, ZIP_OPSYS_VM_CMS);
	crex_string *const_OPSYS_VM_CMS_name = crex_string_init_interned("OPSYS_VM_CMS", sizeof("OPSYS_VM_CMS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_VM_CMS_name, &const_OPSYS_VM_CMS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_VM_CMS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_ATARI_ST_value;
	ZVAL_LONG(&const_OPSYS_ATARI_ST_value, ZIP_OPSYS_ATARI_ST);
	crex_string *const_OPSYS_ATARI_ST_name = crex_string_init_interned("OPSYS_ATARI_ST", sizeof("OPSYS_ATARI_ST") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_ATARI_ST_name, &const_OPSYS_ATARI_ST_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_ATARI_ST_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_OS_2_value;
	ZVAL_LONG(&const_OPSYS_OS_2_value, ZIP_OPSYS_OS_2);
	crex_string *const_OPSYS_OS_2_name = crex_string_init_interned("OPSYS_OS_2", sizeof("OPSYS_OS_2") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_OS_2_name, &const_OPSYS_OS_2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_OS_2_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_MACINTOSH_value;
	ZVAL_LONG(&const_OPSYS_MACINTOSH_value, ZIP_OPSYS_MACINTOSH);
	crex_string *const_OPSYS_MACINTOSH_name = crex_string_init_interned("OPSYS_MACINTOSH", sizeof("OPSYS_MACINTOSH") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_MACINTOSH_name, &const_OPSYS_MACINTOSH_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_MACINTOSH_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_C_SYSTEM_value;
	ZVAL_LONG(&const_OPSYS_C_SYSTEM_value, ZIP_OPSYS_C_SYSTEM);
	crex_string *const_OPSYS_C_SYSTEM_name = crex_string_init_interned("OPSYS_C_SYSTEM", sizeof("OPSYS_C_SYSTEM") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_C_SYSTEM_name, &const_OPSYS_C_SYSTEM_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_C_SYSTEM_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_CPM_value;
	ZVAL_LONG(&const_OPSYS_CPM_value, ZIP_OPSYS_CPM);
	crex_string *const_OPSYS_CPM_name = crex_string_init_interned("OPSYS_CPM", sizeof("OPSYS_CPM") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_CPM_name, &const_OPSYS_CPM_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_CPM_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_WINDOWS_NTFS_value;
	ZVAL_LONG(&const_OPSYS_WINDOWS_NTFS_value, ZIP_OPSYS_WINDOWS_NTFS);
	crex_string *const_OPSYS_WINDOWS_NTFS_name = crex_string_init_interned("OPSYS_WINDOWS_NTFS", sizeof("OPSYS_WINDOWS_NTFS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_WINDOWS_NTFS_name, &const_OPSYS_WINDOWS_NTFS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_WINDOWS_NTFS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_MVS_value;
	ZVAL_LONG(&const_OPSYS_MVS_value, ZIP_OPSYS_MVS);
	crex_string *const_OPSYS_MVS_name = crex_string_init_interned("OPSYS_MVS", sizeof("OPSYS_MVS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_MVS_name, &const_OPSYS_MVS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_MVS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_VSE_value;
	ZVAL_LONG(&const_OPSYS_VSE_value, ZIP_OPSYS_VSE);
	crex_string *const_OPSYS_VSE_name = crex_string_init_interned("OPSYS_VSE", sizeof("OPSYS_VSE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_VSE_name, &const_OPSYS_VSE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_VSE_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_ACORN_RISC_value;
	ZVAL_LONG(&const_OPSYS_ACORN_RISC_value, ZIP_OPSYS_ACORN_RISC);
	crex_string *const_OPSYS_ACORN_RISC_name = crex_string_init_interned("OPSYS_ACORN_RISC", sizeof("OPSYS_ACORN_RISC") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_ACORN_RISC_name, &const_OPSYS_ACORN_RISC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_ACORN_RISC_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_VFAT_value;
	ZVAL_LONG(&const_OPSYS_VFAT_value, ZIP_OPSYS_VFAT);
	crex_string *const_OPSYS_VFAT_name = crex_string_init_interned("OPSYS_VFAT", sizeof("OPSYS_VFAT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_VFAT_name, &const_OPSYS_VFAT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_VFAT_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_ALTERNATE_MVS_value;
	ZVAL_LONG(&const_OPSYS_ALTERNATE_MVS_value, ZIP_OPSYS_ALTERNATE_MVS);
	crex_string *const_OPSYS_ALTERNATE_MVS_name = crex_string_init_interned("OPSYS_ALTERNATE_MVS", sizeof("OPSYS_ALTERNATE_MVS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_ALTERNATE_MVS_name, &const_OPSYS_ALTERNATE_MVS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_ALTERNATE_MVS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_BEOS_value;
	ZVAL_LONG(&const_OPSYS_BEOS_value, ZIP_OPSYS_BEOS);
	crex_string *const_OPSYS_BEOS_name = crex_string_init_interned("OPSYS_BEOS", sizeof("OPSYS_BEOS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_BEOS_name, &const_OPSYS_BEOS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_BEOS_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_TANDEM_value;
	ZVAL_LONG(&const_OPSYS_TANDEM_value, ZIP_OPSYS_TANDEM);
	crex_string *const_OPSYS_TANDEM_name = crex_string_init_interned("OPSYS_TANDEM", sizeof("OPSYS_TANDEM") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_TANDEM_name, &const_OPSYS_TANDEM_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_TANDEM_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_OS_400_value;
	ZVAL_LONG(&const_OPSYS_OS_400_value, ZIP_OPSYS_OS_400);
	crex_string *const_OPSYS_OS_400_name = crex_string_init_interned("OPSYS_OS_400", sizeof("OPSYS_OS_400") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_OS_400_name, &const_OPSYS_OS_400_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_OS_400_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_OS_X_value;
	ZVAL_LONG(&const_OPSYS_OS_X_value, ZIP_OPSYS_OS_X);
	crex_string *const_OPSYS_OS_X_name = crex_string_init_interned("OPSYS_OS_X", sizeof("OPSYS_OS_X") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_OS_X_name, &const_OPSYS_OS_X_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_OS_X_name);
#endif
#if defined(ZIP_OPSYS_DEFAULT)

	zval const_OPSYS_DEFAULT_value;
	ZVAL_LONG(&const_OPSYS_DEFAULT_value, ZIP_OPSYS_DEFAULT);
	crex_string *const_OPSYS_DEFAULT_name = crex_string_init_interned("OPSYS_DEFAULT", sizeof("OPSYS_DEFAULT") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPSYS_DEFAULT_name, &const_OPSYS_DEFAULT_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPSYS_DEFAULT_name);
#endif

	zval const_EM_NONE_value;
	ZVAL_LONG(&const_EM_NONE_value, ZIP_EM_NONE);
	crex_string *const_EM_NONE_name = crex_string_init_interned("EM_NONE", sizeof("EM_NONE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_NONE_name, &const_EM_NONE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_NONE_name);

	zval const_EM_TRAD_PKWARE_value;
	ZVAL_LONG(&const_EM_TRAD_PKWARE_value, ZIP_EM_TRAD_PKWARE);
	crex_string *const_EM_TRAD_PKWARE_name = crex_string_init_interned("EM_TRAD_PKWARE", sizeof("EM_TRAD_PKWARE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_TRAD_PKWARE_name, &const_EM_TRAD_PKWARE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_TRAD_PKWARE_name);
#if defined(HAVE_ENCRYPTION)

	zval const_EM_AES_128_value;
	ZVAL_LONG(&const_EM_AES_128_value, ZIP_EM_AES_128);
	crex_string *const_EM_AES_128_name = crex_string_init_interned("EM_AES_128", sizeof("EM_AES_128") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_AES_128_name, &const_EM_AES_128_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_AES_128_name);
#endif
#if defined(HAVE_ENCRYPTION)

	zval const_EM_AES_192_value;
	ZVAL_LONG(&const_EM_AES_192_value, ZIP_EM_AES_192);
	crex_string *const_EM_AES_192_name = crex_string_init_interned("EM_AES_192", sizeof("EM_AES_192") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_AES_192_name, &const_EM_AES_192_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_AES_192_name);
#endif
#if defined(HAVE_ENCRYPTION)

	zval const_EM_AES_256_value;
	ZVAL_LONG(&const_EM_AES_256_value, ZIP_EM_AES_256);
	crex_string *const_EM_AES_256_name = crex_string_init_interned("EM_AES_256", sizeof("EM_AES_256") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_AES_256_name, &const_EM_AES_256_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_AES_256_name);
#endif

	zval const_EM_UNKNOWN_value;
	ZVAL_LONG(&const_EM_UNKNOWN_value, ZIP_EM_UNKNOWN);
	crex_string *const_EM_UNKNOWN_name = crex_string_init_interned("EM_UNKNOWN", sizeof("EM_UNKNOWN") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_EM_UNKNOWN_name, &const_EM_UNKNOWN_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_EM_UNKNOWN_name);

	zval const_LIBZIP_VERSION_value;
	crex_string *const_LIBZIP_VERSION_value_str = crex_string_init(LIBZIP_VERSION_STR, strlen(LIBZIP_VERSION_STR), 1);
	ZVAL_STR(&const_LIBZIP_VERSION_value, const_LIBZIP_VERSION_value_str);
	crex_string *const_LIBZIP_VERSION_name = crex_string_init_interned("LIBZIP_VERSION", sizeof("LIBZIP_VERSION") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_LIBZIP_VERSION_name, &const_LIBZIP_VERSION_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(const_LIBZIP_VERSION_name);

	zval const_LENGTH_TO_END_value;
	ZVAL_LONG(&const_LENGTH_TO_END_value, ZIP_LENGTH_TO_END);
	crex_string *const_LENGTH_TO_END_name = crex_string_init_interned("LENGTH_TO_END", sizeof("LENGTH_TO_END") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_LENGTH_TO_END_name, &const_LENGTH_TO_END_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_LENGTH_TO_END_name);
#if defined(ZIP_LENGTH_UNCHECKED)

	zval const_LENGTH_UNCHECKED_value;
	ZVAL_LONG(&const_LENGTH_UNCHECKED_value, ZIP_LENGTH_UNCHECKED);
	crex_string *const_LENGTH_UNCHECKED_name = crex_string_init_interned("LENGTH_UNCHECKED", sizeof("LENGTH_UNCHECKED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_LENGTH_UNCHECKED_name, &const_LENGTH_UNCHECKED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_LENGTH_UNCHECKED_name);
#endif

	zval property_lastId_default_value;
	ZVAL_UNDEF(&property_lastId_default_value);
	crex_string *property_lastId_name = crex_string_init("lastId", sizeof("lastId") - 1, 1);
	crex_declare_typed_property(class_entry, property_lastId_name, &property_lastId_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_lastId_name);

	zval property_status_default_value;
	ZVAL_UNDEF(&property_status_default_value);
	crex_string *property_status_name = crex_string_init("status", sizeof("status") - 1, 1);
	crex_declare_typed_property(class_entry, property_status_name, &property_status_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_status_name);

	zval property_statusSys_default_value;
	ZVAL_UNDEF(&property_statusSys_default_value);
	crex_string *property_statusSys_name = crex_string_init("statusSys", sizeof("statusSys") - 1, 1);
	crex_declare_typed_property(class_entry, property_statusSys_name, &property_statusSys_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_statusSys_name);

	zval property_numFiles_default_value;
	ZVAL_UNDEF(&property_numFiles_default_value);
	crex_string *property_numFiles_name = crex_string_init("numFiles", sizeof("numFiles") - 1, 1);
	crex_declare_typed_property(class_entry, property_numFiles_name, &property_numFiles_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(property_numFiles_name);

	zval property_filename_default_value;
	ZVAL_UNDEF(&property_filename_default_value);
	crex_string *property_filename_name = crex_string_init("filename", sizeof("filename") - 1, 1);
	crex_declare_typed_property(class_entry, property_filename_name, &property_filename_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_filename_name);

	zval property_comment_default_value;
	ZVAL_UNDEF(&property_comment_default_value);
	crex_string *property_comment_name = crex_string_init("comment", sizeof("comment") - 1, 1);
	crex_declare_typed_property(class_entry, property_comment_name, &property_comment_default_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_STRING));
	crex_string_release(property_comment_name);


	crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "setpassword", sizeof("setpassword") - 1), 0, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
#if defined(HAVE_ENCRYPTION)

	crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "setencryptionname", sizeof("setencryptionname") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
#endif
#if defined(HAVE_ENCRYPTION)

	crex_add_parameter_attribute(crex_hash_str_find_ptr(&class_entry->function_table, "setencryptionindex", sizeof("setencryptionindex") - 1), 2, ZSTR_KNOWN(CREX_STR_SENSITIVEPARAMETER), 0);
#endif

	return class_entry;
}
