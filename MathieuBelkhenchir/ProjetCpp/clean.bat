del *.sdf
del *.ncb
del *.VC.db
del /A:H *.suo
rmdir /Q /S Debug
rmdir /Q /S Release
rmdir /Q /S "engine_test\Debug"
rmdir /Q /S "engine_test\Release"
rmdir /Q /S "engine_test\x64"
rmdir /Q /S "minicraft\Debug"
rmdir /Q /S "minicraft\Release"
rmdir /Q /S "minicraft\x64"
rmdir /Q /S ipch
rmdir /Q /S .vs
rmdir /Q /S x64
pause
