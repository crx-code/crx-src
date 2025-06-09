/* This is a generated file, edit the .stub.crx file instead.
 * Stub hash: 7e67d07b6079c39a091e91dfddfbe7170067955e */

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplFileInfo___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo_getPath, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileInfo_getFilename arginfo_class_SplFileInfo_getPath

#define arginfo_class_SplFileInfo_getExtension arginfo_class_SplFileInfo_getPath

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo_getBasename, 0, 0, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, suffix, IS_STRING, 0, "\"\"")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileInfo_getPathname arginfo_class_SplFileInfo_getPath

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileInfo_getPerms, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileInfo_getInode arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getSize arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getOwner arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getGroup arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getATime arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getMTime arginfo_class_SplFileInfo_getPerms

#define arginfo_class_SplFileInfo_getCTime arginfo_class_SplFileInfo_getPerms

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileInfo_getType, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo_isWritable, 0, 0, _IS_BOOL, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileInfo_isReadable arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileInfo_isExecutable arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileInfo_isFile arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileInfo_isDir arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileInfo_isLink arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileInfo_getLinkTarget arginfo_class_SplFileInfo_getType

#define arginfo_class_SplFileInfo_getRealPath arginfo_class_SplFileInfo_getType

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SplFileInfo_getFileInfo, 0, 0, SplFileInfo, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SplFileInfo_getPathInfo, 0, 0, SplFileInfo, 1)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 1, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_SplFileInfo_openFile, 0, 0, SplFileObject, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"r\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, useIncludePath, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo_setFileClass, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 0, "SplFileObject::class")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo_setInfoClass, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 0, "SplFileInfo::class")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo___toString, 0, 0, IS_STRING, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo___debugInfo, 0, 0, IS_ARRAY, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileInfo__bad_state_ex, 0, 0, IS_VOID, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_DirectoryIterator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DirectoryIterator_getFilename arginfo_class_SplFileInfo_getPath

#define arginfo_class_DirectoryIterator_getExtension arginfo_class_SplFileInfo_getPath

#define arginfo_class_DirectoryIterator_getBasename arginfo_class_SplFileInfo_getBasename

#define arginfo_class_DirectoryIterator_isDot arginfo_class_SplFileInfo_isWritable

#define arginfo_class_DirectoryIterator_rewind arginfo_class_SplFileInfo__bad_state_ex

#define arginfo_class_DirectoryIterator_valid arginfo_class_SplFileInfo_isWritable

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DirectoryIterator_key, 0, 0, IS_MIXED, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DirectoryIterator_current arginfo_class_DirectoryIterator_key

#define arginfo_class_DirectoryIterator_next arginfo_class_SplFileInfo__bad_state_ex

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_DirectoryIterator_seek, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_DirectoryIterator___toString arginfo_class_SplFileInfo___toString

CREX_BEGIN_ARG_INFO_EX(arginfo_class_FilesystemIterator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FilesystemIterator::KEY_AS_PATHNAME | FilesystemIterator::CURRENT_AS_FILEINFO | FilesystemIterator::SKIP_DOTS")
CREX_END_ARG_INFO()

#define arginfo_class_FilesystemIterator_rewind arginfo_class_SplFileInfo__bad_state_ex

#define arginfo_class_FilesystemIterator_key arginfo_class_SplFileInfo_getPath

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_FilesystemIterator_current, 0, 0, SplFileInfo|FilesystemIterator, MAY_BE_STRING)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_FilesystemIterator_getFlags, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_FilesystemIterator_setFlags, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_INFO_EX(arginfo_class_RecursiveDirectoryIterator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, directory, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FilesystemIterator::KEY_AS_PATHNAME | FilesystemIterator::CURRENT_AS_FILEINFO")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_RecursiveDirectoryIterator_hasChildren, 0, 0, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, allowLinks, _IS_BOOL, 0, "false")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_OBJ_INFO_EX(arginfo_class_RecursiveDirectoryIterator_getChildren, 0, 0, RecursiveDirectoryIterator, 0)
CREX_END_ARG_INFO()

#define arginfo_class_RecursiveDirectoryIterator_getSubPath arginfo_class_SplFileInfo_getPath

#define arginfo_class_RecursiveDirectoryIterator_getSubPathname arginfo_class_SplFileInfo_getPath

#if defined(HAVE_GLOB)
CREX_BEGIN_ARG_INFO_EX(arginfo_class_GlobIterator___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, pattern, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, flags, IS_LONG, 0, "FilesystemIterator::KEY_AS_PATHNAME | FilesystemIterator::CURRENT_AS_FILEINFO")
CREX_END_ARG_INFO()
#endif

#if defined(HAVE_GLOB)
CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_GlobIterator_count, 0, 0, IS_LONG, 0)
CREX_END_ARG_INFO()
#endif

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplFileObject___main, 0, 0, 1)
	CREX_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 0, "\"r\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, useIncludePath, _IS_BOOL, 0, "false")
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(0, context, "null")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_rewind arginfo_class_SplFileInfo__bad_state_ex

