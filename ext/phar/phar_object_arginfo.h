/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 00f5d4fc74e8b7dc67da6f12180c9fae343954cc */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Crxa___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FilesystemIterator::SKIP_DOTS | FilesystemIterator::UNIX_PATHS")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Crxa___destruct, 0, 0, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_addEmptyDir, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_addFile, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, localName, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_addFromString, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, contents, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_buildFromDirectory, 0, 1, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pattern, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_buildFromIterator, 0, 1, IS_ARRAY, 0)
	CREX_ARG_OBJ_INFO(0, iterator, Traversable, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, baseDirectory, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_compressFiles, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, compression, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_decompressFiles arginfo_class_Crxa___destruct

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Crxa_compress, 0, 1, Crxa, 1)
	CREX_ARG_TYPE_INFO(0, compression, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Crxa_decompress, 0, 0, Crxa, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Crxa_convertToExecutable, 0, 0, Crxa, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compression, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Crxa_convertToData, 0, 0, CrxaData, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compression, IS_LONG, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Crxa_copy, 0, 0, 2)
	CREX_ARG_TYPE_INFO(0, from, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, to, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_count, 0, 0, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "COUNT_NORMAL")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Crxa_delete, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, localName, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_delMetadata arginfo_class_Crxa___destruct

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_extractTo, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_MASK(0, files, MAY_BE_ARRAY|MAY_BE_STRING|MAY_BE_NULL, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, overwrite, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_getAlias, 0, 0, IS_STRING, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_getPath, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_getMetadata, 0, 0, IS_MIXED, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, unserializeOptions, IS_ARRAY, 0, "[]")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_getModified, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Crxa_getSignature, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_getStub arginfo_class_Crxa_getPath

#define arginfo_class_Crxa_getVersion arginfo_class_Crxa_getPath

#define arginfo_class_Crxa_hasMetadata arginfo_class_Crxa_getModified

#define arginfo_class_Crxa_isBuffering arginfo_class_Crxa_getModified

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_Crxa_isCompressed, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_isFileFormat, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, format, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_isWritable arginfo_class_Crxa_getModified

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_offsetExists, 0, 1, _IS_BOOL, 0)
	CREX_ARG_INFO(0, localName)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_Crxa_offsetGet, 0, 1, SplFileInfo, 0)
	CREX_ARG_INFO(0, localName)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_offsetSet, 0, 2, IS_VOID, 0)
	CREX_ARG_INFO(0, localName)
	CREX_ARG_INFO(0, value)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_offsetUnset, 0, 1, IS_VOID, 0)
	CREX_ARG_INFO(0, localName)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_setAlias, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, alias, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_setDefaultStub, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, index, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, webIndex, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_setMetadata, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, metadata, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_setSignatureAlgorithm, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, algo, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, privateKey, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_Crxa_setStub, 0, 0, 1)
	CREX_ARG_INFO(0, stub)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_startBuffering, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_stopBuffering arginfo_class_Crxa_startBuffering

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_apiVersion, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_canCompress, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compression, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_canWrite, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_createDefaultStub, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, index, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, webIndex, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_getSupportedCompression, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

#define arginfo_class_Crxa_getSupportedSignatures arginfo_class_Crxa_getSupportedCompression

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_interceptFileFuncs, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_isValidCrxaFilename, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, executable, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_loadCrxa, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_mapCrxa, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, offset, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_running, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, returnCrxa, _IS_BOOL, 0, "true")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_mount, 0, 2, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, crxaPath, IS_STRING, 0)
	CREX_ARG_TYPE_INFO(0, externalPath, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_mungServer, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, variables, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_unlinkArchive, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Crxa_webCrxa, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, index, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fileNotFoundScript, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mimeTypes, IS_ARRAY, 0, "[]")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rewrite, IS_CALLABLE, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrxaData___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FilesystemIterator::SKIP_DOTS | FilesystemIterator::UNIX_PATHS")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, alias, IS_STRING, 1, "null")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_CrxaData___destruct arginfo_class_Crxa___destruct

#define arginfo_class_CrxaData_addEmptyDir arginfo_class_Crxa_addEmptyDir

#define arginfo_class_CrxaData_addFile arginfo_class_Crxa_addFile

#define arginfo_class_CrxaData_addFromString arginfo_class_Crxa_addFromString

#define arginfo_class_CrxaData_buildFromDirectory arginfo_class_Crxa_buildFromDirectory

