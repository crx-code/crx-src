#  +----------------------------------------------------------------------+
#  | Copyright (c) The CRX Group                                          |
#  +----------------------------------------------------------------------+
#  | This source file is subject to version 3.01 of the CRX license,      |
#  | that is bundled with this package in the file LICENSE, and is        |
#  | available through the world-wide-web at the following url:           |
#  | https://www.crx.net/license/3_01.txt                                 |
#  | If you did not receive a copy of the CRX license and are unable to   |
#  | obtain it through the world-wide-web, please send a note to          |
#  | license@crx.net so we can mail you a copy immediately.               |
#  +----------------------------------------------------------------------+
#  | Author: Wez Furlong <wez@thebrainroom.com>                           |
#  +----------------------------------------------------------------------+
#
# This is the makefile template for the win32 build

CC="$(CRX_CL)"
LD="$(LINK)"
MC="$(MC)"
MT="$(MT)"
RE2C="$(RE2C)"
PGOMGR="$(PGOMGR)"
CRX_BUILD=$(CRX_BUILD)

!if "$(CRX_SIMD_SCALE)" == ""
SIMD_SUFFIX=
!else
!if "$(CRX_SIMD_SCALE)" == "SSE2"
SIMD_SUFFIX=
!else
SIMD_SUFFIX=-$(CRX_SIMD_SCALE)
!endif
!endif

!if "$(PKG_SUFFIX)" != ""
PKG_SUFFIX_PART=-$(PKG_SUFFIX)
!endif

MCFILE=$(BUILD_DIR)\wsyslog.rc
BUILD_DIR_DEV_NAME=crx-$(CRX_VERSION_STRING)-devel-$(CRX_COMPILER_SHORT)-$(CRX_ARCHITECTURE)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART)
BUILD_DIR_DEV=$(BUILD_DIR)\$(BUILD_DIR_DEV_NAME)

DIST_ZIP_SNAP=crx-$(CRX_VERSION_STRING)$(CRX_ZTS_ARCHIVE_POSTFIX)-Win32-$(CRX_COMPILER_SHORT)-$(CRX_ARCHITECTURE)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART).zip
DIST_ZIP_PECL=pecl-$(CRX_VERSION_STRING)$(CRX_ZTS_ARCHIVE_POSTFIX)-Win32-$(CRX_COMPILER_SHORT)-$(CRX_ARCHITECTURE)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART).zip
DIST_ZIP_DEV_PACK=crx-devel-pack-$(CRX_VERSION_STRING)$(CRX_ZTS_ARCHIVE_POSTFIX)-Win32-$(CRX_COMPILER_SHORT)-$(CRX_ARCHITECTURE)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART).zip
DIST_ZIP_TEST_PACK=crx-test-pack-$(CRX_VERSION_STRING)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART).zip
DIST_ZIP_DEBUG_PACK=crx-debug-pack-$(CRX_VERSION_STRING)$(CRX_ZTS_ARCHIVE_POSTFIX)-Win32-$(CRX_COMPILER_SHORT)-$(CRX_ARCHITECTURE)$(SIMD_SUFFIX)$(PKG_SUFFIX_PART).zip

!if "$(DEBUGGER)" == "1"
DEBUGGER_CMD=devenv
DEBUGGER_ARGS=/debugexe
!else
DEBUGGER_CMD=
DEBUGGER_ARGS=
!endif

all: generated_files $(EXT_TARGETS) $(PECL_TARGETS) $(SAPI_TARGETS)

build_dirs: $(BUILD_DIR) $(BUILD_DIRS_SUB) $(BUILD_DIR_DEV)

!if $(RE2C) == ""
generated_files: build_dirs \
	Crex\crex_ini_parser.c Crex\crex_ini_parser.h \
	Crex\crex_language_parser.c \
	sapi\crxdbg\crxdbg_parser.c \
	$(CRXDEF) $(MCFILE)
