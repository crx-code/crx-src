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
   | Authors: Rasmus Lerdorf <rasmus@crx.net>                             |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Colin Viebrock <colin@viebrock.ca>                          |
   +----------------------------------------------------------------------+
*/

#include "crx.h"
#include "crx_ini.h"
#include "crx_globals.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "info.h"
#include "credits.h"
#include "css.h"
#include "SAPI.h"
#include <time.h>
#include "crx_main.h"
#include "crex_globals.h"		/* needs ELS */
#include "crex_extensions.h"
#include "crex_highlight.h"
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#include "url.h"

#ifdef CRX_WIN32
# include "winver.h"
#endif

#define SECTION(name)	if (!sapi_module.crxinfo_as_text) { \
							crx_info_print("<h2>" name "</h2>\n"); \
						} else { \
							crx_info_print_table_start(); \
							crx_info_print_table_header(1, name); \
							crx_info_print_table_end(); \
						} \

CRXAPI extern char *crx_ini_opened_path;
CRXAPI extern char *crx_ini_scanned_path;
CRXAPI extern char *crx_ini_scanned_files;

static CREX_COLD int crx_info_print_html_esc(const char *str, size_t len) /* {{{ */
{
	size_t written;
	crex_string *new_str;

	new_str = crx_escape_html_entities((const unsigned char *) str, len, 0, ENT_QUOTES, "utf-8");
	written = crx_output_write(ZSTR_VAL(new_str), ZSTR_LEN(new_str));
	crex_string_free(new_str);
	return written;
}
/* }}} */

static CREX_COLD int crx_info_printf(const char *fmt, ...) /* {{{ */
{
	char *buf;
	size_t len, written;
	va_list argv;

	va_start(argv, fmt);
	len = vspprintf(&buf, 0, fmt, argv);
	va_end(argv);

	written = crx_output_write(buf, len);
	efree(buf);
	return written;
}
/* }}} */

static crex_always_inline int crx_info_print(const char *str) /* {{{ */
{
	return crx_output_write(str, strlen(str));
}
/* }}} */

