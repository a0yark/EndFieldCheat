@echo off
chcp 65001 >nul
if not defined CMAKE_EXE set "CMAKE_EXE=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set "PROJECT_DIR=%~dp0"
if not defined BUILD_DIR set "BUILD_DIR=C:\temp\EndfieldBuild"

echo PROJECT_DIR=%PROJECT_DIR%
echo BUILD_DIR=%BUILD_DIR%

echo === CMake Configure ===
"%CMAKE_EXE%" -B "%BUILD_DIR%" -A x64 -S "%PROJECT_DIR%" 2>&1
if %ERRORLEVEL% neq 0 (
    echo CMake configure FAILED with code %ERRORLEVEL%
    exit /b 1
)

echo === CMake Build ===
"%CMAKE_EXE%" --build "%BUILD_DIR%" --config Release 2>&1
if %ERRORLEVEL% neq 0 (
    echo CMake build FAILED
    exit /b 1
)

echo === Build SUCCESS ===
echo Output: %BUILD_DIR%\Release\
