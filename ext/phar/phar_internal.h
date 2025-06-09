/*
  +----------------------------------------------------------------------+
  | crxa crx single-file executable CRX extension                        |
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
  | Authors: Gregory Beaver <cellog@crx.net>                             |
  |          Marcus Boerger <helly@crx.net>                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include "crx.h"
#include "tar.h"
#include "crx_ini.h"
#include "crex_constants.h"
#include "crex_execute.h"
#include "crex_exceptions.h"
#include "crex_hash.h"
#include "crex_interfaces.h"
#include "crex_operators.h"
#include "crex_sort.h"
#include "crex_vm.h"
#include "crex_smart_str.h"
#include "main/crx_streams.h"
#include "main/streams/crx_stream_plain_wrapper.h"
#include "main/SAPI.h"
#include "main/crx_main.h"
#include "main/crx_open_temporary_file.h"
#include "ext/standard/info.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/file.h"
#include "ext/standard/crx_string.h"
#include "ext/standard/url.h"
#include "ext/standard/crc32.h"
#include "ext/standard/md5.h"
#include "ext/standard/sha1.h"
#include "ext/standard/crx_var.h"
#include "ext/standard/crx_versioning.h"
#include "Crex/crex_virtual_cwd.h"
#include "ext/spl/spl_array.h"
#include "ext/spl/spl_directory.h"
#include "ext/spl/spl_engine.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/spl/spl_iterators.h"
#include "crx_crxa.h"
#include "ext/hash/crx_hash.h"
#include "ext/hash/crx_hash_sha.h"

/* CRX_ because this is public information via MINFO */
#define CRX_CRXA_API_VERSION      "1.1.1"
/* x.y.z maps to 0xyz0 */
#define CRXA_API_VERSION          0x1110
/* if we bump CRXA_API_VERSION, change this from 0x1100 to CRXA_API_VERSION */
#define CRXA_API_VERSION_NODIR    0x1100
#define CRXA_API_MIN_DIR          0x1110
#define CRXA_API_MIN_READ         0x1000
#define CRXA_API_MAJORVERSION     0x1000
#define CRXA_API_MAJORVER_MASK    0xF000
#define CRXA_API_VER_MASK         0xFFF0

#define CRXA_HDR_COMPRESSION_MASK 0x0000F000
#define CRXA_HDR_COMPRESSED_NONE  0x00000000
#define CRXA_HDR_COMPRESSED_GZ    0x00001000
#define CRXA_HDR_COMPRESSED_BZ2   0x00002000
#define CRXA_HDR_SIGNATURE        0x00010000

/* flags for defining that the entire file should be compressed */
#define CRXA_FILE_COMPRESSION_MASK 0x00F00000
#define CRXA_FILE_COMPRESSED_NONE  0x00000000
#define CRXA_FILE_COMPRESSED_GZ    0x00100000
#define CRXA_FILE_COMPRESSED_BZ2   0x00200000

#define CRXA_SIG_MD5              0x0001
#define CRXA_SIG_SHA1             0x0002
#define CRXA_SIG_SHA256           0x0003
#define CRXA_SIG_SHA512           0x0004
#define CRXA_SIG_OPENSSL          0x0010
#define CRXA_SIG_OPENSSL_SHA256   0x0011
#define CRXA_SIG_OPENSSL_SHA512   0x0012

/* flags byte for each file adheres to these bitmasks.
   All unused values are reserved */
#define CRXA_ENT_COMPRESSION_MASK 0x0000F000
#define CRXA_ENT_COMPRESSED_NONE  0x00000000
#define CRXA_ENT_COMPRESSED_GZ    0x00001000
#define CRXA_ENT_COMPRESSED_BZ2   0x00002000

#define CRXA_ENT_PERM_MASK        0x000001FF
#define CRXA_ENT_PERM_MASK_USR    0x000001C0
#define CRXA_ENT_PERM_SHIFT_USR   6
#define CRXA_ENT_PERM_MASK_GRP    0x00000038
#define CRXA_ENT_PERM_SHIFT_GRP   3
#define CRXA_ENT_PERM_MASK_OTH    0x00000007
#define CRXA_ENT_PERM_DEF_FILE    0x000001B6
#define CRXA_ENT_PERM_DEF_DIR     0x000001FF

