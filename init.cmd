@echo OFF

call powershell -executionpolicy remotesigned -file init.ps1 >> hehehe
for /f "delims=" %%x in (hehehe) do set nowayitwasthathard=%%x
call %nowayitwasthathard%
del hehehe

set OLD_PATH=%PATH%
set PROJ_DIR=%~dp0

set VCPKG_INC="%PROJ_DIR%\..\butimagine\build\vcpkg_installed\x64-windows\include"
set VCPKG_ROOT_F=%VCPKG_ROOT:\=/%

set DEBUG_BIN=%PROJ_DIR%\build\Debug\
set RELEASE_BIN=%PROJ_DIR%\build\Release\

:: debug_bin is on the path so its easier running project when debugging
set PATH=%PATH%;%PROJ_DIR%;%PROJ_DIR%\scripts;%DEBUG_BIN%;

git config --global format.pretty oneline