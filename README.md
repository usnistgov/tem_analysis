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

