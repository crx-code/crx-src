@echo off
SET CRX_BUILDCONF_PATH=%~dp0
cscript /nologo /e:jscript %CRX_BUILDCONF_PATH%\script\crxize.js %*
IF NOT EXIST configure.bat (
	echo Error generating configure script, configure script was not copied
	exit /b 3
) ELSE (
	echo Now run 'configure --help'
)
SET CRX_BUILDCONF_PATH=
