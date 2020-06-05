# Build Instructions

The default Hopsan build environment is centered around the QtCreator IDE and uses qmake project files and the qmake program to generate Makefiles.
HopsanGUI and some other components require the Qt libraries but the core components such as the HopsanCore library and HopsanCLI application are written in "Plain C++".
On Windows the MinGW-w64 compiler is used by default. Previously many years ago, the Microsoft VC++ compiler was used as well, but only for the core library.
Currently it is not supported, but as the CMake build-system matures, this support will be restored.

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
* CMake (To build most dependencies)
* Qt framework (Hopsan uses qmake to build, and some parts depend on the Qt libraries)
* MinGW-w64 compiler package

You can use any version of Qt (and corresponding MinGW-w64 version) that you want, but if you want the same ones used to build the official Hopsan releases, you can download it and mingw-w64 using the dependency download scripts.

#### Tool Chain 32-bit

If you want to build a 32-bit release you should use the official Qt OpenSource 5.9.9 or newer package, and make sure to install the included MinGW32 compiler.
You can look and change (if needed) the expected paths to Qt and MingGW inside the *dependencies/setHopsanBuildPaths.bat* file.

#### Tool Chain 64-bit (Recommended)

Technically you can use any Qt/MinGW-w64 combination that you like, so you can download the official Qt release 5.12.7 or newer that come with 64-bit MinGW.
Unfortunately this version does not include QtWebKit (which is no longer officially supported by Qt) but needed by Hopsan for showing built-in documentation.
If you are not bothered by loading documentation in an external browser, then it is recommended to use the official Qt release.

If you want to use the custom build version including QTWebkit (Qt 5.9.9) you can download it using the *dependencies/download-dependencies.py* script.  
```download-dependencies.py mingw-w64:5.4.0 qt-mingw-w64:5.9.9 --include-toolchain```  
The script will only download the files, you can unpack and move them to your desired location.
You can look and change (if needed) the expected paths to Qt and MingGW inside the *dependencies/setHopsanBuildPaths.bat* file.

### Third-party Dependencies

To download, verify and unpack the source code for third-party dependencies, use the *dependencies/download-dependencies.py* script.
* Open a terminal and got to the *dependencies* directory
* To fetch all dependencies: ```download-dependencies.py --all```
* To fetch only the bare minimum run: ```download-dependencies.py qwt fmilibrary tclap```

Build the dependencies using each ```setupName.bat``` scripts. Run them by double-clicking or running them from a CMD terminal.

### Build Hopsan
At this point you can choose to build Hopsan using the Qt Creator IDE that you can download from https://www.qt.io/offline-installers or to build directly in a terminal.  
If you do not want to use the Qt Creator IDE, see the ```.appveyor.yaml``` for an example on how to build using the MinGW compiler.
In this case you should use the ```win32-g++``` spec even if you are building on a 64-bit Windows platform.
Essentially it boils down to running: 
```
cd dependencies
call setHopsanBuildPaths.bat
cd ..
mkdir build
cd build
qmake.exe ..\HopsanNG.pro -r -spec win32-g++ CONFIG+=release
cd ..
mingw32-make.exe -j8 -C build
```

If you want to use Qt Creator to build and develop, open the project file *HopsanNG.pro* and configure the project.
You will need to configure a "kit" that consists of the compiler and qt version that you downloaded as part of the tool-chain step above.

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
pushd dependencies
source ./setHopsanBuildPaths.sh
popd
mkdir -p build
pushd build
qmake ../HopsanNG.pro -r -spec linux-g++ -config release
popd
make -j8 -C build
```

If you want to use Qt Creator to build and develop, open the project file *HopsanNG.pro* and configure the project.
You will need to configure a "kit" that consists of the compiler and qt version that you downloaded as part of the tool-chain step above.


## Build on a Apple macOS system
There is no official build for macOS, but users have reported being able to build Hopsan manually.  
See https://github.com/Hopsan/hopsan/issues/1711 for details.  
You can also look at the macOS continuous-integration build script for Travis CI in the file ```.travis.yml```. 
This script uses Homebrew to download and install the Qt framework, the remaining dependencies are built locally by the *setupName.sh* scripts. They should work on both Linux and macOS.

## Build using CMake
Hopsan has partial support for the cross-platform CMake build-system.
Currently GUI components are not being build.
To use CMake, first download and setup tool chain and third-party dependencies according to the instructions above for your platform.
Then build using something similar to:
```
pushd hopsan/dependencies
source ./setHopsanBuildPaths.sh
popd
mkdir -p hopsan-build
pushd hopsan-build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../hopsan-install ../hopsan
cmake --build . --config Release --parallel 8
ctest --parallel 8
cmake --build . --config Release --target install
```
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