#define arginfo_class_CrxaData_buildFromIterator arginfo_class_Crxa_buildFromIterator

#define arginfo_class_CrxaData_compressFiles arginfo_class_Crxa_compressFiles

#define arginfo_class_CrxaData_decompressFiles arginfo_class_Crxa___destruct

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_CrxaData_compress, 0, 1, CrxaData, 1)
	CREX_ARG_TYPE_INFO(0, compression, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_CrxaData_decompress, 0, 0, CrxaData, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, extension, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_CrxaData_convertToExecutable arginfo_class_Crxa_convertToExecutable

#define arginfo_class_CrxaData_convertToData arginfo_class_Crxa_convertToData

#define arginfo_class_CrxaData_copy arginfo_class_Crxa_copy

#define arginfo_class_CrxaData_count arginfo_class_Crxa_count

#define arginfo_class_CrxaData_delete arginfo_class_Crxa_delete

#define arginfo_class_CrxaData_delMetadata arginfo_class_Crxa___destruct

#define arginfo_class_CrxaData_extractTo arginfo_class_Crxa_extractTo

#define arginfo_class_CrxaData_getAlias arginfo_class_Crxa_getAlias

#define arginfo_class_CrxaData_getPath arginfo_class_Crxa_getPath

#define arginfo_class_CrxaData_getMetadata arginfo_class_Crxa_getMetadata

#define arginfo_class_CrxaData_getModified arginfo_class_Crxa_getModified

#define arginfo_class_CrxaData_getSignature arginfo_class_Crxa_getSignature

#define arginfo_class_CrxaData_getStub arginfo_class_Crxa_getPath

#define arginfo_class_CrxaData_getVersion arginfo_class_Crxa_getPath

#define arginfo_class_CrxaData_hasMetadata arginfo_class_Crxa_getModified

#define arginfo_class_CrxaData_isBuffering arginfo_class_Crxa_getModified

#define arginfo_class_CrxaData_isCompressed arginfo_class_Crxa_isCompressed

#define arginfo_class_CrxaData_isFileFormat arginfo_class_Crxa_isFileFormat

#define arginfo_class_CrxaData_isWritable arginfo_class_Crxa_getModified

#define arginfo_class_CrxaData_offsetExists arginfo_class_Crxa_offsetExists

#define arginfo_class_CrxaData_offsetGet arginfo_class_Crxa_offsetGet

#define arginfo_class_CrxaData_offsetSet arginfo_class_Crxa_offsetSet

#define arginfo_class_CrxaData_offsetUnset arginfo_class_Crxa_offsetUnset

#define arginfo_class_CrxaData_setAlias arginfo_class_Crxa_setAlias

#define arginfo_class_CrxaData_setDefaultStub arginfo_class_Crxa_setDefaultStub

#define arginfo_class_CrxaData_setMetadata arginfo_class_Crxa_setMetadata

#define arginfo_class_CrxaData_setSignatureAlgorithm arginfo_class_Crxa_setSignatureAlgorithm

#define arginfo_class_CrxaData_setStub arginfo_class_Crxa_setStub

#define arginfo_class_CrxaData_startBuffering arginfo_class_Crxa_startBuffering

#define arginfo_class_CrxaData_stopBuffering arginfo_class_Crxa_startBuffering

#define arginfo_class_CrxaData_apiVersion arginfo_class_Crxa_apiVersion

#define arginfo_class_CrxaData_canCompress arginfo_class_Crxa_canCompress

#define arginfo_class_CrxaData_canWrite arginfo_class_Crxa_canWrite

#define arginfo_class_CrxaData_createDefaultStub arginfo_class_Crxa_createDefaultStub

#define arginfo_class_CrxaData_getSupportedCompression arginfo_class_Crxa_getSupportedCompression

#define arginfo_class_CrxaData_getSupportedSignatures arginfo_class_Crxa_getSupportedCompression

#define arginfo_class_CrxaData_interceptFileFuncs arginfo_class_Crxa_interceptFileFuncs

#define arginfo_class_CrxaData_isValidCrxaFilename arginfo_class_Crxa_isValidCrxaFilename

#define arginfo_class_CrxaData_loadCrxa arginfo_class_Crxa_loadCrxa

#define arginfo_class_CrxaData_mapCrxa arginfo_class_Crxa_mapCrxa

#define arginfo_class_CrxaData_running arginfo_class_Crxa_running

#define arginfo_class_CrxaData_mount arginfo_class_Crxa_mount

#define arginfo_class_CrxaData_mungServer arginfo_class_Crxa_mungServer

#define arginfo_class_CrxaData_unlinkArchive arginfo_class_Crxa_unlinkArchive

#define arginfo_class_CrxaData_webCrxa arginfo_class_Crxa_webCrxa

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrxaFileInfo___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrxaFileInfo___destruct arginfo_class_Crxa___destruct

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CrxaFileInfo_chmod, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, perms, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_CrxaFileInfo_compress, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, compression, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrxaFileInfo_decompress arginfo_class_Crxa___destruct

#define arginfo_class_CrxaFileInfo_delMetadata arginfo_class_Crxa___destruct

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CrxaFileInfo_getCompressedSize, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_CrxaFileInfo_getCRC32 arginfo_class_CrxaFileInfo_getCompressedSize

#define arginfo_class_CrxaFileInfo_getContent arginfo_class_Crxa_getPath

#define arginfo_class_CrxaFileInfo_getMetadata arginfo_class_Crxa_getMetadata

#define arginfo_class_CrxaFileInfo_getCrxaFlags arginfo_class_CrxaFileInfo_getCompressedSize

#define arginfo_class_CrxaFileInfo_hasMetadata arginfo_class_Crxa_getModified

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_CrxaFileInfo_isCompressed, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, compression, IS_LONG, 1, "null")
CREX_END_ARG_INFO()