!else
generated_files: build_dirs \
	Crex\crex_ini_parser.c Crex\crex_ini_parser.h \
	Crex\crex_language_parser.c \
	Crex\crex_ini_scanner.c Crex\crex_ini_scanner_defs.h \
	Crex\crex_language_scanner.c Crex\crex_language_scanner_defs.h \
	sapi\crxdbg\crxdbg_parser.c sapi\crxdbg\crxdbg_lexer.c \
	$(CRXDEF) $(MCFILE)
!endif

$(BUILD_DIR)\$(CRXDLL).def: $(CRX_DLL_DEF_SOURCES)
	type $(CRX_DLL_DEF_SOURCES) > $(BUILD_DIR)\$(CRXDLL).def

Crex\crex_ini_parser.c Crex\crex_ini_parser.h: Crex\crex_ini_parser.y
	$(BISON) --output=Crex/crex_ini_parser.c -v -d Crex/crex_ini_parser.y

Crex\crex_language_parser.c Crex\crex_language_parser.h: Crex\crex_language_parser.y
	$(BISON) --output=Crex/crex_language_parser.c -v -d Crex/crex_language_parser.y
	@if "$(SED)" neq "" $(SED) -i "s,^int crexparse\(.*\),CREX_API int crexparse\1,g" Crex/crex_language_parser.c
	@if "$(SED)" neq "" $(SED) -i "s,^int crexparse\(.*\),CREX_API int crexparse\1,g" Crex/crex_language_parser.h
	@if "$(SED)" neq "" $(SED) -i "s,^#ifndef YYTOKENTYPE,#include \"crex.h\"\n#ifndef YYTOKENTYPE,g" Crex/crex_language_parser.h

sapi\crxdbg\crxdbg_parser.c sapi\crxdbg\crxdbg_parser.h: sapi\crxdbg\crxdbg_parser.y
	$(BISON) --output=sapi/crxdbg/crxdbg_parser.c -v -d sapi/crxdbg/crxdbg_parser.y

!if $(RE2C) != ""
Crex\crex_ini_scanner.c Crex\crex_ini_scanner_defs.h: Crex\crex_ini_scanner.l
	$(RE2C) $(RE2C_FLAGS) --no-generation-date --case-inverted -cbdFt Crex/crex_ini_scanner_defs.h -oCrex/crex_ini_scanner.c Crex/crex_ini_scanner.l

Crex\crex_language_scanner.c Crex\crex_language_scanner_defs.h: Crex\crex_language_scanner.l
	$(RE2C) $(RE2C_FLAGS) --no-generation-date --case-inverted -cbdFt Crex/crex_language_scanner_defs.h -oCrex/crex_language_scanner.c Crex/crex_language_scanner.l

sapi\crxdbg\crxdbg_lexer.c: sapi\crxdbg\crxdbg_lexer.l
	$(RE2C) $(RE2C_FLAGS) --no-generation-date -cbdFo sapi/crxdbg/crxdbg_lexer.c sapi/crxdbg/crxdbg_lexer.l
!endif

!if "$(ZTS)" == "1"
CRX8_PGD_SUFFIX=ts
!else
CRX8_PGD_SUFFIX=
!endif

!if $(PGOMGR) != ""
CRX8_PGD_OPTION=/PGD:$(PGOPGD_DIR)\crx8$(CRX8_PGD_SUFFIX).pgd
!else
CRX8_PGD_OPTION=
!endif

CRXDLL_RES=$(BUILD_DIR)\$(CRXDLL).res

$(MCFILE): win32\build\wsyslog.mc
	$(MC) -h win32\ -r $(BUILD_DIR)\ -x $(BUILD_DIR)\ win32\build\wsyslog.mc

#	$(RC) /fo $(MCFILE) $(BUILD_DIR)\wsyslog.rc

!if $(MT) == ""
_VC_MANIFEST_EMBED_EXE=
_VC_MANIFEST_EMBED_DLL=
!else
_VC_MANIFEST_EMBED_EXE= if exist $@.manifest $(MT) -nologo -manifest $@.manifest -outputresource:$@;1
_VC_MANIFEST_EMBED_DLL= if exist $@.manifest $(MT) -nologo -manifest $@.manifest -outputresource:$@;2
!endif

