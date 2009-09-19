@echo off

echo Building libxml2...

pushd libxml2*
pushd win32

if "%HIPPOENV_BUILD%" == "MSVC" goto BUILD_MSVC

:BUILD_MSVC

set LIBXML_OUT_DIR=bin.msvc
cscript configure.js zlib=yes
nmake -f Makefile.msvc

goto end


:end

if exist ..\..\lib goto CHECK_BIN
mkdir ..\..\lib
:CHECK_BIN
if exist ..\..\bin goto COPY_FILES
mkdir ..\..\bin

:COPY_FILES
copy %LIBXML_OUT_DIR%\*.lib ..\..\lib
copy %LIBXML_OUT_DIR%\*.lib ..\..\lib

echo Finished building libxml2

popd
popd 
