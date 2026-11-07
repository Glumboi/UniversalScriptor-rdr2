@echo off

REM Build xmake targets (version, TCCBridge, GlumityLib, etc.)
xmake
if %errorlevel% neq 0 (
    echo [ERROR] xmake build failed!
    goto :error
)

echo.
echo -> Building MainLoader-Odin...
cd src/MainLoader-Odin
call build.bat
if %errorlevel% neq 0 (
    echo [ERROR] MainLoader-Odin build failed!
    cd /d "%~dp0"
    goto :error
)

echo.
echo -> Copying Odin binary to build folder next to version.dll...
REM Adjust "GlumityToolSuite2.dll" if your Odin build script outputs a different name/file path
if exist "build\GlumityToolSuite2.dll" (
    copy /Y "build\GlumityToolSuite2.dll" "..\..\build\"
)

REM Return to original directory
cd /d "%~dp0"

echo.
echo -> Build and copy completed successfully!
goto :end

:error
echo.
echo Build process encountered an error.

:end
pause