$(CRXDLL_RES): win32\build\template.rc
	$(RC) /nologo /fo $(CRXDLL_RES) /d FILE_DESCRIPTION="\"CRX Script Interpreter\"" \
		/d FILE_NAME="\"$(CRXDLL)\"" /d PRODUCT_NAME="\"CRX Script Interpreter\"" \
		/I$(BUILD_DIR) /d MC_INCLUDE="\"$(MCFILE)\"" \
		win32\build\template.rc

$(BUILD_DIR)\$(CRXDLL): generated_files $(CRXDEF) $(CRX_GLOBAL_OBJS) $(STATIC_EXT_OBJS) $(CRXDLL_RES) $(ASM_OBJS) $(MCFILE)
	@copy win32\build\default.manifest $(BUILD_DIR)\$(CRXDLL).manifest >nul
#	@$(CC) $(CRX_GLOBAL_OBJS) $(STATIC_EXT_OBJS) $(STATIC_EXT_LIBS) $(LIBS) $(CRXDLL_RES) /link /out:$(BUILD_DIR)\$(CRXDLL) $(CRX8_PGD_OPTION) $(CRX_LDFLAGS) $(LDFLAGS) $(STATIC_EXT_LDFLAGS)
	@"$(LINK)" $(CRX_GLOBAL_OBJS_RESP) $(STATIC_EXT_OBJS_RESP) $(STATIC_EXT_LIBS) $(LIBS) $(ASM_OBJS) $(CRXDLL_RES) /out:$(BUILD_DIR)\$(CRXDLL) $(CRX8_PGD_OPTION) $(CRX_LDFLAGS) $(LDFLAGS) $(STATIC_EXT_LDFLAGS)
	-@$(_VC_MANIFEST_EMBED_DLL)

$(BUILD_DIR)\$(CRXLIB): $(BUILD_DIR)\$(CRXDLL)

$(BUILD_DIR) $(BUILD_DIRS_SUB) $(BUILD_DIR_DEV):
	@echo Recreating build dirs
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@cd $(BUILD_DIR)
	@for %D in ($(BUILD_DIRS_SUB)) do @if not exist %D @mkdir %D > NUL
	@if not exist $(BUILD_DIR_DEV) @mkdir $(BUILD_DIR_DEV) > NUL
	@cd "$(CRX_SRC_DIR)"


clean-sapi:
	@echo Cleaning SAPI
	@for %D in (_x $(EXT_TARGETS)) do @if exist $(BUILD_DIR)\%D @del /F /Q $(BUILD_DIR)\%D > NUL
	@for %D in (_x $(PECL_TARGETS)) do @if exist $(BUILD_DIR)\%D @del /F /Q $(BUILD_DIR)\%D > NUL
	@for %D in (_x $(SAPI_TARGETS)) do @if exist $(BUILD_DIR)\%D @del /F /Q $(BUILD_DIR)\%D > NUL
	-@del /F /Q $(BUILD_DIR)\$(CRXDLL)

clean: clean-sapi
	@echo Cleaning distribution build dirs
	@cd $(BUILD_DIR)
	@for %D in (_x $(BUILD_DIRS_SUB)) do @if exist %D @del /F /Q %D\*.* > NUL
	@cd "$(CRX_SRC_DIR)"
	-@del /F /Q $(BUILD_DIR)\*.res $(BUILD_DIR)\*.manifest $(BUILD_DIR)\*.lib $(BUILD_DIR)\*.ilk $(BUILD_DIR)\*.pdb $(BUILD_DIR)\*.exp $(CRXDEF) $(BUILD_DIR)\crx-$(CRX_VERSION_STRING)-Win32.zip $(BUILD_DIR)\pecl-$(CRX_VERSION_STRING)-Win32.zip > NUL
	-rd /s /q $(BUILD_DIR)\crx-$(CRX_VERSION_STRING)

clean-pecl:
	@echo Cleaning PECL targets only
	-rd /s /q $(BUILD_DIR)\pecl