#define arginfo_class_SplFileObject_eof arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileObject_valid arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileObject_fgets arginfo_class_SplFileInfo_getPath

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_fread, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, length, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_fgetcsv, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_fputcsv, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, eol, IS_STRING, 0, "\"\\n\"")
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_setCsvControl, 0, 0, IS_VOID, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, separator, IS_STRING, 0, "\",\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, enclosure, IS_STRING, 0, "\"\\\"\"")
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, escape, IS_STRING, 0, "\"\\\\\"")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_getCsvControl arginfo_class_SplFileInfo___debugInfo

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_flock, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, operation, IS_LONG, 0)
	CREX_ARG_INFO_WITH_DEFAULT_VALUE(1, wouldBlock, "null")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_fflush arginfo_class_SplFileInfo_isWritable

#define arginfo_class_SplFileObject_ftell arginfo_class_SplFileInfo_getPerms

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_fseek, 0, 1, IS_LONG, 0)
	CREX_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, whence, IS_LONG, 0, "SEEK_SET")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_fgetc arginfo_class_SplFileInfo_getType

#define arginfo_class_SplFileObject_fpassthru arginfo_class_FilesystemIterator_getFlags

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_fscanf, 0, 1, MAY_BE_ARRAY|MAY_BE_LONG|MAY_BE_NULL)
	CREX_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	CREX_ARG_VARIADIC_TYPE_INFO(1, vars, IS_MIXED, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_fwrite, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	CREX_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 0, "0")
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_fstat arginfo_class_SplFileInfo___debugInfo

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_ftruncate, 0, 1, _IS_BOOL, 0)
	CREX_ARG_TYPE_INFO(0, size, IS_LONG, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_MASK_EX(arginfo_class_SplFileObject_current, 0, 0, MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_FALSE)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_key arginfo_class_FilesystemIterator_getFlags

#define arginfo_class_SplFileObject_next arginfo_class_SplFileInfo__bad_state_ex

#define arginfo_class_SplFileObject_setFlags arginfo_class_FilesystemIterator_setFlags

#define arginfo_class_SplFileObject_getFlags arginfo_class_FilesystemIterator_getFlags

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_setMaxLineLen, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, maxLength, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_getMaxLineLen arginfo_class_FilesystemIterator_getFlags

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_hasChildren, 0, 0, IS_FALSE, 0)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_getChildren, 0, 0, IS_NULL, 1)
CREX_END_ARG_INFO()

CREX_BEGIN_ARG_WITH_TENTATIVE_RETURN_TYPE_INFO_EX(arginfo_class_SplFileObject_seek, 0, 1, IS_VOID, 0)
	CREX_ARG_TYPE_INFO(0, line, IS_LONG, 0)
CREX_END_ARG_INFO()

#define arginfo_class_SplFileObject_getCurrentLine arginfo_class_SplFileInfo_getPath

#define arginfo_class_SplFileObject___toString arginfo_class_SplFileInfo___toString

CREX_BEGIN_ARG_INFO_EX(arginfo_class_SplTempFileObject___main, 0, 0, 0)
	CREX_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxMemory, IS_LONG, 0, "2 * 1024 * 1024")
CREX_END_ARG_INFO()


