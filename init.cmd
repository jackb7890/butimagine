@echo OFF

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

::CMake is bitch and wants file paths to be with forward slashes so here
set VCPKG_ROOT_F=%VCPKG_ROOT:\=/%