@echo off

echo Cleaning boost build files

if not exist build goto end
rmdir /q /s build

:end
echo Done cleaning boost build files
