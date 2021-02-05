# Jpegtest
libjpeg test project

## Pre-requisites
* linux os
* git
* cmake>=3.16
* libjpeg
* libboost
* swig installed

## Task
1. Create a library implementing methods described in jpeg\_manipulator.h. Use the libjpeg library.
2. Compile a simple application with command-line interface performing simple actions on input jpeg images. Use you library from step 1.
3. Create a Python wrapper using SWIG and create a simple test program in Python language. Use CMake as the build tool.

## Notes
Use following commands to build the project:
(in project root)
mkdir build
cd build
cmake ..
make
make install

### To run test Py application:

```
cd ${GITROOT}/python
./test.py -h
```

### To run test C++ application:

```
cd ${GITROOT}/build
./testapp --help
```

###  Docker image
You can get auto-built image from Dockerhub.
https://hub.docker.com/r/mikhailmew/jpeg_wrapped

```