#define arginfo_class_CrxaFileInfo_isCRCChecked arginfo_class_Crxa_getModified

#define arginfo_class_CrxaFileInfo_setMetadata arginfo_class_Crxa_setMetadata


CREX_METHOD(Crxa, __main);
CREX_METHOD(Crxa, __destruct);
CREX_METHOD(Crxa, addEmptyDir);
CREX_METHOD(Crxa, addFile);
CREX_METHOD(Crxa, addFromString);
CREX_METHOD(Crxa, buildFromDirectory);
CREX_METHOD(Crxa, buildFromIterator);
CREX_METHOD(Crxa, compressFiles);
CREX_METHOD(Crxa, decompressFiles);
CREX_METHOD(Crxa, compress);
CREX_METHOD(Crxa, decompress);
CREX_METHOD(Crxa, convertToExecutable);
CREX_METHOD(Crxa, convertToData);
CREX_METHOD(Crxa, copy);
CREX_METHOD(Crxa, count);
CREX_METHOD(Crxa, delete);
CREX_METHOD(Crxa, delMetadata);
CREX_METHOD(Crxa, extractTo);
CREX_METHOD(Crxa, getAlias);
CREX_METHOD(Crxa, getPath);
CREX_METHOD(Crxa, getMetadata);
CREX_METHOD(Crxa, getModified);
CREX_METHOD(Crxa, getSignature);
CREX_METHOD(Crxa, getStub);
CREX_METHOD(Crxa, getVersion);
CREX_METHOD(Crxa, hasMetadata);
CREX_METHOD(Crxa, isBuffering);
CREX_METHOD(Crxa, isCompressed);
CREX_METHOD(Crxa, isFileFormat);
CREX_METHOD(Crxa, isWritable);
CREX_METHOD(Crxa, offsetExists);
CREX_METHOD(Crxa, offsetGet);
CREX_METHOD(Crxa, offsetSet);
CREX_METHOD(Crxa, offsetUnset);
CREX_METHOD(Crxa, setAlias);
CREX_METHOD(Crxa, setDefaultStub);
CREX_METHOD(Crxa, setMetadata);
CREX_METHOD(Crxa, setSignatureAlgorithm);
CREX_METHOD(Crxa, setStub);
CREX_METHOD(Crxa, startBuffering);
CREX_METHOD(Crxa, stopBuffering);
CREX_METHOD(Crxa, apiVersion);
CREX_METHOD(Crxa, canCompress);
CREX_METHOD(Crxa, canWrite);
CREX_METHOD(Crxa, createDefaultStub);
CREX_METHOD(Crxa, getSupportedCompression);
CREX_METHOD(Crxa, getSupportedSignatures);
CREX_METHOD(Crxa, interceptFileFuncs);
CREX_METHOD(Crxa, isValidCrxaFilename);
CREX_METHOD(Crxa, loadCrxa);
CREX_METHOD(Crxa, mapCrxa);
CREX_METHOD(Crxa, running);
CREX_METHOD(Crxa, mount);
CREX_METHOD(Crxa, mungServer);
CREX_METHOD(Crxa, unlinkArchive);
CREX_METHOD(Crxa, webCrxa);
CREX_METHOD(CrxaFileInfo, __main);
CREX_METHOD(CrxaFileInfo, __destruct);
CREX_METHOD(CrxaFileInfo, chmod);
CREX_METHOD(CrxaFileInfo, compress);
CREX_METHOD(CrxaFileInfo, decompress);
CREX_METHOD(CrxaFileInfo, delMetadata);
CREX_METHOD(CrxaFileInfo, getCompressedSize);
CREX_METHOD(CrxaFileInfo, getCRC32);
CREX_METHOD(CrxaFileInfo, getContent);
CREX_METHOD(CrxaFileInfo, getMetadata);
CREX_METHOD(CrxaFileInfo, getCrxaFlags);
CREX_METHOD(CrxaFileInfo, hasMetadata);
CREX_METHOD(CrxaFileInfo, isCompressed);
CREX_METHOD(CrxaFileInfo, isCRCChecked);
CREX_METHOD(CrxaFileInfo, setMetadata);