static CREX_COLD void crx_info_print_stream_hash(const char *name, HashTable *ht) /* {{{ */
{
	crex_string *key;

	if (ht) {
		if (crex_hash_num_elements(ht)) {
			int first = 1;

			if (!sapi_module.crxinfo_as_text) {
				crx_info_printf("<tr><td class=\"e\">Registered %s</td><td class=\"v\">", name);
			} else {
				crx_info_printf("\nRegistered %s => ", name);
			}

			if (!HT_IS_PACKED(ht)) {
				CREX_HASH_MAP_FOREACH_STR_KEY(ht, key) {
					if (key) {
						if (first) {
							first = 0;
						} else {
							crx_info_print(", ");
						}
						if (!sapi_module.crxinfo_as_text) {
							crx_info_print_html_esc(ZSTR_VAL(key), ZSTR_LEN(key));
						} else {
							crx_info_print(ZSTR_VAL(key));
						}
					}
				} CREX_HASH_FOREACH_END();
			}

			if (!sapi_module.crxinfo_as_text) {
				crx_info_print("</td></tr>\n");
			}
		} else {
			char reg_name[128];
			snprintf(reg_name, sizeof(reg_name), "Registered %s", name);
			crx_info_print_table_row(2, reg_name, "none registered");
		}
	} else {
		crx_info_print_table_row(2, name, "disabled");
	}
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_module(crex_module_entry *crex_module) /* {{{ */
{
	if (crex_module->info_func || crex_module->version) {
		if (!sapi_module.crxinfo_as_text) {
			crex_string *url_name = crx_url_encode(crex_module->name, strlen(crex_module->name));

			crex_str_tolower(ZSTR_VAL(url_name), ZSTR_LEN(url_name));
			crx_info_printf("<h2><a name=\"module_%s\" href=\"#module_%s\">%s</a></h2>\n", ZSTR_VAL(url_name), ZSTR_VAL(url_name), crex_module->name);

			efree(url_name);
		} else {
			crx_info_print_table_start();
			crx_info_print_table_header(1, crex_module->name);
			crx_info_print_table_end();
		}
		if (crex_module->info_func) {
			crex_module->info_func(crex_module);
		} else {
			crx_info_print_table_start();
			crx_info_print_table_row(2, "Version", crex_module->version);
			crx_info_print_table_end();
			DISPLAY_INI_ENTRIES();
		}
	} else {
		if (!sapi_module.crxinfo_as_text) {
			crx_info_printf("<tr><td class=\"v\">%s</td></tr>\n", crex_module->name);
		} else {
			crx_info_printf("%s\n", crex_module->name);
		}
	}
}
/* }}} */

/* {{{ crx_print_gpcse_array */
static CREX_COLD void crx_print_gpcse_array(char *name, uint32_t name_length)
{
	zval *data, *tmp;
	crex_string *string_key;
	crex_ulong num_key;
	crex_string *key;

	key = crex_string_init(name, name_length, 0);
	crex_is_auto_global(key);

	if ((data = crex_hash_find_deref(&EG(symbol_table), key)) != NULL && (C_TYPE_P(data) == IS_ARRAY)) {
		CREX_HASH_FOREACH_KEY_VAL(C_ARRVAL_P(data), num_key, string_key, tmp) {
			if (!sapi_module.crxinfo_as_text) {
				crx_info_print("<tr>");
				crx_info_print("<td class=\"e\">");
			}

			crx_info_print("$");
			crx_info_print(name);
			crx_info_print("['");

			if (string_key != NULL) {
				if (!sapi_module.crxinfo_as_text) {
					crx_info_print_html_esc(ZSTR_VAL(string_key), ZSTR_LEN(string_key));
				} else {
					crx_info_print(ZSTR_VAL(string_key));
				}
			} else {
				crx_info_printf(CREX_ULONG_FMT, num_key);
			}
			crx_info_print("']");
			if (!sapi_module.crxinfo_as_text) {
				crx_info_print("</td><td class=\"v\">");
			} else {
				crx_info_print(" => ");
			}
			ZVAL_DEREF(tmp);
			if (C_TYPE_P(tmp) == IS_ARRAY) {
				if (!sapi_module.crxinfo_as_text) {
					crex_string *str = crex_print_zval_r_to_str(tmp, 0);
					crx_info_print("<pre>");
					crx_info_print_html_esc(ZSTR_VAL(str), ZSTR_LEN(str));
					crx_info_print("</pre>");
					crex_string_release_ex(str, 0);
				} else {
					crex_print_zval_r(tmp, 0);
				}
			} else {
				crex_string *tmp2;
				crex_string *str = zval_get_tmp_string(tmp, &tmp2);

				if (!sapi_module.crxinfo_as_text) {
					if (ZSTR_LEN(str) == 0) {
						crx_info_print("<i>no value</i>");
					} else {
						crx_info_print_html_esc(ZSTR_VAL(str), ZSTR_LEN(str));
					}
				} else {
					crx_info_print(ZSTR_VAL(str));
				}

				crex_tmp_string_release(tmp2);
			}
			if (!sapi_module.crxinfo_as_text) {
				crx_info_print("</td></tr>\n");
			} else {
				crx_info_print("\n");
			}
		} CREX_HASH_FOREACH_END();
	}
	crex_string_efree(key);
}
/* }}} */

/* {{{ crx_info_print_style */
CRXAPI CREX_COLD void CREX_COLD crx_info_print_style(void)
{
	crx_info_printf("<style type=\"text/css\">\n");
	crx_info_print_css();
	crx_info_printf("</style>\n");
}
/* }}} */

/* {{{ crx_info_html_esc */
CRXAPI CREX_COLD crex_string *crx_info_html_esc(const char *string)
{
	return crx_escape_html_entities((const unsigned char *) string, strlen(string), 0, ENT_QUOTES, NULL);
}
/* }}} */

#ifdef CRX_WIN32
/* {{{  */

char* crx_get_windows_name()
{
	OSVERSIONINFOEX osvi = EG(windows_version_info);
	SYSTEM_INFO si;
	DWORD dwType;
	char *major = NULL, *sub = NULL, *retval;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));

	GetNativeSystemInfo(&si);

	if (VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion >= 10) {
		if (osvi.dwMajorVersion == 10) {
			if (osvi.dwMinorVersion == 0) {
				if (osvi.wProductType == VER_NT_WORKSTATION) {
					if (osvi.dwBuildNumber >= 22000) {
						major = "Windows 11";
					} else {
						major = "Windows 10";
					}
				} else {
					if (osvi.dwBuildNumber >= 20348) {
						major = "Windows Server 2022";
					} else if (osvi.dwBuildNumber >= 19042) {
						major = "Windows Server, version 20H2";
					} else if (osvi.dwBuildNumber >= 19041) {
						major = "Windows Server, version 2004";
					} else if (osvi.dwBuildNumber >= 18363) {
						major = "Windows Server, version 1909";
					} else if (osvi.dwBuildNumber >= 18362) {
						major = "Windows Server, version 1903";
					} else if (osvi.dwBuildNumber >= 17763) {
						// could also be Windows Server, version 1809, but there's no easy way to tell
						major = "Windows Server 2019";
					} else if (osvi.dwBuildNumber >= 17134) {
						major = "Windows Server, version 1803";
					} else if (osvi.dwBuildNumber >= 16299) {
						major = "Windows Server, version 1709";
					} else {
						major = "Windows Server 2016";
					}
				}
			}
		}
	} else if (VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion >= 6) {
		if (osvi.dwMajorVersion == 6) {
			if( osvi.dwMinorVersion == 0 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION ) {
					major = "Windows Vista";
				} else {
					major = "Windows Server 2008";
				}
			} else if ( osvi.dwMinorVersion == 1 ) {
				if( osvi.wProductType == VER_NT_WORKSTATION )  {
					major = "Windows 7";
				} else {
					major = "Windows Server 2008 R2";
				}
			} else if ( osvi.dwMinorVersion == 2 ) {
				/* could be Windows 8/Windows Server 2012, could be Windows 8.1/Windows Server 2012 R2 */
				/* XXX and one more X - the above comment is true if no manifest is used for two cases:
					- if the CRX build doesn't use the correct manifest
					- if CRX DLL loaded under some binary that doesn't use the correct manifest

					So keep the handling here as is for now, even if we know 6.2 is win8 and nothing else, and think about an improvement. */
				OSVERSIONINFOEX osvi81;
				DWORDLONG dwlConditionMask = 0;
				int op = VER_GREATER_EQUAL;

				ZeroMemory(&osvi81, sizeof(OSVERSIONINFOEX));
				osvi81.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				osvi81.dwMajorVersion = 6;
				osvi81.dwMinorVersion = 3;
				osvi81.wServicePackMajor = 0;

				VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
				VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
				VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, op);

				if (VerifyVersionInfo(&osvi81,
					VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
					dwlConditionMask)) {
					osvi.dwMinorVersion = 3; /* Windows 8.1/Windows Server 2012 R2 */
					if( osvi.wProductType == VER_NT_WORKSTATION )  {
						major = "Windows 8.1";
					} else {
						major = "Windows Server 2012 R2";
					}
				} else {
					if( osvi.wProductType == VER_NT_WORKSTATION )  {
						major = "Windows 8";
					} else {
						major = "Windows Server 2012";
					}
				}
			} else if (osvi.dwMinorVersion == 3) {
				if( osvi.wProductType == VER_NT_WORKSTATION )  {
					major = "Windows 8.1";
				} else {
					major = "Windows Server 2012 R2";
				}
			} else {
				major = "Unknown Windows version";
			}

			/* No return value check, as it can only fail if the input parameters are broken (which we manually supply) */
			GetProductInfo(6, 0, 0, 0, &dwType);

			switch (dwType) {
				case PRODUCT_ULTIMATE:
					sub = "Ultimate Edition";
					break;
				case PRODUCT_HOME_BASIC:
					sub = "Home Basic Edition";
					break;
				case PRODUCT_HOME_PREMIUM:
					sub = "Home Premium Edition";
					break;
				case PRODUCT_ENTERPRISE:
					sub = "Enterprise Edition";
					break;
				case PRODUCT_HOME_BASIC_N:
					sub = "Home Basic N Edition";
					break;
				case PRODUCT_BUSINESS:
					if ((osvi.dwMajorVersion > 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion > 0)) {
						sub = "Professional Edition";
					} else {
						sub = "Business Edition";
					}
					break;
				case PRODUCT_STANDARD_SERVER:
					sub = "Standard Edition";
					break;
				case PRODUCT_DATACENTER_SERVER:
					sub = "Datacenter Edition";
					break;
				case PRODUCT_SMALLBUSINESS_SERVER:
					sub = "Small Business Server";
					break;
				case PRODUCT_ENTERPRISE_SERVER:
					sub = "Enterprise Edition";
					break;
				case PRODUCT_STARTER:
					if ((osvi.dwMajorVersion > 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion > 0)) {
						sub = "Starter N Edition";
					} else {
					    sub = "Starter Edition";
					}
					break;
				case PRODUCT_DATACENTER_SERVER_CORE:
					sub = "Datacenter Edition (core installation)";
					break;
				case PRODUCT_STANDARD_SERVER_CORE:
					sub = "Standard Edition (core installation)";
					break;
				case PRODUCT_ENTERPRISE_SERVER_CORE:
					sub = "Enterprise Edition (core installation)";
					break;
				case PRODUCT_ENTERPRISE_SERVER_IA64:
					sub = "Enterprise Edition for Itanium-based Systems";
					break;
				case PRODUCT_BUSINESS_N:
					if ((osvi.dwMajorVersion > 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion > 0)) {
						sub = "Professional N Edition";
					} else {
						sub = "Business N Edition";
					}
					break;
				case PRODUCT_WEB_SERVER:
					sub = "Web Server Edition";
					break;
				case PRODUCT_CLUSTER_SERVER:
					sub = "HPC Edition";
					break;
				case PRODUCT_HOME_SERVER:
					sub = "Storage Server Essentials Edition";
					break;
				case PRODUCT_STORAGE_EXPRESS_SERVER:
					sub = "Storage Server Express Edition";
					break;
				case PRODUCT_STORAGE_STANDARD_SERVER:
					sub = "Storage Server Standard Edition";
					break;
				case PRODUCT_STORAGE_WORKGROUP_SERVER:
					sub = "Storage Server Workgroup Edition";
					break;
				case PRODUCT_STORAGE_ENTERPRISE_SERVER:
					sub = "Storage Server Enterprise Edition";
					break;
				case PRODUCT_SERVER_FOR_SMALLBUSINESS:
					sub = "Essential Server Solutions Edition";
					break;
				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
					sub = "Small Business Server Premium Edition";
					break;
				case PRODUCT_HOME_PREMIUM_N:
					sub = "Home Premium N Edition";
					break;
				case PRODUCT_ENTERPRISE_N:
					sub = "Enterprise N Edition";
					break;
				case PRODUCT_ULTIMATE_N:
					sub = "Ultimate N Edition";
					break;
				case PRODUCT_WEB_SERVER_CORE:
					sub = "Web Server Edition (core installation)";
					break;
				case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
					sub = "Essential Business Server Management Server Edition";
					break;
				case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
					sub = "Essential Business Server Management Security Edition";
					break;
				case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
					sub = "Essential Business Server Management Messaging Edition";
					break;
				case PRODUCT_SERVER_FOUNDATION:
					sub = "Foundation Edition";
					break;
				case PRODUCT_HOME_PREMIUM_SERVER:
					sub = "Home Server 2011 Edition";
					break;
				case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
					sub = "Essential Server Solutions Edition (without Hyper-V)";
					break;
				case PRODUCT_STANDARD_SERVER_V:
					sub = "Standard Edition (without Hyper-V)";
					break;
				case PRODUCT_DATACENTER_SERVER_V:
					sub = "Datacenter Edition (without Hyper-V)";
					break;
				case PRODUCT_ENTERPRISE_SERVER_V:
					sub = "Enterprise Edition (without Hyper-V)";
					break;
				case PRODUCT_DATACENTER_SERVER_CORE_V:
					sub = "Datacenter Edition (core installation, without Hyper-V)";
					break;
				case PRODUCT_STANDARD_SERVER_CORE_V:
					sub = "Standard Edition (core installation, without Hyper-V)";
					break;
				case PRODUCT_ENTERPRISE_SERVER_CORE_V:
					sub = "Enterprise Edition (core installation, without Hyper-V)";
					break;
				case PRODUCT_HYPERV:
					sub = "Hyper-V Server";
					break;
				case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
					sub = "Storage Server Express Edition (core installation)";
					break;
				case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
					sub = "Storage Server Standard Edition (core installation)";
					break;
				case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
					sub = "Storage Server Workgroup Edition (core installation)";
					break;
				case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
					sub = "Storage Server Enterprise Edition (core installation)";
					break;
				case PRODUCT_STARTER_N:
					sub = "Starter N Edition";
					break;
				case PRODUCT_PROFESSIONAL:
					sub = "Professional Edition";
					break;
				case PRODUCT_PROFESSIONAL_N:
					sub = "Professional N Edition";
					break;
				case PRODUCT_SB_SOLUTION_SERVER:
					sub = "Small Business Server 2011 Essentials Edition";
					break;
				case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
					sub = "Server For SB Solutions Edition";
					break;
				case PRODUCT_STANDARD_SERVER_SOLUTIONS:
					sub = "Solutions Premium Edition";
					break;
				case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
					sub = "Solutions Premium Edition (core installation)";
					break;
				case PRODUCT_SB_SOLUTION_SERVER_EM:
					sub = "Server For SB Solutions EM Edition";
					break;
				case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
					sub = "Server For SB Solutions EM Edition";
					break;
				case PRODUCT_SOLUTION_EMBEDDEDSERVER:
					sub = "MultiPoint Server Edition";
					break;
				case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
					sub = "Essential Server Solution Management Edition";
					break;
				case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
					sub = "Essential Server Solution Additional Edition";
					break;
				case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
					sub = "Essential Server Solution Management SVC Edition";
					break;
				case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
					sub = "Essential Server Solution Additional SVC Edition";
					break;
				case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
					sub = "Small Business Server Premium Edition (core installation)";
					break;
				case PRODUCT_CLUSTER_SERVER_V:
					sub = "Hyper Core V Edition";
					break;
				case PRODUCT_STARTER_E:
					sub = "Hyper Core V Edition";
					break;
				case PRODUCT_ENTERPRISE_EVALUATION:
					sub = "Enterprise Edition (evaluation installation)";
					break;
				case PRODUCT_MULTIPOINT_STANDARD_SERVER:
					sub = "MultiPoint Server Standard Edition (full installation)";
					break;
				case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
					sub = "MultiPoint Server Premium Edition (full installation)";
					break;
				case PRODUCT_STANDARD_EVALUATION_SERVER:
					sub = "Standard Edition (evaluation installation)";
					break;
				case PRODUCT_DATACENTER_EVALUATION_SERVER:
					sub = "Datacenter Edition (evaluation installation)";
					break;
				case PRODUCT_ENTERPRISE_N_EVALUATION:
					sub = "Enterprise N Edition (evaluation installation)";
					break;
				case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
					sub = "Storage Server Workgroup Edition (evaluation installation)";
					break;
				case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
					sub = "Storage Server Standard Edition (evaluation installation)";
					break;
				case PRODUCT_CORE_N:
					sub = "Windows 8 N Edition";
					break;
				case PRODUCT_CORE_COUNTRYSPECIFIC:
					sub = "Windows 8 China Edition";
					break;
				case PRODUCT_CORE_SINGLELANGUAGE:
					sub = "Windows 8 Single Language Edition";
					break;
				case PRODUCT_CORE:
					sub = "Windows 8 Edition";
					break;
				case PRODUCT_PROFESSIONAL_WMC:
					sub = "Professional with Media Center Edition";
					break;
			}
		}
	} else {
		return NULL;
	}

	spprintf(&retval, 0, "%s%s%s%s%s", major, sub?" ":"", sub?sub:"", osvi.szCSDVersion[0] != '\0'?" ":"", osvi.szCSDVersion);
	return retval;
}
/* }}}  */