#define CRXA_FORMAT_SAME    0
#define CRXA_FORMAT_CRXA    1
#define CRXA_FORMAT_TAR     2
#define CRXA_FORMAT_ZIP     3

#define TAR_FILE    '0'
#define TAR_LINK    '1'
#define TAR_SYMLINK '2'
#define TAR_DIR     '5'
#define TAR_NEW     '8'
#define TAR_GLOBAL_HDR 'g'
#define TAR_FILE_HDR   'x'

#define CRXA_MUNG_CRX_SELF			(1<<0)
#define CRXA_MUNG_REQUEST_URI		(1<<1)
#define CRXA_MUNG_SCRIPT_NAME		(1<<2)
#define CRXA_MUNG_SCRIPT_FILENAME	(1<<3)

typedef struct _crxa_entry_fp crxa_entry_fp;
typedef struct _crxa_archive_data crxa_archive_data;

CREX_BEGIN_MODULE_GLOBALS(crxa)
	/* a list of crxa_archive_data objects that reference a cached crxa, so
	   that if copy-on-write is performed, we can swap them out for the new value */
	HashTable   crxa_persist_map;
	HashTable   crxa_fname_map;
	/* for cached crxas, this is a per-process store of fp/ufp */
	crxa_entry_fp *cached_fp;
	HashTable   crxa_alias_map;
	int         crxa_SERVER_mung_list;
	int         readonly;
	char*       cache_list;
	int         manifest_cached;
	int         persist;
	int         has_zlib;
	int         has_bz2;
	bool   readonly_orig;
	bool   require_hash_orig;
	bool   intercepted;
	int         request_init;
	int         require_hash;
	int         request_done;
	int         request_ends;
	zif_handler orig_fopen;
	zif_handler orig_file_get_contents;
	zif_handler orig_is_file;
	zif_handler orig_is_link;
	zif_handler orig_is_dir;
	zif_handler orig_opendir;
	zif_handler orig_file_exists;
	zif_handler orig_fileperms;
	zif_handler orig_fileinode;
	zif_handler orig_filesize;
	zif_handler orig_fileowner;
	zif_handler orig_filegroup;
	zif_handler orig_fileatime;
	zif_handler orig_filemtime;
	zif_handler orig_filectime;
	zif_handler orig_filetype;
	zif_handler orig_is_writable;
	zif_handler orig_is_readable;
	zif_handler orig_is_executable;
	zif_handler orig_lstat;
	zif_handler orig_readfile;
	zif_handler orig_stat;
	/* used for includes with . in them inside front controller */
	char*       cwd;
	uint32_t    cwd_len;
	int         cwd_init;
	char        *openssl_privatekey;
	uint32_t    openssl_privatekey_len;
	/* crxa_get_archive cache */
	char*       last_crxa_name;
	uint32_t    last_crxa_name_len;
	char*       last_alias;
	uint32_t    last_alias_len;
	crxa_archive_data* last_crxa;
	HashTable mime_types;
CREX_END_MODULE_GLOBALS(crxa)

CREX_EXTERN_MODULE_GLOBALS(crxa)
#define CRXA_G(v) CREX_MODULE_GLOBALS_ACCESSOR(crxa, v)

#if defined(ZTS) && defined(COMPILE_DL_CRXA)
CREX_TSRMLS_CACHE_EXTERN()
#endif

#include "crxazip.h"

typedef union _crxa_archive_object  crxa_archive_object;
typedef union _crxa_entry_object    crxa_entry_object;

/*
 * used in crxa_entry_info->fp_type to
 */
enum crxa_fp_type {
	/* regular file pointer crxa_archive_data->fp */
	CRXA_FP,
	/* uncompressed file pointer crxa_archive_data->uncompressed_fp */
	CRXA_UFP,
	/* modified file pointer crxa_entry_info->fp */
	CRXA_MOD,
	/* temporary manifest entry (file outside of the crxa mapped to a location inside the crxa)
	   this entry stores the stream to open in link (normally used for tars, but we steal it here) */
	CRXA_TMP
};

/*
 * Represents the metadata of the crxa file or a file entry within the crxa.
 * Can contain any combination of serialized data and the value as needed.
 */
