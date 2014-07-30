@echo off

"C:\Program Files\7-Zip\7z.exe" e InsightToolkit-4.5.0-nist.tar.gz
"C:\Program Files\7-Zip\7z.exe" x InsightToolkit-4.5.0-nist.tar
move InsightToolKit-4.5.0 ITK-4.5.0
del InsightToolKit-4.5.0-nist.tar

cd ITK-4.5.0
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j8
cd ..\..


"C:\Program Files\7-Zip\7z.exe" e VTK-6.1.0-nist.tar.gz
"C:\Program Files\7-Zip\7z.exe" x VTK-6.1.0-nist.tar
copy /Y VTK-6.0.1-CMakeLists-windows.txt VTK-6.1.0\CMakeLists.txt
del VTK-6.1.0-nist.tar

cd VTK-6.1.0
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j8
cd ..\..