CREX_METHOD(SplFileInfo, __main);
CREX_METHOD(SplFileInfo, getPath);
CREX_METHOD(SplFileInfo, getFilename);
CREX_METHOD(SplFileInfo, getExtension);
CREX_METHOD(SplFileInfo, getBasename);
CREX_METHOD(SplFileInfo, getPathname);
CREX_METHOD(SplFileInfo, getPerms);
CREX_METHOD(SplFileInfo, getInode);
CREX_METHOD(SplFileInfo, getSize);
CREX_METHOD(SplFileInfo, getOwner);
CREX_METHOD(SplFileInfo, getGroup);
CREX_METHOD(SplFileInfo, getATime);
CREX_METHOD(SplFileInfo, getMTime);
CREX_METHOD(SplFileInfo, getCTime);
CREX_METHOD(SplFileInfo, getType);
CREX_METHOD(SplFileInfo, isWritable);
CREX_METHOD(SplFileInfo, isReadable);
CREX_METHOD(SplFileInfo, isExecutable);
CREX_METHOD(SplFileInfo, isFile);
CREX_METHOD(SplFileInfo, isDir);
CREX_METHOD(SplFileInfo, isLink);
CREX_METHOD(SplFileInfo, getLinkTarget);
CREX_METHOD(SplFileInfo, getRealPath);
CREX_METHOD(SplFileInfo, getFileInfo);
CREX_METHOD(SplFileInfo, getPathInfo);
CREX_METHOD(SplFileInfo, openFile);
CREX_METHOD(SplFileInfo, setFileClass);
CREX_METHOD(SplFileInfo, setInfoClass);
CREX_METHOD(SplFileInfo, __debugInfo);
CREX_METHOD(SplFileInfo, _bad_state_ex);
CREX_METHOD(DirectoryIterator, __main);
CREX_METHOD(DirectoryIterator, getFilename);
CREX_METHOD(DirectoryIterator, getExtension);
CREX_METHOD(DirectoryIterator, getBasename);
CREX_METHOD(DirectoryIterator, isDot);
CREX_METHOD(DirectoryIterator, rewind);
CREX_METHOD(DirectoryIterator, valid);
CREX_METHOD(DirectoryIterator, key);
CREX_METHOD(DirectoryIterator, current);
CREX_METHOD(DirectoryIterator, next);
CREX_METHOD(DirectoryIterator, seek);
CREX_METHOD(FilesystemIterator, __main);
CREX_METHOD(FilesystemIterator, rewind);
CREX_METHOD(FilesystemIterator, key);
CREX_METHOD(FilesystemIterator, current);
CREX_METHOD(FilesystemIterator, getFlags);
CREX_METHOD(FilesystemIterator, setFlags);
CREX_METHOD(RecursiveDirectoryIterator, __main);
CREX_METHOD(RecursiveDirectoryIterator, hasChildren);
CREX_METHOD(RecursiveDirectoryIterator, getChildren);
CREX_METHOD(RecursiveDirectoryIterator, getSubPath);
CREX_METHOD(RecursiveDirectoryIterator, getSubPathname);
#if defined(HAVE_GLOB)
CREX_METHOD(GlobIterator, __main);
#endif
#if defined(HAVE_GLOB)
CREX_METHOD(GlobIterator, count);
#endif
CREX_METHOD(SplFileObject, __main);
CREX_METHOD(SplFileObject, rewind);
CREX_METHOD(SplFileObject, eof);
CREX_METHOD(SplFileObject, valid);
CREX_METHOD(SplFileObject, fgets);
CREX_METHOD(SplFileObject, fread);
CREX_METHOD(SplFileObject, fgetcsv);
CREX_METHOD(SplFileObject, fputcsv);
CREX_METHOD(SplFileObject, setCsvControl);
CREX_METHOD(SplFileObject, getCsvControl);
CREX_METHOD(SplFileObject, flock);
CREX_METHOD(SplFileObject, fflush);
CREX_METHOD(SplFileObject, ftell);
CREX_METHOD(SplFileObject, fseek);
CREX_METHOD(SplFileObject, fgetc);
CREX_METHOD(SplFileObject, fpassthru);
CREX_METHOD(SplFileObject, fscanf);
CREX_METHOD(SplFileObject, fwrite);
CREX_METHOD(SplFileObject, fstat);
CREX_METHOD(SplFileObject, ftruncate);
CREX_METHOD(SplFileObject, current);
CREX_METHOD(SplFileObject, key);
CREX_METHOD(SplFileObject, next);
CREX_METHOD(SplFileObject, setFlags);
CREX_METHOD(SplFileObject, getFlags);
CREX_METHOD(SplFileObject, setMaxLineLen);
CREX_METHOD(SplFileObject, getMaxLineLen);
CREX_METHOD(SplFileObject, hasChildren);
CREX_METHOD(SplFileObject, getChildren);
CREX_METHOD(SplFileObject, seek);
CREX_METHOD(SplFileObject, __toString);
CREX_METHOD(SplTempFileObject, __main);