/* {{{  */
void crx_get_windows_cpu(char *buf, int bufsize)
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	switch (SysInfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_INTEL :
			snprintf(buf, bufsize, "i%d", SysInfo.dwProcessorType);
			break;
		case PROCESSOR_ARCHITECTURE_MIPS :
			snprintf(buf, bufsize, "MIPS R%d000", SysInfo.wProcessorLevel);
			break;
		case PROCESSOR_ARCHITECTURE_ALPHA :
			snprintf(buf, bufsize, "Alpha %d", SysInfo.wProcessorLevel);
			break;
		case PROCESSOR_ARCHITECTURE_PPC :
			snprintf(buf, bufsize, "PPC 6%02d", SysInfo.wProcessorLevel);
			break;
		case PROCESSOR_ARCHITECTURE_IA64 :
			snprintf(buf, bufsize,  "IA64");
			break;
#if defined(PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)
		case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 :
			snprintf(buf, bufsize, "IA32");
			break;
#endif
#if defined(PROCESSOR_ARCHITECTURE_AMD64)
		case PROCESSOR_ARCHITECTURE_AMD64 :
			snprintf(buf, bufsize, "AMD64");
			break;
#endif
#if defined(PROCESSOR_ARCHITECTURE_ARM64)
		case PROCESSOR_ARCHITECTURE_ARM64 :
			snprintf(buf, bufsize, "ARM64");
			break;
#endif
		case PROCESSOR_ARCHITECTURE_UNKNOWN :
		default:
			snprintf(buf, bufsize, "Unknown");
			break;
	}
}
/* }}}  */
#endif

