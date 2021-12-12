if not exist "dev" mkdir dev

pushd dev

cls 

gcc ..\win32.cpp -luser32 -lgdi32 -o win32.exe

popd