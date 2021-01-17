# Build Instructions

The default Hopsan build environment is centered around the QtCreator IDE and uses qmake project files and the qmake program to generate Makefiles.
HopsanGUI and some other components require the Qt libraries but the core components such as the HopsanCore library and HopsanCLI application are written in "Plain C++".
On Windows the MinGW-w64 compiler is used by default. But it is also possible to build using Microsofts Visual C++ Compiler, if the CMake build system is used.

## Cloning the Source Code
To get the source code, clone the main repository and required submodules using the following commands.
```
git clone https://github.com/Hopsan/hopsan.git
cd hopsan
git submodule update --init
```
### Checkout and Updated Submodules
Occasionally submodules are updated. After you checkout or update (pull) a branch, you should always re-run ```git submodule update --init``` to ensure that all submodules are up-to-date and that new ones are added.

If a submodule has been removed or replaced, you may also need to run: ```git submodule deinit NAME```

If you get an error similar to: ```error: Server does not allow request for unadvertised object d062edd...```  
Then you may need to run:
```
git submodule sync
git submodule update --init
```

## Build on a Microsoft Windows System

### Basic Requirements
To build Hopsan and dependency libraries you will need to install
* Python3 (To run the dependency download script)
* CMake (To build most dependencies and Hopsan (experimental))
* Qt framework (Hopsan uses qmake to build, and some parts depend on the Qt libraries)
* MinGW-w64 compiler package or Microsoft Visual C++ Compiler

You can use any version of Qt5 (and corresponding MinGW-w64 version) that you want, but if you want the same ones used to build the official Hopsan releases, you can download it and mingw-w64 using the dependency download scripts.
If you want to build Hopsan using the Microsoft Visual C++ Compiler, then download either Microsoft Visual Studio Build Tools or Microsoft Visual Studio.

**Note!** You should choose one tool chain, MinGW or MS Visual C++ Compiler. Do not mix building with both of them in the same code and build directories.

#### Tool Chain 32-bit MinGW

If you want to build a 32-bit release you should use the official Qt OpenSource 5.9.9 or newer package, and make sure to install the included MinGW32 compiler.
You can look and change (if needed) the expected paths to Qt and MingGW inside the ```dependencies/setHopsanBuildPaths.bat``` file. Make sure that you change ```hopsan_arch=x64``` to ```hopsan_arch=x86```.

#### Tool Chain 64-bit MinGW (Recommended)

Technically you can use any Qt/MinGW-w64 combination that you like, so you can download the official Qt release 5.12.7 or newer that come with 64-bit MinGW.
Unfortunately this version does not include QtWebKit (which is no longer officially supported by Qt) but needed by Hopsan for showing built-in documentation.
If you are not bothered by loading documentation in an external browser, then it is recommended to use the official Qt release.

If you want to use the custom build version including QtWebkit (Qt 5.9.9) you can download it using the ```dependencies/download-dependencies.py``` script.  
```download-dependencies.py mingw-w64:5.4.0 qt-mingw-w64:5.9.9 --include-toolchain```  
The script will only download the files, you can unpack and move them to your desired location. 
After unpack and move, you may need to run the bundled ```qtbinpatcher.exe``` to adjust the Qt installation file paths.
You can find and change (if needed) the expected paths to Qt and MingGW inside the ```dependencies/setHopsanBuildPaths.bat``` file.

###  Tool Chain 64-bit (Microsoft Visual C++ Compiler)

If you want to use the Microsoft compiler, download the official Qt packages for the version you want to use.
Any recent Qt5 version should work.
You also need  Microsoft Visual Studio Build Tools or Microsoft Visual Studio.

### Third-party Dependencies

To download, verify and unpack the source code for third-party dependencies, use the ```dependencies/download-dependencies.py``` script.
* Open a terminal and got to the ```dependencies``` directory
* To fetch all dependencies: ```download-dependencies.py --all```
* To fetch only the bare minimum run: ```download-dependencies.py qwt fmilibrary tclap```

#### Build Third-party Dependencies (MinGW)
Build the dependencies using each ```setupName.bat``` scripts. Run them by double-clicking or running them from a CMD terminal.