/* {{{ crx_get_uname */
CRXAPI crex_string *crx_get_uname(char mode)
{
	char *crx_uname;
	char tmp_uname[256];
#ifdef CRX_WIN32
	DWORD dwBuild=0;
	DWORD dwVersion = GetVersion();
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
	char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];

	GetComputerName(ComputerName, &dwSize);

	if (mode == 's') {
		crx_uname = "Windows NT";
	} else if (mode == 'r') {
		snprintf(tmp_uname, sizeof(tmp_uname), "%d.%d", dwWindowsMajorVersion, dwWindowsMinorVersion);
		crx_uname = tmp_uname;
	} else if (mode == 'n') {
		crx_uname = ComputerName;
	} else if (mode == 'v') {
		char *winver = crx_get_windows_name();
		dwBuild = (DWORD)(HIWORD(dwVersion));
		if(winver == NULL) {
			snprintf(tmp_uname, sizeof(tmp_uname), "build %d", dwBuild);
		} else {
			snprintf(tmp_uname, sizeof(tmp_uname), "build %d (%s)", dwBuild, winver);
		}
		crx_uname = tmp_uname;
		if(winver) {
			efree(winver);
		}
	} else if (mode == 'm') {
		crx_get_windows_cpu(tmp_uname, sizeof(tmp_uname));
		crx_uname = tmp_uname;
	} else { /* assume mode == 'a' */
		char *winver = crx_get_windows_name();
		char wincpu[20];

		CREX_ASSERT(winver != NULL);

		crx_get_windows_cpu(wincpu, sizeof(wincpu));
		dwBuild = (DWORD)(HIWORD(dwVersion));

		/* Windows "version" 6.2 could be Windows 8/Windows Server 2012, but also Windows 8.1/Windows Server 2012 R2 */
		if (dwWindowsMajorVersion == 6 && dwWindowsMinorVersion == 2) {
			if (strncmp(winver, "Windows 8.1", 11) == 0 || strncmp(winver, "Windows Server 2012 R2", 22) == 0) {
				dwWindowsMinorVersion = 3;
			}
		}

		snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %d.%d build %d (%s) %s",
				 "Windows NT", ComputerName,
				 dwWindowsMajorVersion, dwWindowsMinorVersion, dwBuild, winver?winver:"unknown", wincpu);
		if(winver) {
			efree(winver);
		}
		crx_uname = tmp_uname;
	}