typedef struct _crxa_metadata_tracker {
	/* Can be IS_UNDEF or a regular value */
	zval val;
	/* Nullable string with the serialized value, if the serialization was performed or read from a file. */
	crex_string *str;
} crxa_metadata_tracker;

/* entry for one file in a crxa file */
typedef struct _crxa_entry_info {
	/* first bytes are exactly as in file */
	uint32_t                 uncompressed_filesize;
	uint32_t                 timestamp;
	uint32_t                 compressed_filesize;
	uint32_t                 crc32;
	uint32_t                 flags;
	/* remainder */
	/* when changing compression, save old flags in case fp is NULL */
	uint32_t                 old_flags;
	crxa_metadata_tracker metadata_tracker;
	uint32_t                 filename_len;
	char                     *filename;
	enum crxa_fp_type        fp_type;
	/* offset within original crxa file of the file contents */
	crex_long                     offset_abs;
	/* offset within fp of the file contents */
	crex_long                     offset;
	/* offset within original crxa file of the file header (for zip-based/tar-based) */
	crex_long                     header_offset;
	crx_stream               *fp;
	crx_stream               *cfp;
	int                      fp_refcount;
	char                     *tmp;
	crxa_archive_data        *crxa;
	char                     *link; /* symbolic link to another file */
	char                     tar_type;
	/* position in the manifest */
	uint32_t                     manifest_pos;
	/* for stat */
	unsigned short           inode;

	uint32_t             is_crc_checked:1;
	uint32_t             is_modified:1;
	uint32_t             is_deleted:1;
	uint32_t             is_dir:1;
	/* this flag is used for mounted entries (external files mapped to location
	   inside a crxa */
	uint32_t             is_mounted:1;
	/* used when iterating */
	uint32_t             is_temp_dir:1;
	/* tar-based crxa file stuff */
	uint32_t             is_tar:1;
	/* zip-based crxa file stuff */
	uint32_t             is_zip:1;
	/* for cached crxa entries */
	uint32_t             is_persistent:1;
} crxa_entry_info;

/* information about a crxa file (the archive itself) */
struct _crxa_archive_data {
	char                     *fname;
	uint32_t                 fname_len;
	/* for crxa_detect_fname_ext, this stores the location of the file extension within fname */
	char                     *ext;
	uint32_t                 ext_len;
	char                     *alias;
	uint32_t                      alias_len;
	char                     version[12];
	size_t                   internal_file_start;
	size_t                   halt_offset;
	HashTable                manifest;
	/* hash of virtual directories, as in path/to/file.txt has path/to and path as virtual directories */
	HashTable                virtual_dirs;
	/* hash of mounted directory paths */
	HashTable                mounted_dirs;
	uint32_t                 flags;
	uint32_t                 min_timestamp;
	uint32_t                 max_timestamp;
	crx_stream               *fp;
	/* decompressed file contents are stored here */
	crx_stream               *ufp;
	int                      refcount;
	uint32_t                 sig_flags;
	uint32_t                 sig_len;
	char                     *signature;
	crxa_metadata_tracker metadata_tracker;
	uint32_t                 crxa_pos;
	/* if 1, then this alias was manually specified by the user and is not a permanent alias */
	uint32_t             is_temporary_alias:1;
	uint32_t             is_modified:1;
	uint32_t             is_writeable:1;
	uint32_t             is_brandnew:1;
	/* defer crxa creation */
	uint32_t             donotflush:1;
	/* zip-based crxa variables */
	uint32_t             is_zip:1;
	/* tar-based crxa variables */
	uint32_t             is_tar:1;
	/* CrxaData variables       */
	uint32_t             is_data:1;
	/* for cached crxa manifests */
	uint32_t             is_persistent:1;
};

typedef struct _crxa_entry_fp_info {
	enum crxa_fp_type        fp_type;
	/* offset within fp of the file contents */
	crex_long                     offset;
} crxa_entry_fp_info;

struct _crxa_entry_fp {
	crx_stream *fp;
	crx_stream *ufp;
	crxa_entry_fp_info *manifest;
};

static inline crx_stream *crxa_get_entrypfp(crxa_entry_info *entry)
{
	if (!entry->is_persistent) {
		return entry->crxa->fp;
	}
	return CRXA_G(cached_fp)[entry->crxa->crxa_pos].fp;
}

