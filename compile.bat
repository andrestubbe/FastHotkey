@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo FastHotkey v1.0 - Native Build Script
echo ===========================================
echo.

:: Check for Java
if not defined JAVA_HOME (
    :: Try to find Java automatically
    if exist "C:\Program Files\Java\jdk-25\include\jni.h" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-25"
    ) else if exist "C:\Program Files\Java\jdk-21\include\jni.h" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-21"
    ) else if exist "C:\Program Files\Java\jdk-17\include\jni.h" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-17"
    ) else if exist "C:\Program Files\Java\jdk-11\include\jni.h" (
        set "JAVA_HOME=C:\Program Files\Java\jdk-11"
    ) else (
        echo ERROR: Cannot find Java installation with jni.h
        echo Please set JAVA_HOME environment variable
        pause
        exit /b 1
    )
)

if not exist "%JAVA_HOME%\include\jni.h" (
    echo ERROR: Cannot find jni.h in %JAVA_HOME%\include
    echo Please check your Java installation
    echo JAVA_HOME is set to: %JAVA_HOME%
    pause
    exit /b 1
)

echo Found Java at: %JAVA_HOME%

:: Use vswhere to find Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found!
    echo Visual Studio Installer might be missing.
    echo.
    pause
    exit /b 1
)

:: Find VS installation path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL=%%i"
)

if not defined VS_INSTALL (
    echo ERROR: Visual Studio with C++ tools not found!
    echo.
    pause
    exit /b 1
)

echo Found Visual Studio at: %VS_INSTALL%

:: Setup VS environment
set "VCVARS=%VS_INSTALL%\VC\Auxiliary\Build\vcvars64.bat"

echo Setting up Visual Studio environment...
call "%VCVARS%"
if errorlevel 1 (
    echo ERROR: Failed to setup VS environment
    pause
    exit /b 1
)

:: Create build directory
if not exist build mkdir build

:: Compile
echo.
echo Compiling FastHotkey v1.0 with Win32 Hotkey API...
echo =====================================================
cl /LD /Fe:build\fasthotkey.dll ^
    native\fasthotkey.cpp ^
    user32.lib "%JAVA_HOME%\lib\jvm.lib" ^
    /I"%JAVA_HOME%\include" ^
    /I"%JAVA_HOME%\include\win32" ^
    /EHsc /std:c++17 /O2 /W3

:: Check result
if %errorlevel% neq 0 (
    echo.
    echo =====================================================
    echo COMPILATION FAILED
    echo =====================================================
    echo Check errors above
    pause
    exit /b 1
)

:: Copy to resources
echo.
echo Copying DLL to resources...
if not exist src\main\resources mkdir src\main\resources
if not exist src\main\resources\native mkdir src\main\resources\native
copy build\fasthotkey.dll src\main\resources\native\fasthotkey.dll

:: Success
echo.
echo =====================================================
echo COMPILATION SUCCESSFUL!
echo =====================================================
echo.
echo FastHotkey v1.0 DLL created with:
echo - Win32 RegisterHotKey/UnregisterHotKey API
echo - Global system-wide hotkey support
echo - Low-level message loop for hotkey events
echo.
echo You can now build with Maven:
echo   mvn clean package
echo.
pause