static const crex_function_entry class_SplFileInfo_methods[] = {
	CREX_ME(SplFileInfo, __main, arginfo_class_SplFileInfo___main, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getPath, arginfo_class_SplFileInfo_getPath, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getFilename, arginfo_class_SplFileInfo_getFilename, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getExtension, arginfo_class_SplFileInfo_getExtension, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getBasename, arginfo_class_SplFileInfo_getBasename, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getPathname, arginfo_class_SplFileInfo_getPathname, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getPerms, arginfo_class_SplFileInfo_getPerms, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getInode, arginfo_class_SplFileInfo_getInode, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getSize, arginfo_class_SplFileInfo_getSize, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getOwner, arginfo_class_SplFileInfo_getOwner, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getGroup, arginfo_class_SplFileInfo_getGroup, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getATime, arginfo_class_SplFileInfo_getATime, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getMTime, arginfo_class_SplFileInfo_getMTime, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getCTime, arginfo_class_SplFileInfo_getCTime, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getType, arginfo_class_SplFileInfo_getType, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isWritable, arginfo_class_SplFileInfo_isWritable, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isReadable, arginfo_class_SplFileInfo_isReadable, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isExecutable, arginfo_class_SplFileInfo_isExecutable, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isFile, arginfo_class_SplFileInfo_isFile, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isDir, arginfo_class_SplFileInfo_isDir, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, isLink, arginfo_class_SplFileInfo_isLink, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getLinkTarget, arginfo_class_SplFileInfo_getLinkTarget, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getRealPath, arginfo_class_SplFileInfo_getRealPath, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getFileInfo, arginfo_class_SplFileInfo_getFileInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, getPathInfo, arginfo_class_SplFileInfo_getPathInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, openFile, arginfo_class_SplFileInfo_openFile, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, setFileClass, arginfo_class_SplFileInfo_setFileClass, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, setInfoClass, arginfo_class_SplFileInfo_setInfoClass, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplFileInfo, __toString, getPathname, arginfo_class_SplFileInfo___toString, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, __debugInfo, arginfo_class_SplFileInfo___debugInfo, CREX_ACC_PUBLIC)
	CREX_ME(SplFileInfo, _bad_state_ex, arginfo_class_SplFileInfo__bad_state_ex, CREX_ACC_PUBLIC|CREX_ACC_FINAL|CREX_ACC_DEPRECATED)
	CREX_FE_END
};