static const crex_function_entry class_CrxaException_methods[] = {
	CREX_FE_END
};


static const crex_function_entry class_Crxa_methods[] = {
	CREX_ME(Crxa, __main, arginfo_class_Crxa___main, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, __destruct, arginfo_class_Crxa___destruct, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, addEmptyDir, arginfo_class_Crxa_addEmptyDir, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, addFile, arginfo_class_Crxa_addFile, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, addFromString, arginfo_class_Crxa_addFromString, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, buildFromDirectory, arginfo_class_Crxa_buildFromDirectory, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, buildFromIterator, arginfo_class_Crxa_buildFromIterator, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, compressFiles, arginfo_class_Crxa_compressFiles, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, decompressFiles, arginfo_class_Crxa_decompressFiles, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, compress, arginfo_class_Crxa_compress, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, decompress, arginfo_class_Crxa_decompress, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, convertToExecutable, arginfo_class_Crxa_convertToExecutable, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, convertToData, arginfo_class_Crxa_convertToData, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, copy, arginfo_class_Crxa_copy, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, count, arginfo_class_Crxa_count, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, delete, arginfo_class_Crxa_delete, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, delMetadata, arginfo_class_Crxa_delMetadata, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, extractTo, arginfo_class_Crxa_extractTo, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getAlias, arginfo_class_Crxa_getAlias, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getPath, arginfo_class_Crxa_getPath, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getMetadata, arginfo_class_Crxa_getMetadata, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getModified, arginfo_class_Crxa_getModified, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getSignature, arginfo_class_Crxa_getSignature, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getStub, arginfo_class_Crxa_getStub, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, getVersion, arginfo_class_Crxa_getVersion, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, hasMetadata, arginfo_class_Crxa_hasMetadata, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, isBuffering, arginfo_class_Crxa_isBuffering, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, isCompressed, arginfo_class_Crxa_isCompressed, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, isFileFormat, arginfo_class_Crxa_isFileFormat, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, isWritable, arginfo_class_Crxa_isWritable, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, offsetExists, arginfo_class_Crxa_offsetExists, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, offsetGet, arginfo_class_Crxa_offsetGet, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, offsetSet, arginfo_class_Crxa_offsetSet, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, offsetUnset, arginfo_class_Crxa_offsetUnset, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, setAlias, arginfo_class_Crxa_setAlias, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, setDefaultStub, arginfo_class_Crxa_setDefaultStub, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, setMetadata, arginfo_class_Crxa_setMetadata, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, setSignatureAlgorithm, arginfo_class_Crxa_setSignatureAlgorithm, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, setStub, arginfo_class_Crxa_setStub, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, startBuffering, arginfo_class_Crxa_startBuffering, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, stopBuffering, arginfo_class_Crxa_stopBuffering, CREX_ACC_PUBLIC)
	CREX_ME(Crxa, apiVersion, arginfo_class_Crxa_apiVersion, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, canCompress, arginfo_class_Crxa_canCompress, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, canWrite, arginfo_class_Crxa_canWrite, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, createDefaultStub, arginfo_class_Crxa_createDefaultStub, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, getSupportedCompression, arginfo_class_Crxa_getSupportedCompression, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, getSupportedSignatures, arginfo_class_Crxa_getSupportedSignatures, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, interceptFileFuncs, arginfo_class_Crxa_interceptFileFuncs, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, isValidCrxaFilename, arginfo_class_Crxa_isValidCrxaFilename, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, loadCrxa, arginfo_class_Crxa_loadCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, mapCrxa, arginfo_class_Crxa_mapCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, running, arginfo_class_Crxa_running, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, mount, arginfo_class_Crxa_mount, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, mungServer, arginfo_class_Crxa_mungServer, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, unlinkArchive, arginfo_class_Crxa_unlinkArchive, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_ME(Crxa, webCrxa, arginfo_class_Crxa_webCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_FE_END
};


static const crex_function_entry class_CrxaData_methods[] = {
	CREX_MALIAS(Crxa, __main, __main, arginfo_class_CrxaData___main, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, __destruct, __destruct, arginfo_class_CrxaData___destruct, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, addEmptyDir, addEmptyDir, arginfo_class_CrxaData_addEmptyDir, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, addFile, addFile, arginfo_class_CrxaData_addFile, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, addFromString, addFromString, arginfo_class_CrxaData_addFromString, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, buildFromDirectory, buildFromDirectory, arginfo_class_CrxaData_buildFromDirectory, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, buildFromIterator, buildFromIterator, arginfo_class_CrxaData_buildFromIterator, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, compressFiles, compressFiles, arginfo_class_CrxaData_compressFiles, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, decompressFiles, decompressFiles, arginfo_class_CrxaData_decompressFiles, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, compress, compress, arginfo_class_CrxaData_compress, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, decompress, decompress, arginfo_class_CrxaData_decompress, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, convertToExecutable, convertToExecutable, arginfo_class_CrxaData_convertToExecutable, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, convertToData, convertToData, arginfo_class_CrxaData_convertToData, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, copy, copy, arginfo_class_CrxaData_copy, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, count, count, arginfo_class_CrxaData_count, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, delete, delete, arginfo_class_CrxaData_delete, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, delMetadata, delMetadata, arginfo_class_CrxaData_delMetadata, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, extractTo, extractTo, arginfo_class_CrxaData_extractTo, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getAlias, getAlias, arginfo_class_CrxaData_getAlias, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getPath, getPath, arginfo_class_CrxaData_getPath, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getMetadata, getMetadata, arginfo_class_CrxaData_getMetadata, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getModified, getModified, arginfo_class_CrxaData_getModified, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getSignature, getSignature, arginfo_class_CrxaData_getSignature, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getStub, getStub, arginfo_class_CrxaData_getStub, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, getVersion, getVersion, arginfo_class_CrxaData_getVersion, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, hasMetadata, hasMetadata, arginfo_class_CrxaData_hasMetadata, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, isBuffering, isBuffering, arginfo_class_CrxaData_isBuffering, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, isCompressed, isCompressed, arginfo_class_CrxaData_isCompressed, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, isFileFormat, isFileFormat, arginfo_class_CrxaData_isFileFormat, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, isWritable, isWritable, arginfo_class_CrxaData_isWritable, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, offsetExists, offsetExists, arginfo_class_CrxaData_offsetExists, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, offsetGet, offsetGet, arginfo_class_CrxaData_offsetGet, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, offsetSet, offsetSet, arginfo_class_CrxaData_offsetSet, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, offsetUnset, offsetUnset, arginfo_class_CrxaData_offsetUnset, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, setAlias, setAlias, arginfo_class_CrxaData_setAlias, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, setDefaultStub, setDefaultStub, arginfo_class_CrxaData_setDefaultStub, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, setMetadata, setMetadata, arginfo_class_CrxaData_setMetadata, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, setSignatureAlgorithm, setSignatureAlgorithm, arginfo_class_CrxaData_setSignatureAlgorithm, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, setStub, setStub, arginfo_class_CrxaData_setStub, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, startBuffering, startBuffering, arginfo_class_CrxaData_startBuffering, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, stopBuffering, stopBuffering, arginfo_class_CrxaData_stopBuffering, CREX_ACC_PUBLIC)
	CREX_MALIAS(Crxa, apiVersion, apiVersion, arginfo_class_CrxaData_apiVersion, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, canCompress, canCompress, arginfo_class_CrxaData_canCompress, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, canWrite, canWrite, arginfo_class_CrxaData_canWrite, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, createDefaultStub, createDefaultStub, arginfo_class_CrxaData_createDefaultStub, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, getSupportedCompression, getSupportedCompression, arginfo_class_CrxaData_getSupportedCompression, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, getSupportedSignatures, getSupportedSignatures, arginfo_class_CrxaData_getSupportedSignatures, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, interceptFileFuncs, interceptFileFuncs, arginfo_class_CrxaData_interceptFileFuncs, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, isValidCrxaFilename, isValidCrxaFilename, arginfo_class_CrxaData_isValidCrxaFilename, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, loadCrxa, loadCrxa, arginfo_class_CrxaData_loadCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, mapCrxa, mapCrxa, arginfo_class_CrxaData_mapCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, running, running, arginfo_class_CrxaData_running, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, mount, mount, arginfo_class_CrxaData_mount, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, mungServer, mungServer, arginfo_class_CrxaData_mungServer, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, unlinkArchive, unlinkArchive, arginfo_class_CrxaData_unlinkArchive, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_MALIAS(Crxa, webCrxa, webCrxa, arginfo_class_CrxaData_webCrxa, CREX_ACC_PUBLIC|CREX_ACC_STATIC|CREX_ACC_FINAL)
	CREX_FE_END
};


static const crex_function_entry class_CrxaFileInfo_methods[] = {
	CREX_ME(CrxaFileInfo, __main, arginfo_class_CrxaFileInfo___main, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, __destruct, arginfo_class_CrxaFileInfo___destruct, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, chmod, arginfo_class_CrxaFileInfo_chmod, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, compress, arginfo_class_CrxaFileInfo_compress, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, decompress, arginfo_class_CrxaFileInfo_decompress, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, delMetadata, arginfo_class_CrxaFileInfo_delMetadata, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, getCompressedSize, arginfo_class_CrxaFileInfo_getCompressedSize, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, getCRC32, arginfo_class_CrxaFileInfo_getCRC32, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, getContent, arginfo_class_CrxaFileInfo_getContent, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, getMetadata, arginfo_class_CrxaFileInfo_getMetadata, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, getCrxaFlags, arginfo_class_CrxaFileInfo_getCrxaFlags, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, hasMetadata, arginfo_class_CrxaFileInfo_hasMetadata, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, isCompressed, arginfo_class_CrxaFileInfo_isCompressed, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, isCRCChecked, arginfo_class_CrxaFileInfo_isCRCChecked, CREX_ACC_PUBLIC)
	CREX_ME(CrxaFileInfo, setMetadata, arginfo_class_CrxaFileInfo_setMetadata, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_CrxaException(crex_class_entry *class_entry_Exception)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrxaException", class_CrxaException_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_Exception);

	return class_entry;
}

static crex_class_entry *register_class_Crxa(crex_class_entry *class_entry_RecursiveDirectoryIterator, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_ArrayAccess)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Crxa", class_Crxa_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RecursiveDirectoryIterator);
	crex_class_implements(class_entry, 2, class_entry_Countable, class_entry_ArrayAccess);

	zval const_BZ2_value;
	ZVAL_LONG(&const_BZ2_value, CRXA_ENT_COMPRESSED_BZ2);
	crex_string *const_BZ2_name = crex_string_init_interned("BZ2", sizeof("BZ2") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_BZ2_name, &const_BZ2_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_BZ2_name);

	zval const_GC_value;
	ZVAL_LONG(&const_GC_value, CRXA_ENT_COMPRESSED_GZ);
	crex_string *const_GC_name = crex_string_init_interned("GZ", sizeof("GZ") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_GC_name, &const_GC_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_GC_name);

	zval const_NONE_value;
	ZVAL_LONG(&const_NONE_value, CRXA_ENT_COMPRESSED_NONE);
	crex_string *const_NONE_name = crex_string_init_interned("NONE", sizeof("NONE") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_NONE_name, &const_NONE_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_NONE_name);

	zval const_CRXA_value;
	ZVAL_LONG(&const_CRXA_value, CRXA_FORMAT_CRXA);
	crex_string *const_CRXA_name = crex_string_init_interned("CRXA", sizeof("CRXA") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CRXA_name, &const_CRXA_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CRXA_name);

	zval const_TAR_value;
	ZVAL_LONG(&const_TAR_value, CRXA_FORMAT_TAR);
	crex_string *const_TAR_name = crex_string_init_interned("TAR", sizeof("TAR") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_TAR_name, &const_TAR_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_TAR_name);

	zval const_ZIP_value;
	ZVAL_LONG(&const_ZIP_value, CRXA_FORMAT_ZIP);
	crex_string *const_ZIP_name = crex_string_init_interned("ZIP", sizeof("ZIP") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_ZIP_name, &const_ZIP_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_ZIP_name);

	zval const_COMPRESSED_value;
	ZVAL_LONG(&const_COMPRESSED_value, CRXA_ENT_COMPRESSION_MASK);
	crex_string *const_COMPRESSED_name = crex_string_init_interned("COMPRESSED", sizeof("COMPRESSED") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_COMPRESSED_name, &const_COMPRESSED_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_COMPRESSED_name);

	zval const_CRX_value;
	ZVAL_LONG(&const_CRX_value, CRXA_MIME_CRX);
	crex_string *const_CRX_name = crex_string_init_interned("CRX", sizeof("CRX") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CRX_name, &const_CRX_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CRX_name);

	zval const_CRXS_value;
	ZVAL_LONG(&const_CRXS_value, CRXA_MIME_CRXS);
	crex_string *const_CRXS_name = crex_string_init_interned("CRXS", sizeof("CRXS") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_CRXS_name, &const_CRXS_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_CRXS_name);

	zval const_MD5_value;
	ZVAL_LONG(&const_MD5_value, CRXA_SIG_MD5);
	crex_string *const_MD5_name = crex_string_init_interned("MD5", sizeof("MD5") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_MD5_name, &const_MD5_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_MD5_name);

	zval const_OPENSSL_value;
	ZVAL_LONG(&const_OPENSSL_value, CRXA_SIG_OPENSSL);
	crex_string *const_OPENSSL_name = crex_string_init_interned("OPENSSL", sizeof("OPENSSL") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPENSSL_name, &const_OPENSSL_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPENSSL_name);

	zval const_OPENSSL_SHA256_value;
	ZVAL_LONG(&const_OPENSSL_SHA256_value, CRXA_SIG_OPENSSL_SHA256);
	crex_string *const_OPENSSL_SHA256_name = crex_string_init_interned("OPENSSL_SHA256", sizeof("OPENSSL_SHA256") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPENSSL_SHA256_name, &const_OPENSSL_SHA256_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPENSSL_SHA256_name);

	zval const_OPENSSL_SHA512_value;
	ZVAL_LONG(&const_OPENSSL_SHA512_value, CRXA_SIG_OPENSSL_SHA512);
	crex_string *const_OPENSSL_SHA512_name = crex_string_init_interned("OPENSSL_SHA512", sizeof("OPENSSL_SHA512") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_OPENSSL_SHA512_name, &const_OPENSSL_SHA512_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_OPENSSL_SHA512_name);

	zval const_SHA1_value;
	ZVAL_LONG(&const_SHA1_value, CRXA_SIG_SHA1);
	crex_string *const_SHA1_name = crex_string_init_interned("SHA1", sizeof("SHA1") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_SHA1_name, &const_SHA1_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_SHA1_name);

	zval const_SHA256_value;
	ZVAL_LONG(&const_SHA256_value, CRXA_SIG_SHA256);
	crex_string *const_SHA256_name = crex_string_init_interned("SHA256", sizeof("SHA256") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_SHA256_name, &const_SHA256_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_SHA256_name);

	zval const_SHA512_value;
	ZVAL_LONG(&const_SHA512_value, CRXA_SIG_SHA512);
	crex_string *const_SHA512_name = crex_string_init_interned("SHA512", sizeof("SHA512") - 1, 1);
	crex_declare_typed_class_constant(class_entry, const_SHA512_name, &const_SHA512_value, CREX_ACC_PUBLIC, NULL, (crex_type) CREX_TYPE_INIT_MASK(MAY_BE_LONG));
	crex_string_release(const_SHA512_name);

	return class_entry;
}

static crex_class_entry *register_class_CrxaData(crex_class_entry *class_entry_RecursiveDirectoryIterator, crex_class_entry *class_entry_Countable, crex_class_entry *class_entry_ArrayAccess)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrxaData", class_CrxaData_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_RecursiveDirectoryIterator);
	crex_class_implements(class_entry, 2, class_entry_Countable, class_entry_ArrayAccess);

	return class_entry;
}

static crex_class_entry *register_class_CrxaFileInfo(crex_class_entry *class_entry_SplFileInfo)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "CrxaFileInfo", class_CrxaFileInfo_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplFileInfo);

	return class_entry;
}