#else
#ifdef HAVE_SYS_UTSNAME_H
	struct utsname buf;
	if (uname((struct utsname *)&buf) == -1) {
		crx_uname = CRX_UNAME;
	} else {
		if (mode == 's') {
			crx_uname = buf.sysname;
		} else if (mode == 'r') {
			crx_uname = buf.release;
		} else if (mode == 'n') {
			crx_uname = buf.nodename;
		} else if (mode == 'v') {
			crx_uname = buf.version;
		} else if (mode == 'm') {
			crx_uname = buf.machine;
		} else { /* assume mode == 'a' */
			snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %s %s %s",
					 buf.sysname, buf.nodename, buf.release, buf.version,
					 buf.machine);
			crx_uname = tmp_uname;
		}
	}
#else
	crx_uname = CRX_UNAME;
#endif
#endif
	return crex_string_init(crx_uname, strlen(crx_uname), 0);
}
/* }}} */

/* {{{ crx_print_info_htmlhead */
CRXAPI CREX_COLD void crx_print_info_htmlhead(void)
{
	crx_info_print("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-transitional.dtd\">\n");
	crx_info_print("<html xmlns=\"http://www.w3.org/1999/xhtml\">");
	crx_info_print("<head>\n");
	crx_info_print_style();
	crx_info_printf("<title>CRX %s - crxinfo()</title>", CRX_VERSION);
	crx_info_print("<meta name=\"ROBOTS\" content=\"NOINDEX,NOFOLLOW,NOARCHIVE\" />");
	crx_info_print("</head>\n");
	crx_info_print("<body><div class=\"center\">\n");
}
/* }}} */

/* {{{ module_name_cmp */
static int module_name_cmp(Bucket *f, Bucket *s)
{
	return strcasecmp(((crex_module_entry *)C_PTR(f->val))->name,
				  ((crex_module_entry *)C_PTR(s->val))->name);
}
/* }}} */

