Imebra - 4.0.11.1
******************

This is a Github fork of the imebra Dicom library (version 4.0.11.1) hosted on Bitbucket. Please find the original repository on https://bitbucket.org/binarno/imebra/wiki/Home. No modifications have been made to the original source code. The remainder of the Readme file is taken from the original project.



Introduction
============

Imebra is a C++ DICOM library able to parse & build DICOM files decompress/compress/process the embedded images.

A JNI interface allows to use the library also with the JAVA language, including on Android OS.


Creating the source distribution from the code in the VCS
=========================================================

If you downloaded the source distribution then you can skip this part and jump directly to "Compiling Imebra".

This section describes how to create an Imebra Source Distribution containing the full
C++source code, the Java wrappers for Android, the HTML documentation, the test units code coverage
information.

The Imebra image is built on Ubuntu 16.04 with the following apt packages:

- mercurial
- doxygen
- g++
- lcov
- python
- python-pip
- libgtest-dev
- ant
- cmake
- swig

Additionally, two Python packages must be present (installable with pip):

- breathe
- sphinx_rtd_theme

In order to install the packages, type the following commands as root or prepend them with sudo:
::

    apt-get install mercurial
    apt-get install doxygen
    apt-get install g++
    apt-get install lcov
    apt-get install python
    apt-get install python-pip
    apt-get install python-sphinx
    pip install breathe
    pip install sphinx_rtd_theme
    apt-get install libgtest-dev
    apt-get install ant
    apt-get install cmake
    apt-get install swig

Then clone the Imebra mercurial repository:
::

    hg clone https://bitbucket.org/binarno/imebra

Finally, cd into the imebra folder and execute ant to build the source distribution that includes:

- full C++ source code
- generated Java source code for the Android wrapper
- html documentation
- unit tests
- code coverage report

::

    cd imebra
    ant


Versioning
==========

Public releases of Imebra V4 are versioned according to the rules defined in `Semantic versioning <http://semver.org/>`_.

Nightly releases are versioned as public releases but their minor version or patch number are set to 999 (the master branch
or the major version branches have a minor version set to 999, the minor version branches have a patch number set to 999).

For instance:

- version 4.1.999.45 indicates a nightly build of version 4.1, build from the commit number 45 in the branch.
- version 4.999.999.678 indicates a nightly build of version 4 (master branch), build from the commit number 678 in the branch.


Compiling Imebra
================

This section explains how to compile Imebra from the source distribution.
You can download the source distribution from https://imebra.com, or you can create a source distribution
by following the instructions at "Creating the source distribution from the code in the VCS"

The result of the Imebra compilation is a shared library for your operating system of choice.

The Imebra Source Distribution includes:

- the source code for the library
- the source code for the tests
- pre-built documentation
- pre-generated JNI headers for Java


Compiling the C++ version of Imebra
-----------------------------------

Prerequisites
.............

In order to build the library from the Imebra Source Distribution you need:

- a modern C++ compiler (GCC, clang, Visual Studio, etc)
- CMake version 2.8 or newer (https://cmake.org/)

Building Imebra
...............

The library folder contains a CMakeLists file, which contains the information needed by
CMake to generate a solution file for your platform (a make file, a VisualStudio solution, etc).

To generate the Imebra shared library, execute the following steps:

1. Create a folder that will contain the result of the compilation (artifacts)
2. cd into the created artifacts folder
3. execute cmake with the path to the Imebra's library folder as parameter
4. execute cmake with the --build option, and on Windows optionally specify the build configuration

For instance:

::

    md artifacts
    cd artifacts
    cmake imebra_location/library
    cmake --build .

The first CMake command will generate a solution file for the your platform: this will be a 
make file on Linux, a VisualStudio solution of Windows, an XCode project on Mac.

The second CMake command with the --build option will launch make, VisualStudio or the build
chain defined on your machine.

On Windows, the last cmake command can be followed by --config release or --config debug, like
this:

::

    cmake --build . --config release

or

::

    cmake --build . --config debug



Compiling the Android version of Imebra
---------------------------------------

Prerequisites
.............

In order to build the Android version of Imebra you need:

- Apache Ant
- the Android SDK
- the Android NDK

Building Imebra
...............

The Android version of the library needs both the Java source code (located in the wrappers/javaWrapper folder)
and the C++ source code (located in the library folder)

To generate the Imebra Jar library:

1. cd into the Imebra wrappers/javaWrapper folder
2. run ant and define the properties sdk.dir and ndk.dir so they point to the home folders of the Android SDK and NDK respectively
3. the produced JAR will be located in the folder wrappers/javaWrapper/out/artifacts

For instance:

::

    cd wrappers/javaWrapper
    ant -Dsdk.dir=path/to/Android/SDK -Dndk.dir=path/to/Android/NDK
    


Compiling the test units
------------------------

Prerequisites
.............

In order to build and execute the tests you need:

- a compiled gtest library and its include file (get it here https://github.com/google/googletest)
- the compiled C++ version of Imebra

Building the tests
..................

To compile te tests, execute the following steps:

1. create a folder that will contain the test units executable
2. cd into the created folder
3. run cmake with the path to the tests/CMakeLists.txt as a parameter. You can also define the
   CMake variables imebra_library, gtest_library and gtest_include with the path to the
   imebra library, gtest library and gtest include folder respectively
4. run cmake --build .

For instance:

::

    md tests_artifacts
    cd tests_artifacts
    cmake -Dimebra_library="path/to/imebra/library" -Dgtest_library="path/to/gtest/library" -Dgtest_include="path/to/gtest/include" imebra_location/tests
    cmake --build .