static const crex_function_entry class_DirectoryIterator_methods[] = {
	CREX_ME(DirectoryIterator, __main, arginfo_class_DirectoryIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, getFilename, arginfo_class_DirectoryIterator_getFilename, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, getExtension, arginfo_class_DirectoryIterator_getExtension, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, getBasename, arginfo_class_DirectoryIterator_getBasename, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, isDot, arginfo_class_DirectoryIterator_isDot, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, rewind, arginfo_class_DirectoryIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, valid, arginfo_class_DirectoryIterator_valid, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, key, arginfo_class_DirectoryIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, current, arginfo_class_DirectoryIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, next, arginfo_class_DirectoryIterator_next, CREX_ACC_PUBLIC)
	CREX_ME(DirectoryIterator, seek, arginfo_class_DirectoryIterator_seek, CREX_ACC_PUBLIC)
	CREX_MALIAS(DirectoryIterator, __toString, getFilename, arginfo_class_DirectoryIterator___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_FilesystemIterator_methods[] = {
	CREX_ME(FilesystemIterator, __main, arginfo_class_FilesystemIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(FilesystemIterator, rewind, arginfo_class_FilesystemIterator_rewind, CREX_ACC_PUBLIC)
	CREX_ME(FilesystemIterator, key, arginfo_class_FilesystemIterator_key, CREX_ACC_PUBLIC)
	CREX_ME(FilesystemIterator, current, arginfo_class_FilesystemIterator_current, CREX_ACC_PUBLIC)
	CREX_ME(FilesystemIterator, getFlags, arginfo_class_FilesystemIterator_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(FilesystemIterator, setFlags, arginfo_class_FilesystemIterator_setFlags, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_RecursiveDirectoryIterator_methods[] = {
	CREX_ME(RecursiveDirectoryIterator, __main, arginfo_class_RecursiveDirectoryIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveDirectoryIterator, hasChildren, arginfo_class_RecursiveDirectoryIterator_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveDirectoryIterator, getChildren, arginfo_class_RecursiveDirectoryIterator_getChildren, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveDirectoryIterator, getSubPath, arginfo_class_RecursiveDirectoryIterator_getSubPath, CREX_ACC_PUBLIC)
	CREX_ME(RecursiveDirectoryIterator, getSubPathname, arginfo_class_RecursiveDirectoryIterator_getSubPathname, CREX_ACC_PUBLIC)
	CREX_FE_END
};


#if defined(HAVE_GLOB)
static const crex_function_entry class_GlobIterator_methods[] = {
	CREX_ME(GlobIterator, __main, arginfo_class_GlobIterator___main, CREX_ACC_PUBLIC)
	CREX_ME(GlobIterator, count, arginfo_class_GlobIterator_count, CREX_ACC_PUBLIC)
	CREX_FE_END
};
#endif


static const crex_function_entry class_SplFileObject_methods[] = {
	CREX_ME(SplFileObject, __main, arginfo_class_SplFileObject___main, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, rewind, arginfo_class_SplFileObject_rewind, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, eof, arginfo_class_SplFileObject_eof, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, valid, arginfo_class_SplFileObject_valid, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fgets, arginfo_class_SplFileObject_fgets, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fread, arginfo_class_SplFileObject_fread, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fgetcsv, arginfo_class_SplFileObject_fgetcsv, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fputcsv, arginfo_class_SplFileObject_fputcsv, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, setCsvControl, arginfo_class_SplFileObject_setCsvControl, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, getCsvControl, arginfo_class_SplFileObject_getCsvControl, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, flock, arginfo_class_SplFileObject_flock, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fflush, arginfo_class_SplFileObject_fflush, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, ftell, arginfo_class_SplFileObject_ftell, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fseek, arginfo_class_SplFileObject_fseek, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fgetc, arginfo_class_SplFileObject_fgetc, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fpassthru, arginfo_class_SplFileObject_fpassthru, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fscanf, arginfo_class_SplFileObject_fscanf, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fwrite, arginfo_class_SplFileObject_fwrite, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, fstat, arginfo_class_SplFileObject_fstat, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, ftruncate, arginfo_class_SplFileObject_ftruncate, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, current, arginfo_class_SplFileObject_current, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, key, arginfo_class_SplFileObject_key, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, next, arginfo_class_SplFileObject_next, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, setFlags, arginfo_class_SplFileObject_setFlags, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, getFlags, arginfo_class_SplFileObject_getFlags, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, setMaxLineLen, arginfo_class_SplFileObject_setMaxLineLen, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, getMaxLineLen, arginfo_class_SplFileObject_getMaxLineLen, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, hasChildren, arginfo_class_SplFileObject_hasChildren, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, getChildren, arginfo_class_SplFileObject_getChildren, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, seek, arginfo_class_SplFileObject_seek, CREX_ACC_PUBLIC)
	CREX_MALIAS(SplFileObject, getCurrentLine, fgets, arginfo_class_SplFileObject_getCurrentLine, CREX_ACC_PUBLIC)
	CREX_ME(SplFileObject, __toString, arginfo_class_SplFileObject___toString, CREX_ACC_PUBLIC)
	CREX_FE_END
};


static const crex_function_entry class_SplTempFileObject_methods[] = {
	CREX_ME(SplTempFileObject, __main, arginfo_class_SplTempFileObject___main, CREX_ACC_PUBLIC)
	CREX_FE_END
};

static crex_class_entry *register_class_SplFileInfo(crex_class_entry *class_entry_Stringable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplFileInfo", class_SplFileInfo_methods);
	class_entry = crex_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= CREX_ACC_NOT_SERIALIZABLE;
	crex_class_implements(class_entry, 1, class_entry_Stringable);

	return class_entry;
}

static crex_class_entry *register_class_DirectoryIterator(crex_class_entry *class_entry_SplFileInfo, crex_class_entry *class_entry_SeekableIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DirectoryIterator", class_DirectoryIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplFileInfo);
	crex_class_implements(class_entry, 1, class_entry_SeekableIterator);

	return class_entry;
}

static crex_class_entry *register_class_FilesystemIterator(crex_class_entry *class_entry_DirectoryIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FilesystemIterator", class_FilesystemIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_DirectoryIterator);

	zval const_CURRENT_MODE_MASK_value;
	ZVAL_LONG(&const_CURRENT_MODE_MASK_value, SPL_FILE_DIR_CURRENT_MODE_MASK);
	crex_string *const_CURRENT_MODE_MASK_name = crex_string_init_interned("CURRENT_MODE_MASK", sizeof("CURRENT_MODE_MASK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENT_MODE_MASK_name, &const_CURRENT_MODE_MASK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENT_MODE_MASK_name);

	zval const_CURRENT_AS_PATHNAME_value;
	ZVAL_LONG(&const_CURRENT_AS_PATHNAME_value, SPL_FILE_DIR_CURRENT_AS_PATHNAME);
	crex_string *const_CURRENT_AS_PATHNAME_name = crex_string_init_interned("CURRENT_AS_PATHNAME", sizeof("CURRENT_AS_PATHNAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENT_AS_PATHNAME_name, &const_CURRENT_AS_PATHNAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENT_AS_PATHNAME_name);

	zval const_CURRENT_AS_FILEINFO_value;
	ZVAL_LONG(&const_CURRENT_AS_FILEINFO_value, SPL_FILE_DIR_CURRENT_AS_FILEINFO);
	crex_string *const_CURRENT_AS_FILEINFO_name = crex_string_init_interned("CURRENT_AS_FILEINFO", sizeof("CURRENT_AS_FILEINFO") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENT_AS_FILEINFO_name, &const_CURRENT_AS_FILEINFO_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENT_AS_FILEINFO_name);

	zval const_CURRENT_AS_SELF_value;
	ZVAL_LONG(&const_CURRENT_AS_SELF_value, SPL_FILE_DIR_CURRENT_AS_SELF);
	crex_string *const_CURRENT_AS_SELF_name = crex_string_init_interned("CURRENT_AS_SELF", sizeof("CURRENT_AS_SELF") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_CURRENT_AS_SELF_name, &const_CURRENT_AS_SELF_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_CURRENT_AS_SELF_name);

	zval const_KEY_MODE_MASK_value;
	ZVAL_LONG(&const_KEY_MODE_MASK_value, SPL_FILE_DIR_KEY_MODE_MASK);
	crex_string *const_KEY_MODE_MASK_name = crex_string_init_interned("KEY_MODE_MASK", sizeof("KEY_MODE_MASK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_MODE_MASK_name, &const_KEY_MODE_MASK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_MODE_MASK_name);

	zval const_KEY_AS_PATHNAME_value;
	ZVAL_LONG(&const_KEY_AS_PATHNAME_value, SPL_FILE_DIR_KEY_AS_PATHNAME);
	crex_string *const_KEY_AS_PATHNAME_name = crex_string_init_interned("KEY_AS_PATHNAME", sizeof("KEY_AS_PATHNAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_AS_PATHNAME_name, &const_KEY_AS_PATHNAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_AS_PATHNAME_name);

	zval const_FOLLOW_SYMLINKS_value;
	ZVAL_LONG(&const_FOLLOW_SYMLINKS_value, SPL_FILE_DIR_FOLLOW_SYMLINKS);
	crex_string *const_FOLLOW_SYMLINKS_name = crex_string_init_interned("FOLLOW_SYMLINKS", sizeof("FOLLOW_SYMLINKS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_FOLLOW_SYMLINKS_name, &const_FOLLOW_SYMLINKS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_FOLLOW_SYMLINKS_name);

	zval const_KEY_AS_FILENAME_value;
	ZVAL_LONG(&const_KEY_AS_FILENAME_value, SPL_FILE_DIR_KEY_AS_FILENAME);
	crex_string *const_KEY_AS_FILENAME_name = crex_string_init_interned("KEY_AS_FILENAME", sizeof("KEY_AS_FILENAME") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_KEY_AS_FILENAME_name, &const_KEY_AS_FILENAME_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_KEY_AS_FILENAME_name);

	zval const_NEW_CURRENT_AND_KEY_value;
	ZVAL_LONG(&const_NEW_CURRENT_AND_KEY_value, SPL_FILE_NEW_CURRENT_AND_KEY);
	crex_string *const_NEW_CURRENT_AND_KEY_name = crex_string_init_interned("NEW_CURRENT_AND_KEY", sizeof("NEW_CURRENT_AND_KEY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_NEW_CURRENT_AND_KEY_name, &const_NEW_CURRENT_AND_KEY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_NEW_CURRENT_AND_KEY_name);

	zval const_OTHER_MODE_MASK_value;
	ZVAL_LONG(&const_OTHER_MODE_MASK_value, SPL_FILE_DIR_OTHERS_MASK);
	crex_string *const_OTHER_MODE_MASK_name = crex_string_init_interned("OTHER_MODE_MASK", sizeof("OTHER_MODE_MASK") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_OTHER_MODE_MASK_name, &const_OTHER_MODE_MASK_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_OTHER_MODE_MASK_name);

	zval const_SKIP_DOTS_value;
	ZVAL_LONG(&const_SKIP_DOTS_value, SPL_FILE_DIR_SKIPDOTS);
	crex_string *const_SKIP_DOTS_name = crex_string_init_interned("SKIP_DOTS", sizeof("SKIP_DOTS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SKIP_DOTS_name, &const_SKIP_DOTS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SKIP_DOTS_name);

	zval const_UNIX_PATHS_value;
	ZVAL_LONG(&const_UNIX_PATHS_value, SPL_FILE_DIR_UNIXPATHS);
	crex_string *const_UNIX_PATHS_name = crex_string_init_interned("UNIX_PATHS", sizeof("UNIX_PATHS") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_UNIX_PATHS_name, &const_UNIX_PATHS_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_UNIX_PATHS_name);

	return class_entry;
}

static crex_class_entry *register_class_RecursiveDirectoryIterator(crex_class_entry *class_entry_FilesystemIterator, crex_class_entry *class_entry_RecursiveIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "RecursiveDirectoryIterator", class_RecursiveDirectoryIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FilesystemIterator);
	crex_class_implements(class_entry, 1, class_entry_RecursiveIterator);

	return class_entry;
}

#if defined(HAVE_GLOB)
static crex_class_entry *register_class_GlobIterator(crex_class_entry *class_entry_FilesystemIterator, crex_class_entry *class_entry_Countable)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "GlobIterator", class_GlobIterator_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_FilesystemIterator);
	crex_class_implements(class_entry, 1, class_entry_Countable);

	return class_entry;
}
#endif

static crex_class_entry *register_class_SplFileObject(crex_class_entry *class_entry_SplFileInfo, crex_class_entry *class_entry_RecursiveIterator, crex_class_entry *class_entry_SeekableIterator)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplFileObject", class_SplFileObject_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplFileInfo);
	crex_class_implements(class_entry, 2, class_entry_RecursiveIterator, class_entry_SeekableIterator);

	zval const_DROP_NEW_LINE_value;
	ZVAL_LONG(&const_DROP_NEW_LINE_value, SPL_FILE_OBJECT_DROP_NEW_LINE);
	crex_string *const_DROP_NEW_LINE_name = crex_string_init_interned("DROP_NEW_LINE", sizeof("DROP_NEW_LINE") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_DROP_NEW_LINE_name, &const_DROP_NEW_LINE_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_DROP_NEW_LINE_name);

	zval const_READ_AHEAD_value;
	ZVAL_LONG(&const_READ_AHEAD_value, SPL_FILE_OBJECT_READ_AHEAD);
	crex_string *const_READ_AHEAD_name = crex_string_init_interned("READ_AHEAD", sizeof("READ_AHEAD") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_READ_AHEAD_name, &const_READ_AHEAD_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_READ_AHEAD_name);

	zval const_SKIP_EMPTY_value;
	ZVAL_LONG(&const_SKIP_EMPTY_value, SPL_FILE_OBJECT_SKIP_EMPTY);
	crex_string *const_SKIP_EMPTY_name = crex_string_init_interned("SKIP_EMPTY", sizeof("SKIP_EMPTY") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_SKIP_EMPTY_name, &const_SKIP_EMPTY_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_SKIP_EMPTY_name);

	zval const_READ_CSV_value;
	ZVAL_LONG(&const_READ_CSV_value, SPL_FILE_OBJECT_READ_CSV);
	crex_string *const_READ_CSV_name = crex_string_init_interned("READ_CSV", sizeof("READ_CSV") - 1, 1);
	crex_declare_class_constant_ex(class_entry, const_READ_CSV_name, &const_READ_CSV_value, CREX_ACC_PUBLIC, NULL);
	crex_string_release(const_READ_CSV_name);

	return class_entry;
}

static crex_class_entry *register_class_SplTempFileObject(crex_class_entry *class_entry_SplFileObject)
{
	crex_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SplTempFileObject", class_SplTempFileObject_methods);
	class_entry = crex_register_internal_class_ex(&ce, class_entry_SplFileObject);

	return class_entry;
}
