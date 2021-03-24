# Hopsan

![](https://github.com/Hopsan/hopsan/workflows/ghactions-ci-build/badge.svg) [![Build Status](https://travis-ci.org/Hopsan/hopsan.svg?branch=master)](https://travis-ci.org/Hopsan/hopsan) [![Build status](https://ci.appveyor.com/api/projects/status/ouf883yvoel0djgn/branch/master?svg=true)](https://ci.appveyor.com/project/peterNordin/hopsan/branch/master)

Hopsan is a free open-source multi-domain system simulation tool developed at the division of Fluid and mechatronic systems at Linkoping university.

Features include:

* Simulation core library
    * Plain C++ library for easy integration
    * Multi-core support for faster simulations
    * Create your own component models libraries in C++  
      using a subset of Modelica is also supported
    * Embedded simple numeric script language (numhop)
* Command line application
    * Call from external software, save results to file
    * Automate batch simulation
    * Run validation or generate validation data set from model
* Graphical users interface
    * Drag and drop / power-port based modeling including support for reusable subsystems
    * Advanced simulation result analysis capabilities
    * Energy losses calculations
    * Data export to CSV, XML, Gnuplot, HDF5 & Matlab formats
    * Interactive animation of the simulated system (real-time and playback)
    * Model variable sensitivity analysis
    * Frequency-domain analysis (based on simulation results)
    * Numerical optimization
    * Scripting using the HCOM or Python language
    * Functional Mock-Up Interface (FMI) model import/export using co-simulation FMUs
    * Model export to Matlab/Simulink
* Parallel simulation and optimization
    * On local machine using multiple cores
    * On networked computers using the Hopsan simulation server application

## License

The Hopsan simulation core and support libraries are released under the permissive **Apache License 2.0**
* HopsanCore simulation library
* Hopsan default component library
* HopsanCLI Command Line Interface application
* Hopsan generator library
* Ops optimization library
* Symhop symbolic expression library

The GUI applications are released under the copyleft **GNU General Public License 3.0**
* HopsanGUI Graphical modeling and results analysis application

## Download and Installation

### Windows

You can download official release packages from  
https://github.com/Hopsan/hopsan/releases

The Windows version is packaged as a installer or as a portable zip that you can use if you do not have permission to install software on your computer.
You can also choose if you want the compiler included for importing and exporting component libraries and models.

If you choose the zip version, no start menu entry for Hopsan will be added, instead go into the bin directory and double-click hopsangui.exe to start.

### Ubuntu and Debian Packages

You can download official release packages from  
https://github.com/Hopsan/hopsan/releases

Deb packages are built for the current Ubuntu and Debian releases.
You should be able to install them by opening them in you package manager, but if that does not work, try to install it manually using.
```
apt install ./hopsanPackageName
```
Note that you must specify the package name as a path, otherwise apt will search for the package in the repository.

### Snapcraft
https://snapcraft.io/hopsan  
If your GNU/Linux distribution supports Snap packages you can install Hopsan from the "Snap Store"  
[![Hopsan](https://snapcraft.io/hopsan/badge.svg)](https://snapcraft.io/hopsan)  


or from the terminal:
```
snap install hopsan
```
Application menu entries for HopsanGUI will be added but to start the command line applications use a command like:
```
hopsan.addresserver
hopsan.cli
hopsan.gui
hopsan.remoteclient
hopsan.server
hopsan.servermonitor
```
**Known issues:**  

* The Hopsan snap runs in confinement. If the log cache runs out of storage space, you can change temp directory under Options->Plotting, but due to the confinement you do not have many locations to choose from. This may cause problems if you are low on disk space.


### Flatpak
https://flathub.org/apps/details/com.github.hopsan.Hopsan  
If your GNU/Linux distribution supports Flatpak you can install Hopsan from Flathub  
<a href='https://flathub.org/apps/details/com.github.hopsan.Hopsan'><img width='186' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.svg'/></a>

or from a terminal:
```
flatpak install flathub com.github.hopsan.Hopsan
```
**Run with compiler:**  
The compiler is not included inside the Hopsan flatpak. If you need to use the compiler then start Hopsan from the command line using the Sdk as runtime.
This unfortunately also requires that you install the entire org.kde.Sdk
```
flatpak run --devel com.github.hopsan.Hopsan
```
**Run other Hopsan applications:**  
A Hopsan entry that starts HopsanGUI will be added to your application menu, but if you want to start one of the other bundled applications you must use the command line.
```
# List the contents of the bin directory (to see what other executable are available)
flatpak run --command=ls com.github.hopsan.Hopsan  -l /app/bin

# Start application "hopsancli" from inside the flatpak
flatpak run --command=hopsancli com.github.hopsan.Hopsan [arguments]
# or
flatpak run --command=hopsancli --devel com.github.hopsan.Hopsan [arguments]
```

**Known issues:**  

* The Hopsan flatpak runs in confinement and the default log cache storage is in /tmp/Hopsan (inside the container). If the log cache runs out of storage space, you can change temp directory under Options->Plotting, but due to the confinement you do not have many locations to choose from. This may cause problems if you are low on disk space.


## Documentation

https://hopsan.github.io/documentation

## Issue Tracker and Questions
If you want to report an issue, make a feature request or have a question, please create a new issue in the issue tracker.  
If you want to ask a question, please label the issue with the "question" label.
https://github.com/Hopsan/hopsan/issues

## Links

https://github.com/Hopsan  
https://liu.se/en/research/hopsan

# Build Instructions

Hopsan is a cross-platform supported application and it should be possible to build
on most GNU/Linux based distributions, Microsoft Windows (using MinGW) and Apple macOS.

For detailed instructions see [build-instructions.md](build-instructions.md) 