/* {{{ crx_print_info */
CRXAPI CREX_COLD void crx_print_info(int flag)
{
	char **env, *tmp1, *tmp2;
	crex_string *crx_uname;

	if (!sapi_module.crxinfo_as_text) {
		crx_print_info_htmlhead();
	} else {
		crx_info_print("crxinfo()\n");
	}

	if (flag & CRX_INFO_GENERAL) {
		const char *crex_version = get_crex_version();
		char temp_api[10];

		crx_uname = crx_get_uname('a');

		if (!sapi_module.crxinfo_as_text) {
			crx_info_print_box_start(1);
		}

		if (!sapi_module.crxinfo_as_text) {
	        time_t the_time;
	        struct tm *ta, tmbuf;

	        the_time = time(NULL);
	        ta = crx_localtime_r(&the_time, &tmbuf);

			crx_info_print("<a href=\"http://www.crx.net/\"><img border=\"0\" src=\"");
	        if (ta && (ta->tm_mon==3) && (ta->tm_mday==1)) {
		        crx_info_print(CRX_EGG_LOGO_DATA_URI "\" alt=\"CRX logo\" /></a>");
	        } else {
		        crx_info_print(CRX_LOGO_DATA_URI "\" alt=\"CRX logo\" /></a>");
			}
		}

		if (!sapi_module.crxinfo_as_text) {
			crx_info_printf("<h1 class=\"p\">CRX Version %s</h1>\n", CRX_VERSION);
		} else {
			crx_info_print_table_row(2, "CRX Version", CRX_VERSION);
		}
		crx_info_print_box_end();
		crx_info_print_table_start();
		crx_info_print_table_row(2, "System", ZSTR_VAL(crx_uname));
		crx_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__);
#ifdef CRX_BUILD_SYSTEM
		crx_info_print_table_row(2, "Build System", CRX_BUILD_SYSTEM);
#endif
#ifdef CRX_BUILD_PROVIDER
		crx_info_print_table_row(2, "Build Provider", CRX_BUILD_PROVIDER);
#endif
#ifdef CRX_BUILD_COMPILER
		crx_info_print_table_row(2, "Compiler", CRX_BUILD_COMPILER);
#endif
#ifdef CRX_BUILD_ARCH
		crx_info_print_table_row(2, "Architecture", CRX_BUILD_ARCH);
#endif
#ifdef CONFIGURE_COMMAND
		crx_info_print_table_row(2, "Configure Command", CONFIGURE_COMMAND );
#endif

		if (sapi_module.pretty_name) {
			crx_info_print_table_row(2, "Server API", sapi_module.pretty_name );
		}

#ifdef VIRTUAL_DIR
		crx_info_print_table_row(2, "Virtual Directory Support", "enabled" );
#else
		crx_info_print_table_row(2, "Virtual Directory Support", "disabled" );
#endif

		crx_info_print_table_row(2, "Configuration File (crx.ini) Path", CRX_CONFIG_FILE_PATH);
		crx_info_print_table_row(2, "Loaded Configuration File", crx_ini_opened_path ? crx_ini_opened_path : "(none)");
		crx_info_print_table_row(2, "Scan this dir for additional .ini files", crx_ini_scanned_path ? crx_ini_scanned_path : "(none)");
		crx_info_print_table_row(2, "Additional .ini files parsed", crx_ini_scanned_files ? crx_ini_scanned_files : "(none)");

		snprintf(temp_api, sizeof(temp_api), "%d", CRX_API_VERSION);
		crx_info_print_table_row(2, "CRX API", temp_api);

		snprintf(temp_api, sizeof(temp_api), "%d", CREX_MODULE_API_NO);
		crx_info_print_table_row(2, "CRX Extension", temp_api);

		snprintf(temp_api, sizeof(temp_api), "%d", CREX_EXTENSION_API_NO);
		crx_info_print_table_row(2, "Crex Extension", temp_api);

		crx_info_print_table_row(2, "Crex Extension Build", CREX_EXTENSION_BUILD_ID);
		crx_info_print_table_row(2, "CRX Extension Build", CREX_MODULE_BUILD_ID);

#if CREX_DEBUG
		crx_info_print_table_row(2, "Debug Build", "yes" );
#else
		crx_info_print_table_row(2, "Debug Build", "no" );
#endif

#ifdef ZTS
		crx_info_print_table_row(2, "Thread Safety", "enabled" );
		crx_info_print_table_row(2, "Thread API", tsrm_api_name() );
#else
		crx_info_print_table_row(2, "Thread Safety", "disabled" );
#endif

#ifdef CREX_SIGNALS
		crx_info_print_table_row(2, "Crex Signal Handling", "enabled" );
#else
		crx_info_print_table_row(2, "Crex Signal Handling", "disabled" );
#endif

		crx_info_print_table_row(2, "Crex Memory Manager", is_crex_mm() ? "enabled" : "disabled" );

		{
			const crex_multibyte_functions *functions = crex_multibyte_get_functions();
			char *descr;
			if (functions) {
				spprintf(&descr, 0, "provided by %s", functions->provider_name);
			} else {
				descr = estrdup("disabled");
			}
			crx_info_print_table_row(2, "Crex Multibyte Support", descr);
			efree(descr);
		}

#ifdef CREX_MAX_EXECUTION_TIMERS
		crx_info_print_table_row(2, "Crex Max Execution Timers", "enabled" );
#else
		crx_info_print_table_row(2, "Crex Max Execution Timers", "disabled" );
#endif

#ifdef HAVE_IPV6
		crx_info_print_table_row(2, "IPv6 Support", "enabled" );
#else
		crx_info_print_table_row(2, "IPv6 Support", "disabled" );
#endif

#ifdef HAVE_DTRACE
		crx_info_print_table_row(2, "DTrace Support", (crex_dtrace_enabled ? "enabled" : "available, disabled"));
#else
		crx_info_print_table_row(2, "DTrace Support", "disabled" );
#endif

		crx_info_print_stream_hash("CRX Streams",  crx_stream_get_url_stream_wrappers_hash());
		crx_info_print_stream_hash("Stream Socket Transports", crx_stream_xport_get_hash());
		crx_info_print_stream_hash("Stream Filters", crx_get_stream_filters_hash());

		crx_info_print_table_end();

		/* Crex Engine */
		crx_info_print_box_start(0);
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print("<a href=\"http://www.crex.com/\"><img border=\"0\" src=\"");
			crx_info_print(CREX_LOGO_DATA_URI "\" alt=\"Crex logo\" /></a>\n");
		}
		crx_info_print("This program makes use of the Crex Scripting Language Engine:");
		crx_info_print(!sapi_module.crxinfo_as_text?"<br />":"\n");
		if (sapi_module.crxinfo_as_text) {
			crx_info_print(crex_version);
		} else {
			crex_html_puts(crex_version, strlen(crex_version));
		}
		crx_info_print_box_end();
		crex_string_free(crx_uname);
	}

	crex_ini_sort_entries();

	if (flag & CRX_INFO_CONFIGURATION) {
		crx_info_print_hr();
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print("<h1>Configuration</h1>\n");
		} else {
			SECTION("Configuration");
		}
		if (!(flag & CRX_INFO_MODULES)) {
			SECTION("CRX Core");
			display_ini_entries(NULL);
		}
	}

	if (flag & CRX_INFO_MODULES) {
		HashTable sorted_registry;
		crex_module_entry *module;

		crex_hash_init(&sorted_registry, crex_hash_num_elements(&module_registry), NULL, NULL, 1);
		crex_hash_copy(&sorted_registry, &module_registry, NULL);
		crex_hash_sort(&sorted_registry, module_name_cmp, 0);

		CREX_HASH_MAP_FOREACH_PTR(&sorted_registry, module) {
			if (module->info_func || module->version) {
				crx_info_print_module(module);
			}
		} CREX_HASH_FOREACH_END();

		SECTION("Additional Modules");
		crx_info_print_table_start();
		crx_info_print_table_header(1, "Module Name");
		CREX_HASH_MAP_FOREACH_PTR(&sorted_registry, module) {
			if (!module->info_func && !module->version) {
				crx_info_print_module(module);
			}
		} CREX_HASH_FOREACH_END();
		crx_info_print_table_end();

		crex_hash_destroy(&sorted_registry);
	}

	if (flag & CRX_INFO_ENVIRONMENT) {
		SECTION("Environment");
		crx_info_print_table_start();
		crx_info_print_table_header(2, "Variable", "Value");
		tsrm_env_lock();
		for (env=environ; env!=NULL && *env !=NULL; env++) {
			tmp1 = estrdup(*env);
			if (!(tmp2=strchr(tmp1,'='))) { /* malformed entry? */
				efree(tmp1);
				continue;
			}
			*tmp2 = 0;
			tmp2++;
			crx_info_print_table_row(2, tmp1, tmp2);
			efree(tmp1);
		}
		tsrm_env_unlock();
		crx_info_print_table_end();
	}

	if (flag & CRX_INFO_VARIABLES) {
		zval *data;

		SECTION("CRX Variables");

		crx_info_print_table_start();
		crx_info_print_table_header(2, "Variable", "Value");
		if ((data = crex_hash_str_find(&EG(symbol_table), "CRX_SELF", sizeof("CRX_SELF")-1)) != NULL && C_TYPE_P(data) == IS_STRING) {
			crx_info_print_table_row(2, "CRX_SELF", C_STRVAL_P(data));
		}
		if ((data = crex_hash_str_find(&EG(symbol_table), "CRX_AUTH_TYPE", sizeof("CRX_AUTH_TYPE")-1)) != NULL && C_TYPE_P(data) == IS_STRING) {
			crx_info_print_table_row(2, "CRX_AUTH_TYPE", C_STRVAL_P(data));
		}
		if ((data = crex_hash_str_find(&EG(symbol_table), "CRX_AUTH_USER", sizeof("CRX_AUTH_USER")-1)) != NULL && C_TYPE_P(data) == IS_STRING) {
			crx_info_print_table_row(2, "CRX_AUTH_USER", C_STRVAL_P(data));
		}
		if ((data = crex_hash_str_find(&EG(symbol_table), "CRX_AUTH_PW", sizeof("CRX_AUTH_PW")-1)) != NULL && C_TYPE_P(data) == IS_STRING) {
			crx_info_print_table_row(2, "CRX_AUTH_PW", C_STRVAL_P(data));
		}
		crx_print_gpcse_array(CREX_STRL("_REQUEST"));
		crx_print_gpcse_array(CREX_STRL("_GET"));
		crx_print_gpcse_array(CREX_STRL("_POST"));
		crx_print_gpcse_array(CREX_STRL("_FILES"));
		crx_print_gpcse_array(CREX_STRL("_COOKIE"));
		crx_print_gpcse_array(CREX_STRL("_SERVER"));
		crx_print_gpcse_array(CREX_STRL("_ENV"));
		crx_info_print_table_end();
	}


	if (flag & CRX_INFO_CREDITS) {
		crx_info_print_hr();
		crx_print_credits(CRX_CREDITS_ALL & ~CRX_CREDITS_FULLPAGE);
	}

	if (flag & CRX_INFO_LICENSE) {
		if (!sapi_module.crxinfo_as_text) {
			SECTION("CRX License");
			crx_info_print_box_start(0);
			crx_info_print("<p>\n");
			crx_info_print("This program is free software; you can redistribute it and/or modify ");
			crx_info_print("it under the terms of the CRX License as published by the CRX Group ");
			crx_info_print("and included in the distribution in the file:  LICENSE\n");
			crx_info_print("</p>\n");
			crx_info_print("<p>");
			crx_info_print("This program is distributed in the hope that it will be useful, ");
			crx_info_print("but WITHOUT ANY WARRANTY; without even the implied warranty of ");
			crx_info_print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
			crx_info_print("</p>\n");
			crx_info_print("<p>");
			crx_info_print("If you did not receive a copy of the CRX license, or have any questions about ");
			crx_info_print("CRX licensing, please contact license@crx.net.\n");
			crx_info_print("</p>\n");
			crx_info_print_box_end();
		} else {
			crx_info_print("\nCRX License\n");
			crx_info_print("This program is free software; you can redistribute it and/or modify\n");
			crx_info_print("it under the terms of the CRX License as published by the CRX Group\n");
			crx_info_print("and included in the distribution in the file:  LICENSE\n");
			crx_info_print("\n");
			crx_info_print("This program is distributed in the hope that it will be useful,\n");
			crx_info_print("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
			crx_info_print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
			crx_info_print("\n");
			crx_info_print("If you did not receive a copy of the CRX license, or have any\n");
			crx_info_print("questions about CRX licensing, please contact license@crx.net.\n");
		}
	}

	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("</div></body></html>");
	}
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_table_start(void) /* {{{ */
{
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("<table>\n");
	} else {
		crx_info_print("\n");
	}
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_table_end(void) /* {{{ */
{
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("</table>\n");
	}

}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_box_start(int flag) /* {{{ */
{
	crx_info_print_table_start();
	if (flag) {
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print("<tr class=\"h\"><td>\n");
		}
	} else {
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print("<tr class=\"v\"><td>\n");
		} else {
			crx_info_print("\n");
		}
	}
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_box_end(void) /* {{{ */
{
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("</td></tr>\n");
	}
	crx_info_print_table_end();
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_hr(void) /* {{{ */
{
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("<hr />\n");
	} else {
		crx_info_print("\n\n _______________________________________________________________________\n\n");
	}
}
/* }}} */