static inline crx_stream *crxa_get_entrypufp(crxa_entry_info *entry)
{
	if (!entry->is_persistent) {
		return entry->crxa->ufp;
	}
	return CRXA_G(cached_fp)[entry->crxa->crxa_pos].ufp;
}

static inline void crxa_set_entrypfp(crxa_entry_info *entry, crx_stream *fp)
{
	if (!entry->crxa->is_persistent) {
		entry->crxa->fp =  fp;
		return;
	}

	CRXA_G(cached_fp)[entry->crxa->crxa_pos].fp = fp;
}

static inline void crxa_set_entrypufp(crxa_entry_info *entry, crx_stream *fp)
{
	if (!entry->crxa->is_persistent) {
		entry->crxa->ufp =  fp;
		return;
	}

	CRXA_G(cached_fp)[entry->crxa->crxa_pos].ufp = fp;
}

static inline crx_stream *crxa_get_crxafp(crxa_archive_data *crxa)
{
	if (!crxa->is_persistent) {
		return crxa->fp;
	}
	return CRXA_G(cached_fp)[crxa->crxa_pos].fp;
}

static inline crx_stream *crxa_get_crxaufp(crxa_archive_data *crxa)
{
	if (!crxa->is_persistent) {
		return crxa->ufp;
	}
	return CRXA_G(cached_fp)[crxa->crxa_pos].ufp;
}

static inline void crxa_set_crxafp(crxa_archive_data *crxa, crx_stream *fp)
{
	if (!crxa->is_persistent) {
		crxa->fp =  fp;
		return;
	}

	CRXA_G(cached_fp)[crxa->crxa_pos].fp = fp;
}

static inline void crxa_set_crxaufp(crxa_archive_data *crxa, crx_stream *fp)
{
	if (!crxa->is_persistent) {
		crxa->ufp =  fp;
		return;
	}

	CRXA_G(cached_fp)[crxa->crxa_pos].ufp = fp;
}

static inline void crxa_set_fp_type(crxa_entry_info *entry, enum crxa_fp_type type, crex_off_t offset)
{
	crxa_entry_fp_info *data;

	if (!entry->is_persistent) {
		entry->fp_type = type;
		entry->offset = offset;
		return;
	}
	data = &(CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos]);
	data->fp_type = type;
	data->offset = offset;
}

static inline enum crxa_fp_type crxa_get_fp_type(crxa_entry_info *entry)
{
	if (!entry->is_persistent) {
		return entry->fp_type;
	}
	return CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos].fp_type;
}

static inline crex_off_t crxa_get_fp_offset(crxa_entry_info *entry)
{
	if (!entry->is_persistent) {
		return entry->offset;
	}
	if (CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos].fp_type == CRXA_FP) {
		if (!CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos].offset) {
			CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos].offset = entry->offset;
		}
	}
	return CRXA_G(cached_fp)[entry->crxa->crxa_pos].manifest[entry->manifest_pos].offset;
}

#define CRXA_MIME_CRX '\0'
#define CRXA_MIME_CRXS '\1'
#define CRXA_MIME_OTHER '\2'

typedef struct _crxa_mime_type {
	char *mime;
	uint32_t len;
	/* one of CRXA_MIME_* */
	char type;
} crxa_mime_type;

/* stream access data for one file entry in a crxa file */
typedef struct _crxa_entry_data {
	crxa_archive_data        *crxa;
	crx_stream               *fp;
	/* stream position proxy, allows multiple open streams referring to the same fp */
	crex_off_t                    position;
	/* for copies of the crxa fp, defines where 0 is */
	crex_off_t                    zero;
	uint32_t             for_write:1;
	uint32_t             is_zip:1;
	uint32_t             is_tar:1;
	crxa_entry_info          *internal_file;
} crxa_entry_data;

/* archive crx object */
union _crxa_archive_object {
	spl_filesystem_object    spl;
	crxa_archive_data        *archive;
};

/* entry crx object */
union _crxa_entry_object {
	spl_filesystem_object    spl;
	crxa_entry_info          *entry;
};

BEGIN_EXTERN_C()

