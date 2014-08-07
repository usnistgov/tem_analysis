tem_analysis
============

TEM Image Analysis software tool.

## Layout
- centroid: main code, include modules
- data: sample data
- sdk: third party libraries
- old: initial versions of code, for reference

## Build Instructions

### Build Dependencies
- QT 5
- CMake 2.8
- Java 7 JRE

### UNIX Build Instructions
- cd sdk
- ./compile
- cd ../centroid
- mkdir build
- cd build
- cmake ..
- make -j8

### Windows Build Environment
- Download Java SE 7 JRE and install with the default options.
  - Make sure the java\bin directory is present on your PATH
- Download Qt (5.3 is the latest version of Qt5 at this writing) and install
  with the default options.
- Download CMake 2.8 and installed with the default options.
- Download the latest version of Git
  - This is only necessary if you are going to use the Git repository to
    update the source tree. Configuring Git on Windows to use SSH is not
    covered here.
- Download and install 7-zip: http://www.7-zip.org/
  - The sdk\compile.bat file expects to use 7-zip to extract the tar files.
- To open a Qt-enabled command prompt:
  - Start -> All Programs -> Qt -> 5.3 -> MinGW 4.8 (32bit) ->
    Qt 5.3 for Desktop (MinGW 4.8 32bit)
- Verify you can run 7z, cmake, and java from the command prompt.

### Windows Build Instructions
- cd sdk
- compile.bat
- cd ..\centroid
- mkdir build
- cd build
- cmake -G "MinGW Makefiles" ..
- make -j8

## Contributors
- Tomasz Bednarz, CSIRO, tomasz.bednarz@gmail.com
- Wesley Griffin, NIST, wesley.griffin@nist.gov
- John Hagedorn, NIST, john.hagedorn@nist.gov


## PI

Wesley Griffin Div. 771
301.975.8855
wesley.griffin@nist.gov

**Updated:** 2014 Aug 07