#### Build Third-party Dependencies (Microsoft Visual C++ Compiler)
Open a CMD terminal, (not Powershell) and set some environment variables according to this example:
```
REM Set override values before calls to setupName.bat which in turn calls setHopsanBuildPaths.bat
set HOPSAN_BUILD_QT_HOME=C:\Qt\5.12.10\msvc2017_64
set HOPSAN_BUILD_CMAKE_GENERATOR="Visual Studio 15 2017 Win64"
REM or
set HOPSAN_BUILD_QT_HOME=C:\Qt\5.15.2\msvc2019_64
set HOPSAN_BUILD_CMAKE_GENERATOR="Visual Studio 16 2019"

set HOPSAN_BUILD_MINGW_HOME=WeDoNotWantMingwInPATH
```
Build the dependencies using each ```setupName.bat``` scripts. Run them from inside the terminal where you set the environment variables (do not double click the scripts).

### Build Hopsan (MinGW)
At this point you can choose to build Hopsan using the Qt Creator IDE that you can download from https://www.qt.io/offline-installers or to build directly in a terminal.  
If you do not want to use the Qt Creator IDE, see the ```.appveyor.yaml``` for an example on how to build using the MinGW compiler.
In this case you should use the ```win32-g++``` spec even if you are building on a 64-bit Windows platform.
Essentially it boils down to running: 
```
pushd hopsan-code
call dependencies\setHopsanBuildPaths.bat
mkdir build
pushd build
qmake.exe ..\HopsanNG.pro -r -spec win32-g++ CONFIG+=release
popd
mingw32-make.exe -j16 -C build
```

If you want to use Qt Creator to build and develop, open the project file ```HopsanNG.pro``` and configure the project.
You will need to configure a "kit" that consists of the compiler and qt version that you downloaded as part of the tool-chain step above.

**Note!** With this method, the resulting binaries will be installed back into the source-code directory.

### Build Hopsan (CMake for Microsoft Visual C++ Compiler)

**Note!** Not all features of Hopsan are available when using the Visual C++ Compiler, primarily code-generator support import/export for MSVC is missing.  
The following example shows how to build using Microsoft Visual Studio Build Tools 2019, but you can also use Visual Studio if you want.
Open a CMD terminal (not Powershell).

```
REM Set override values before call to setHopsanBuildPaths.bat
set HOPSAN_BUILD_QT_HOME=C:\Qt\5.15.2\msvc2019_64
set HOPSAN_BUILD_CMAKE_GENERATOR="Visual Studio 16 2019"
set HOPSAN_BUILD_MINGW_HOME=WeDoNotWantMingwInPATH

set PATH=C:\Program Files\Cmake\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64
call C:\Workspace\hopsan-code\dependencies\setHopsanBuildPaths.bat

REM Create and enter build directory
mkdir c:\Workspace\hopsan-build-msvc
pushd c:\Workspace\hopsan-build-msvc

REM Configure
set BUILD_TYPE=Release
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=..\hopsan-install-msvc ..\hopsan-code -DCMAKE_PREFIX_PATH=%HOPSAN_BUILD_QT_HOME% -G%HOPSAN_BUILD_CMAKE_GENERATOR%
REM Build
cmake --build . --config %BUILD_TYPE% --parallel 16
REM Optional, run tests
ctest --parallel 16 --output-on-failure -C %BUILD_TYPE%
REM Install
cmake --build . --config %BUILD_TYPE% --parallel 16 --target install
```

You can also choose to open the generated Visual Studio project and develop/build inside Visual Studio. From the same terminal (since it has the environment set up):
```
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" Hopsan.sln
```

## Build on a GNU/Linux system

### Basic Requirements
Exactly what packages you need to install varies between different Linux distributions, the packages listed here are valid for Debian based ones.
The package names may be different in your distribution.

The following packages are required:  
```build-essential cmake qt5-default qtbase5-dev qtbase5-private-dev libqt5opengl5-dev libqt5svg5-dev libqt5webkit5-dev```  
(```build-essential``` can be replaced with: ```gcc g++ make```)

### Third-party Dependencies