CRXAPI CREX_COLD void crx_info_print_table_colspan_header(int num_cols, const char *header) /* {{{ */
{
	int spaces;

	if (!sapi_module.crxinfo_as_text) {
		crx_info_printf("<tr class=\"h\"><th colspan=\"%d\">%s</th></tr>\n", num_cols, header );
	} else {
		spaces = (int)(74 - strlen(header));
		crx_info_printf("%*s%s%*s\n", (int)(spaces/2), " ", header, (int)(spaces/2), " ");
	}
}
/* }}} */

/* {{{ crx_info_print_table_header */
CRXAPI CREX_COLD void crx_info_print_table_header(int num_cols, ...)
{
	int i;
	va_list row_elements;
	char *row_element;

	va_start(row_elements, num_cols);
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("<tr class=\"h\">");
	}
	for (i=0; i<num_cols; i++) {
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			row_element = " ";
		}
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print("<th>");
			crx_info_print(row_element);
			crx_info_print("</th>");
		} else {
			crx_info_print(row_element);
			if (i < num_cols-1) {
				crx_info_print(" => ");
			} else {
				crx_info_print("\n");
			}
		}
	}
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("</tr>\n");
	}

	va_end(row_elements);
}
/* }}} */

/* {{{ crx_info_print_table_row_internal */
static CREX_COLD void crx_info_print_table_row_internal(int num_cols,
		const char *value_class, va_list row_elements)
{
	int i;
	char *row_element;

	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("<tr>");
	}
	for (i=0; i<num_cols; i++) {
		if (!sapi_module.crxinfo_as_text) {
			crx_info_printf("<td class=\"%s\">",
			   (i==0 ? "e" : value_class )
			);
		}
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			if (!sapi_module.crxinfo_as_text) {
				crx_info_print( "<i>no value</i>" );
			} else {
				crx_info_print( " " );
			}
		} else {
			if (!sapi_module.crxinfo_as_text) {
				crx_info_print_html_esc(row_element, strlen(row_element));
			} else {
				crx_info_print(row_element);
				if (i < num_cols-1) {
					crx_info_print(" => ");
				}
			}
		}
		if (!sapi_module.crxinfo_as_text) {
			crx_info_print(" </td>");
		} else if (i == (num_cols - 1)) {
			crx_info_print("\n");
		}
	}
	if (!sapi_module.crxinfo_as_text) {
		crx_info_print("</tr>\n");
	}
}
/* }}} */

