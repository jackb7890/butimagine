pushd %~dp0\..\build
cmake --build . %*
popd