clean-all:
	@echo Cleaning standard build dirs
	@cd $(BUILD_DIR)
	@for %D in (_x $(BUILD_DIRS_SUB)) do @if exist %D @rd /s /q %D
	@cd "$(CRX_SRC_DIR)"
	-@del /f /q $(BUILD_DIR)\*.res $(BUILD_DIR)\*.lib $(BUILD_DIR)\*.ilk $(BUILD_DIR)\*.pdb $(BUILD_DIR)\*.exp $(CRXDEF) $(BUILD_DIR)\*.rc $(BUILD_DIR)\*.manifest $(BUILD_DIR)\*.dbg $(BUILD_DIR)\*.bin $(BUILD_DIR)\crx*.dll $(BUILD_DIR)\crx*.exe > NUL

clean-pgo: clean-all
	-rd /s /q $(BUILD_DIR)\crx-$(CRX_VERSION_STRING)
	-rd /s /q $(BUILD_DIR)\pecl-$(CRX_VERSION_STRING)
	-rd /s /q $(BUILD_DIR)\crx-test-pack-$(CRX_VERSION_STRING)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_SNAP)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_DEBUG_PACK)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_PECL)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_TEST_PACK)


!if $(CRX_TEST_INI_PATH) == ""
test: set-tmp-env
	$(DEBUGGER_CMD) $(DEBUGGER_ARGS) "$(BUILD_DIR)\crx.exe" -d open_basedir= -d output_buffering=0 run-tests.crx $(TESTS) -p "$(BUILD_DIR)\crx.exe"

run: set-tmp-env
	$(DEBUGGER_CMD) $(DEBUGGER_ARGS) "$(BUILD_DIR)\crx.exe" $(ARGS)
!else
test: set-tmp-env
	$(DEBUGGER_CMD) $(DEBUGGER_ARGS) "$(BUILD_DIR)\crx.exe" -n -d open_basedir= -d output_buffering=0 -d memory_limit=-1 run-tests.crx -p "$(BUILD_DIR)\crx.exe" -n -c $(CRX_TEST_INI_PATH) $(TESTS)

run: set-tmp-env
	$(DEBUGGER_CMD) $(DEBUGGER_ARGS) "$(BUILD_DIR)\crx.exe" -n -c $(CRX_TEST_INI_PATH) $(ARGS)
!endif

build-snap: set-tmp-env generated_files
	@$(MAKE) "$(BUILD_DIR)\$(CRXDLL)"
	-for %T in ($(SAPI_TARGETS)) do $(MAKE) /I /nologo "%T"
	-for %T in ($(EXT_TARGETS)) do $(MAKE) /I /nologo "%T"
	-for %T in ($(PECL_TARGETS)) do $(MAKE) /I /nologo "%T"

build-dist: $(BUILD_DIR)\deplister.exe
	-rd /s /q $(BUILD_DIR)\crx-$(CRX_VERSION_STRING)
	-rd /s /q $(BUILD_DIR)\pecl-$(CRX_VERSION_STRING)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_SNAP)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_DEBUG_PACK)
	-del /f /q $(BUILD_DIR)\$(DIST_ZIP_PECL)
	$(CRX_SDK_CRX_CMD) -d date.timezone=UTC -n -dcrxa.readonly=0 win32/build/mkdist.crx "$(CRX_VERSION_STRING)" "$(BUILD_DIR)" "$(CRX_BUILD)" "$(CRXDLL)" "$(SAPI_TARGETS)" "$(EXT_TARGETS) $(CRX_EXTRA_DIST_FILES)" "$(PECL_TARGETS) $(PECL_EXTRA_DIST_FILES)" "$(SNAPSHOT_TEMPLATE)"
	cd $(BUILD_DIR)\crx-$(CRX_VERSION_STRING)
	-$(7ZA) a ..\$(DIST_ZIP_SNAP) .
	cd $(BUILD_DIR)\pecl-$(CRX_VERSION_STRING)
	-$(7ZA) a ..\$(DIST_ZIP_PECL) .
	cd $(BUILD_DIR)\crx-test-pack-$(CRX_VERSION_STRING)
	-$(7ZA) a ..\$(DIST_ZIP_TEST_PACK) .
	cd $(BUILD_DIR)
	-$(7ZA) a $(DIST_ZIP_DEBUG_PACK) *.pdb
	-$(7ZA) a $(DIST_ZIP_DEV_PACK) $(BUILD_DIR_DEV_NAME)
	cd $(BUILD_DIR)

