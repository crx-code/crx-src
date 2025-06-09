@echo off
cscript /nologo /e:jscript win32\build\buildconf.js %*
SET CRX_BUILDCONF_PATH=%~dp0
copy %CRX_BUILDCONF_PATH%\win32\build\configure.bat %CRX_BUILDCONF_PATH% > nul
SET CRX_SDK_SCRIPT_PATH=

IF NOT EXIST %CRX_BUILDCONF_PATH% (echo Error generating configure script, configure script was not copied) ELSE (echo Now run 'configure --help')
