@echo off

echo Cleaning up libxml2 build files...

if not exist bin goto clean_lib
rmdir /q /s bin
:clean_lib
if not exist lib goto clean_msvc
rmdir /q /s lib
:clean_msvc
set CONTINUE_CLEAN=clean_mingw
set CLEAN_EXT="msvc"
goto do_clean

:clean_mingw
set CONTINUE_CLEAN=end
set CLEAN_EXT="mingw"
goto do_clean

goto end
:do_clean

if not exist bin.%CLEAN_EXT% goto cleaner_a_dll
rmdir /q /s bin.%CLEAN_EXT%

:cleaner_a_dll
if not exist int.a.dll.%CLEAN_EXT% goto cleaner_a
rmdir /q /s int.a.dll.%CLEAN_EXT%

:cleaner_a
if not exist int.a.%CLEAN_EXT% goto cleaner
rmdir /q /s int.a.%CLEAN_EXT%

:cleaner
if not exist int.%CLEAN_EXT% goto cleaner_utils
rmdir /q /s int.%CLEAN_EXT%

:cleaner_utils
if not exist int.utils.%CLEAN_EXT% goto cleanup_finished
rmdir /q /s int.utils.%CLEAN_EXT%

:cleanup_finished
goto %CONTINUE_CLEAN%

:end
echo Cleanup of libxml2 build files done