#ifdef CRX_WIN32
static inline void crxa_unixify_path_separators(char *path, size_t path_len)
{
	char *s;

	/* unixify win paths */
	for (s = path; (size_t)(s - path) < path_len; ++s) {
		if (*s == '\\') {
			*s = '/';
		}
	}
}
#endif
/**
 * validate an alias, returns 1 for success, 0 for failure
 */
static inline int crxa_validate_alias(const char *alias, size_t alias_len) /* {{{ */
{
	return !(memchr(alias, '/', alias_len) || memchr(alias, '\\', alias_len) || memchr(alias, ':', alias_len) ||
		memchr(alias, ';', alias_len) || memchr(alias, '\n', alias_len) || memchr(alias, '\r', alias_len));
}
/* }}} */

static inline void crxa_set_inode(crxa_entry_info *entry) /* {{{ */
{
	char tmp[MAXPATHLEN];
	size_t tmp_len;
	size_t len1, len2;

	tmp_len = MIN(MAXPATHLEN, entry->filename_len + entry->crxa->fname_len);

	len1 = MIN(entry->crxa->fname_len, tmp_len);
	if (entry->crxa->fname) {
		memcpy(tmp, entry->crxa->fname, len1);
	}

	len2 = MIN(tmp_len - len1, entry->filename_len);
	memcpy(tmp + len1, entry->filename, len2);

	entry->inode = (unsigned short) crex_hash_func(tmp, tmp_len);
}
/* }}} */

void crxa_request_initialize(void);

void crxa_object_init(void);
void crxa_destroy_crxa_data(crxa_archive_data *crxa);

