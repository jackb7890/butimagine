@echo OFF

call powershell -executionpolicy remotesigned -file init.ps1 >> hehehe
for /f "delims=" %%x in (hehehe) do set nowayitwasthathard=%%x
call %nowayitwasthathard%
del hehehe

set PATH=%PATH%;%~dp0\scripts

set VCPKG_INC="%~dp0\..\butimagine\build\vcpkg_installed\x64-windows\include"
set VCPKG_ROOT_F=%VCPKG_ROOT:\=/%
