# Hopsan

[![Build Status](https://travis-ci.org/Hopsan/hopsan.svg?branch=master)](https://travis-ci.org/Hopsan/hopsan) [![Build status](https://ci.appveyor.com/api/projects/status/ouf883yvoel0djgn/branch/master?svg=true)](https://ci.appveyor.com/project/peterNordin/hopsan/branch/master)

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
* HoLC The Hopsan Library Creator application

## Download and Installation

### Windows and Deb Packages

You can find official releases and development snapshots at:  
https://flumes.iei.liu.se/hopsan/files/releases

#### Windows
The Windows version comes packaged as either an installer or as portable zip that you can use if you do not have permission to install software on your computer. You can also choose if you want the compiler included for importing and exporting component libraries and models.

If you choose the zip version, no start menu entry for Hopsan will be added, instead go into the bin directory and double-click hopsangui.exe to start.

#### Deb Packages
Deb packages are built for the current Ubuntu and Debian releases.
You should be able to install them by opening them in you package manager, but if that does not work, try to install it manually using.
```
dpkg -i hopsanPackageName
```
In this case you will also need to install dependencies manually, dpkg will tell you what you need.
Use "apt-get install" to install them.


### Flatpak
https://www.flathub.org

If your GNU/Linux distribution supports Flatpak you can install Hopsan from flathub
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


### Snapcraft
https://snapcraft.io

If your GNU/Linux distribution supports Snap packages you can install Hopsan from the "Snapstore"
```
snap install hopsan
```
Application menu entries for HopsanGUI and HoLC will be added but to start the command line applications use a command like:
```
hopsan.addresserver
hopsan.cli
hopsan.gui
hopsan.holc
hopsan.remoteclient
hopsan.server
hopsan.servermonitor
```
**Known issues:**  

* Hopsan currently looks like old software due to not inheriting the desktop theme properly, you can go into Options and enable "Native style sheet" to make it look slightly better.
* The Hopsan snap runs in confinement. If the log cache runs out of storage space, you can change temp directory under Options->Plotting, but due to the confinement you do not have many locations to choose from. This may cause problems if you are low on disk space.


## Documentation

https://flumes.iei.liu.se/hopsan/docs/latest/html

## Issue Tracker and Questions
If you want to report an issue, make a feature request or have a question, please create a new issue in the issue tracker.  
If you want to ask a question, please label the issue with the "question" label.
https://github.com/Hopsan/hopsan/issues

## Links

https://github.com/Hopsan  
https://liu.se/en/research/hopsan  
https://flumes.iei.liu.se/hopsan

## Migration Notice

In may 2017 the Hopsan source code was migrated from subversion to git(hub).
To reduce the repository size, the revision history was thoroughly pruned.
The old repository should still be accessible (read only) at:

**Repository URL:** https://flumes.iei.liu.se/svn/hopsan

**Username:** anonymous  
**Password:** hut6Opoj

In late February 2018, the issue tracker was also moved to GitHub.  
The old one was previously available here: https://flumes.iei.liu.se/redmine/projects/hopsan

# Build Instructions

Hopsan is a cross-platform supported application and it should be possible to build
on most GNU/Linux based distributions, Microsoft Windows (using MinGW) and Apple macOS.
*macOS building is not supported out of the box yet.*

**Note!** See the developer documentation for detailed instructions.
https://flumes.iei.liu.se/hopsan/docs/latest/html/page_hopsandevelopment.html

## Cloning the Source Code
To get the source code clone it including dependencies using the following commands.
```
git clone https://github.com/Hopsan/hopsan.git
cd hopsan
git submodule update --init
```
If a submodule fails to clone then you must get a hold of it some other way.
**Note!** Not all of the dependencies are required. See the documentation for details.

### Checkout and updated submodules
Occasionally submodules are updated. After you checkout or update (pull) a branch, you may need to re-run ```git submodule update --init```

If you get an error similar to:  
```error: Server does not allow request for unadvertised object d062edd...```
Then you may need to run:
```
git submodule sync
git submodule update --init
```

## Build on a GNU/Linux system
TODO Write this  
For now, see the developer documentation linked above

## Build on a Microsoft Windows system
TODO Write this  
For now, see the developer documentation linked above