int crxa_open_entry_file(crxa_archive_data *crxa, crxa_entry_info *entry, char **error);
int crxa_postprocess_file(crxa_entry_data *idata, uint32_t crc32, char **error, int process_zip);
int crxa_open_from_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_open_or_create_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_create_or_parse_filename(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_open_executed_filename(char *alias, size_t alias_len, char **error);
int crxa_free_alias(crxa_archive_data *crxa, char *alias, size_t alias_len);
int crxa_get_archive(crxa_archive_data **archive, char *fname, size_t fname_len, char *alias, size_t alias_len, char **error);
int crxa_open_parsed_crxa(char *fname, size_t fname_len, char *alias, size_t alias_len, bool is_data, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_verify_signature(crx_stream *fp, size_t end_of_crxa, uint32_t sig_type, char *sig, size_t sig_len, char *fname, char **signature, size_t *signature_len, char **error);
int crxa_create_signature(crxa_archive_data *crxa, crx_stream *fp, char **signature, size_t *signature_length, char **error);

/* utility functions */
crex_string *crxa_create_default_stub(const char *index_crx, const char *web_index, char **error);
char *crxa_decompress_filter(crxa_entry_info * entry, int return_unknown);
char *crxa_compress_filter(crxa_entry_info * entry, int return_unknown);

/* void crxa_remove_virtual_dirs(crxa_archive_data *crxa, char *filename, size_t filename_len); */
void crxa_add_virtual_dirs(crxa_archive_data *crxa, char *filename, size_t filename_len);
int crxa_mount_entry(crxa_archive_data *crxa, char *filename, size_t filename_len, char *path, size_t path_len);
crex_string *crxa_find_in_include_path(crex_string *file, crxa_archive_data **pcrxa);
char *crxa_fix_filepath(char *path, size_t *new_len, int use_cwd);
crxa_entry_info * crxa_open_jit(crxa_archive_data *crxa, crxa_entry_info *entry, char **error);
void crxa_parse_metadata_lazy(const char *buffer, crxa_metadata_tracker *tracker, uint32_t zip_metadata_len, int persistent);
bool crxa_metadata_tracker_has_data(const crxa_metadata_tracker* tracker, int persistent);
/* If this has data, free it and set all values to undefined. */
void crxa_metadata_tracker_free(crxa_metadata_tracker* val, int persistent);
void crxa_metadata_tracker_copy(crxa_metadata_tracker* dest, const crxa_metadata_tracker *source, int persistent);
void crxa_metadata_tracker_clone(crxa_metadata_tracker* tracker);
void crxa_metadata_tracker_try_ensure_has_serialized_data(crxa_metadata_tracker* tracker, int persistent);
int crxa_metadata_tracker_unserialize_or_copy(crxa_metadata_tracker* tracker, zval *value, int persistent, HashTable *unserialize_options, const char* method_name);
void crxa_release_entry_metadata(crxa_entry_info *entry);
void crxa_release_archive_metadata(crxa_archive_data *crxa);
void destroy_crxa_manifest_entry(zval *zv);
int crxa_seek_efp(crxa_entry_info *entry, crex_off_t offset, int whence, crex_off_t position, int follow_links);
crx_stream *crxa_get_efp(crxa_entry_info *entry, int follow_links);
int crxa_copy_entry_fp(crxa_entry_info *source, crxa_entry_info *dest, char **error);
int crxa_open_entry_fp(crxa_entry_info *entry, char **error, int follow_links);
crxa_entry_info *crxa_get_link_source(crxa_entry_info *entry);
int crxa_create_writeable_entry(crxa_archive_data *crxa, crxa_entry_info *entry, char **error);
int crxa_separate_entry_fp(crxa_entry_info *entry, char **error);
int crxa_open_archive_fp(crxa_archive_data *crxa);
int crxa_copy_on_write(crxa_archive_data **pcrxa);

/* tar functions in tar.c */
int crxa_is_tar(char *buf, char *fname);
int crxa_parse_tarfile(crx_stream* fp, char *fname, size_t fname_len, char *alias, size_t alias_len, crxa_archive_data** pcrxa, int is_data, uint32_t compression, char **error);
int crxa_open_or_create_tar(char *fname, size_t fname_len, char *alias, size_t alias_len, int is_data, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_tar_flush(crxa_archive_data *crxa, char *user_stub, crex_long len, int defaultstub, char **error);

/* zip functions in zip.c */
int crxa_parse_zipfile(crx_stream *fp, char *fname, size_t fname_len, char *alias, size_t alias_len, crxa_archive_data** pcrxa, char **error);
int crxa_open_or_create_zip(char *fname, size_t fname_len, char *alias, size_t alias_len, int is_data, uint32_t options, crxa_archive_data** pcrxa, char **error);
int crxa_zip_flush(crxa_archive_data *archive, char *user_stub, crex_long len, int defaultstub, char **error);

#ifdef CRXA_MAIN
static int crxa_open_from_fp(crx_stream* fp, char *fname, size_t fname_len, char *alias, size_t alias_len, uint32_t options, crxa_archive_data** pcrxa, int is_data, char **error);
extern const crx_stream_wrapper crx_stream_crxa_wrapper;
#else
extern HashTable cached_crxas;
extern HashTable cached_alias;
#endif

int crxa_archive_delref(crxa_archive_data *crxa);
int crxa_entry_delref(crxa_entry_data *idata);

crxa_entry_info *crxa_get_entry_info(crxa_archive_data *crxa, char *path, size_t path_len, char **error, int security);
crxa_entry_info *crxa_get_entry_info_dir(crxa_archive_data *crxa, char *path, size_t path_len, char dir, char **error, int security);
crxa_entry_data *crxa_get_or_create_entry_data(char *fname, size_t fname_len, char *path, size_t path_len, const char *mode, char allow_dir, char **error, int security);
int crxa_get_entry_data(crxa_entry_data **ret, char *fname, size_t fname_len, char *path, size_t path_len, const char *mode, char allow_dir, char **error, int security);
int crxa_flush(crxa_archive_data *archive, char *user_stub, crex_long len, int convert, char **error);
int crxa_detect_crxa_fname_ext(const char *filename, size_t filename_len, const char **ext_str, size_t *ext_len, int executable, int for_create, int is_complete);
int crxa_split_fname(const char *filename, size_t filename_len, char **arch, size_t *arch_len, char **entry, size_t *entry_len, int executable, int for_create);

typedef enum {
	pcr_use_query,
	pcr_is_ok,
	pcr_err_double_slash,
	pcr_err_up_dir,
	pcr_err_curr_dir,
	pcr_err_back_slash,
	pcr_err_star,
	pcr_err_illegal_char,
	pcr_err_empty_entry
} crxa_path_check_result;

crxa_path_check_result crxa_path_check(char **p, size_t *len, const char **error);

END_EXTERN_C()