dist: all build-dist
snap: build-snap build-devel build-dist

$(BUILD_DIR)\deplister.exe:	win32\build\deplister.c
	$(CC) /nologo /Fo$(BUILD_DIR)\ /Fd$(BUILD_DIR)\ /Fp$(BUILD_DIR)\ /FR$(BUILD_DIR) /Fe$(BUILD_DIR)\deplister.exe win32\build\deplister.c imagehlp.lib

install: really-install install-sdk

build-lib: build-ext-libs
	@if not exist $(BUILD_DIR_DEV)\lib mkdir $(BUILD_DIR_DEV)\lib >nul
	@copy $(BUILD_DIR)\$(CRXLIB) $(BUILD_DIR_DEV)\lib /y >nul

build-devel: build-headers build-lib
	@if not exist $(BUILD_DIR_DEV)\script mkdir $(BUILD_DIR_DEV)\script >nul
	@if not exist $(BUILD_DIR_DEV)\build mkdir $(BUILD_DIR_DEV)\build >nul
	@copy run-tests.crx $(BUILD_DIR_DEV)\script\ /y >nul
	@copy build\gen_stub.crx $(BUILD_DIR_DEV)\build\ /y >nul
	@copy win32\build\confutils.js $(BUILD_DIR_DEV)\script\ /y >nul
	@copy win32\build\configure.tail $(BUILD_DIR_DEV)\script\ /y >nul
	@copy win32\build\config.w32.crxize.in $(BUILD_DIR_DEV)\script\ /y >nul
	@copy win32\build\Makefile.crxize $(BUILD_DIR_DEV)\script\ /y >nul
	@copy win32\build\crxize.bat $(BUILD_DIR_DEV)\ /y >nul
	@copy win32\build\template.rc $(BUILD_DIR_DEV)\build\ /y >nul
	@copy win32\build\default.manifest $(BUILD_DIR_DEV)\build\ /y >nul
	@copy $(BUILD_DIR)\devel\config.crxize.js $(BUILD_DIR_DEV)\script\ /y >nul
	@copy $(BUILD_DIR)\devel\crxize.js $(BUILD_DIR_DEV)\script\ /y >nul
	@copy $(BUILD_DIR)\devel\ext_deps.js $(BUILD_DIR_DEV)\script\ /y >nul

install-sdk: build-devel
	@xcopy /Q /Y /E /I $(BUILD_DIR_DEV)\* $(CRX_PREFIX)\SDK

really-install:
	@if not exist $(CRX_PREFIX) mkdir $(CRX_PREFIX)
	@if not exist $(CRX_PREFIX)\ext mkdir $(CRX_PREFIX)\ext
	@echo Installing files under $(CRX_PREFIX)
	@if exist $(BUILD_DIR)\*.exe copy $(BUILD_DIR)\*.exe $(CRX_PREFIX) /y >nul
	@type nul > $(BUILD_DIR)\extension_dlls.txt
	@if exist $(BUILD_DIR)\crx_*.dll copy $(BUILD_DIR)\crx_*.dll $(CRX_PREFIX)\ext /y >nul & dir /b $(BUILD_DIR)\crx_*.dll > $(BUILD_DIR)\extension_dlls.txt
	@xcopy $(BUILD_DIR)\*.dll /exclude:$(BUILD_DIR)\extension_dlls.txt $(CRX_PREFIX) /y >nul
	@echo Registering event source with syslog (requires admin rights)
	@echo It's okay for this step to fail:
	-$(CRX_PREFIX)\crx.exe -n -dextension_dir=$(CRX_PREFIX) win32/build/registersyslog.crx $(CRX_PREFIX)\$(CRXDLL)
	-regedit /s win32\syslog.reg