/* {{{ crx_info_print_table_row */
CRXAPI CREX_COLD void crx_info_print_table_row(int num_cols, ...)
{
	va_list row_elements;

	va_start(row_elements, num_cols);
	crx_info_print_table_row_internal(num_cols, "v", row_elements);
	va_end(row_elements);
}
/* }}} */

/* {{{ crx_info_print_table_row_ex */
CRXAPI CREX_COLD void crx_info_print_table_row_ex(int num_cols, const char *value_class,
		...)
{
	va_list row_elements;

	va_start(row_elements, value_class);
	crx_info_print_table_row_internal(num_cols, value_class, row_elements);
	va_end(row_elements);
}
/* }}} */

/* {{{ Output a page of useful information about CRX and the current request */
CRX_FUNCTION(crxinfo)
{
	crex_long flag = CRX_INFO_ALL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flag)
	CREX_PARSE_PARAMETERS_END();

	/* Andale!  Andale!  Yee-Hah! */
	crx_output_start_default();
	crx_print_info((int)flag);
	crx_output_end();

	RETURN_TRUE;
}

/* }}} */

/* {{{ Return the current CRX version */
CRX_FUNCTION(crxversion)
{
	char *ext_name = NULL;
	size_t ext_name_len = 0;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING_OR_NULL(ext_name, ext_name_len)
	CREX_PARSE_PARAMETERS_END();

	if (!ext_name) {
		RETURN_STRING(CRX_VERSION);
	} else {
		const char *version;
		version = crex_get_module_version(ext_name);
		if (version == NULL) {
			RETURN_FALSE;
		}
		RETURN_STRING(version);
	}
}
/* }}} */

/* {{{ Prints the list of people who've contributed to the CRX project */
CRX_FUNCTION(crxcredits)
{
	crex_long flag = CRX_CREDITS_ALL;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_LONG(flag)
	CREX_PARSE_PARAMETERS_END();

	crx_print_credits((int)flag);
	RETURN_TRUE;
}
/* }}} */

/* {{{ Return the current SAPI module name */
CRX_FUNCTION(crx_sapi_name)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (sapi_module.name) {
		RETURN_STRING(sapi_module.name);
	} else {
		RETURN_FALSE;
	}
}

/* }}} */

/* {{{ Return information about the system CRX was built on */
CRX_FUNCTION(crx_uname)
{
	char *mode = "a";
	size_t modelen = sizeof("a")-1;

	CREX_PARSE_PARAMETERS_START(0, 1)
		C_PARAM_OPTIONAL
		C_PARAM_STRING(mode, modelen)
	CREX_PARSE_PARAMETERS_END();

	RETURN_STR(crx_get_uname(*mode));
}

/* }}} */

/* {{{ Return comma-separated string of .ini files parsed from the additional ini dir */
CRX_FUNCTION(crx_ini_scanned_files)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (crx_ini_scanned_files) {
		RETURN_STRING(crx_ini_scanned_files);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ Return the actual loaded ini filename */
CRX_FUNCTION(crx_ini_loaded_file)
{
	CREX_PARSE_PARAMETERS_NONE();

	if (crx_ini_opened_path) {
		RETURN_STRING(crx_ini_opened_path);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
