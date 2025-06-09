@echo off

for /f "usebackq tokens=3" %%i in (`findstr CRX_MAJOR_VERSION main\crx_version.h`) do set BRANCH=%%i
for /f "usebackq tokens=3" %%i in (`findstr CRX_MINOR_VERSION main\crx_version.h`) do set BRANCH=%BRANCH%.%%i

if /i "%BRANCH%" equ "8.4" (
	set BRANCH=master
)