You can choose to build local variants of the dependencies using the setup scripts in the *dependencies* sub directory, or use version from your distribution package repository.
Some dependencies are not available in the distribution package archives and need to be built locally by running:
* ```./setupFMILibrary.sh```
* ```./setupKatex.sh```
* ```./setupTclap.sh```

The shell scripts will automatically call *download-dependencies.py* to download, verify and unpack the source code before building.

#### Packages
You may want to install the following packages unless you build them locally using the *setupName.sh* scripts:  
```libhdf5-dev libmarkdown2-dev libqwt-qt5-dev libzmq3-dev libmsgpack-dev```


### Build Hopsan
At this point you can choose to build Hopsan using the Qt Creator IDE that you can download from https://www.qt.io/offline-installers or to build directly in a terminal.  
If you do not want to use the Qt Creator IDE, see the ```.travis.yml``` for an example on how to build.
Essentially it boils down to running: 
```
pushd hopsan-code
source ./dependencies/setHopsanBuildPaths.sh
mkdir -p build
pushd build
qmake ../HopsanNG.pro -r -spec linux-g++ -config release
popd
make -j16 -C build
```

If you want to use Qt Creator to build and develop, open the project file ```HopsanNG.pro``` and configure the project.
You will need to configure a "kit" that consists of the compiler and qt version that you downloaded as part of the tool-chain step above.

**Note!** With this method, the resulting binaries will be installed back into the source-code directory.

### Build Hopsan using CMake
To use CMake, first download and setup tool chain and third-party dependencies according to the instructions above for your platform.
Then build using something similar to:
```
# Create and enter build directory
mkdir -p ${HOME}/workspace/hopsan-build
pushd ${HOME}/workspace/hopsan-build
# Setup environment
source ${HOME}/workspace/hopsan-code/dependencies/setHopsanBuildPaths.sh
# Configure
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../hopsan-install ../hopsan-code
# Build
cmake --build . --config Release --parallel 16
# Run tests (optional)
ctest -C Release --output-on-failure --parallel 16
# Install
cmake --build . --config Release --target install
```

## Build on a Apple macOS system
There is no official build for macOS, but users have reported being able to build Hopsan manually.  
See https://github.com/Hopsan/hopsan/issues/1711 for details.  
You can also look at the macOS continuous-integration build script for Travis CI in the file ```.travis.yml```. 
This script uses Homebrew to download and install the Qt framework, the remaining dependencies are built locally by the *setupName.sh* scripts. They should work on both Linux and macOS.

## Generating Documentation
The Hopsan documentation is based on Doxygen. Additionally Graphviz and dvipng (comes with LaTeX) are used to generate dependency graphs and equations.

### Generate on GNU/Linux
Install the necessary packages with: `apt-get install doxygen graphviz dvipng`

To build the documentation, run the *buildDocumentation.sh* script. Add the argument, **full**, to build the full documentation including code reference.
Open `doc/html/index.html` in a web browser after building.

### Generate on Windows

* (Required) The latest version of Doxygen can be downloaded here: http://www.stack.nl/~dimitri/doxygen/download.html#latestsrc
* (Optional) A LaTeX package including dvipng (to generate png for latex equations), We recommend: http://miktex.org/
* (Optional) Ghostscript **32-bit** version for formulas, install to the default directory. Example: `C:/ProgramFiles/gs/gs9.27` or `C:/ProgramFiles (x86)/gs/gs9.27`  
  The search paths are hard-coded in the *buildDocumentation.bat* script, you may need to add new path setting in the script if your version is newer.  
  You can find it here: https://www.ghostscript.com/download/gsdnld.html
* (Optional) The latest stable version of Graphviz can be downloaded here: https://graphviz.org/download  
  The search paths are hard-coded in the *buildDocumentation.bat* script, you may need to add new path setting in the script if your version is newer.

To build the documentation, double click the *buildDocumentation.bat* or *buildFullDocumentation.bat* script files.
Open `doc/html/index.html` in a web browser after building.

### Generate with CMake
If you use CMake to build hopsan, you can add the configuration option ```-DBUILD_USER_DOCUMENTATION=ON``` or ```-DBUILD_DEVELOPER_DOCUMENTATION=ON``` to automatically
include the documentation in the build without the need to call additional scripts.
