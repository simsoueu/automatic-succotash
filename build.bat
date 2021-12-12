if not exist "build" mkdir build

pushd build

cls 

gcc ..\win32.cpp -luser32 -lgdi32 -o win32.exe

popd