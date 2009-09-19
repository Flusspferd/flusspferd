@echo off

set BOOST_ROOT=%CD%\boost_1_40_0

if exist build goto run_cmake
mkdir build

:run_cmake
pushd build

if "%HIPPOENV_BUILD%" == "MSVC" goto build_msvc

:build_msvc
cmake -G "NMake Makefiles" %BOOST_ROOT%
nmake boost_filesystem boost_thread

goto done_build

:build_mingw
echo TODO: Implement building boost for MINGW
goto done

:done_build
if exist ..\..\bin goto check_lib
mkdir ..\..\bin
:check_lib
if exist ..\..\lib goto copy_files
mkdir ..\..\lib

:copy_files
copy /Y lib\* ..\..\lib
copy /Y bin\* ..\..\bin

set BOOST_INCLUDE_DIR=%BOOST_ROOT%

pushd ..\..
set BOOST_LIBRARIES_DIR=%CD%\lib
set BOOST_BINARIES_DIR=%CD%\bin
popd

:done
popd

