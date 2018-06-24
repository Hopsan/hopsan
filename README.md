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

## Download

You can find official releases and more or less experimental development snapshots at:  
https://flumes.iei.liu.se/hopsan/files/releases

## Documentation

https://flumes.iei.liu.se/hopsan/docs/2.8/html

## Issue Tracker and Questions
If you want to report an issue, make a feature request or have a question, please create a new issue in the issue tracker.  
If you want to ask a question, please label the issue with the "question" label.
https://github.com/Hopsan/hopsan/issues

## Links

https://github.com/Hopsan  
https://liu.se/en/research/hopsan  
https://flumes.iei.liu.se/hopsan

## Migration notice

In may 2017 the Hopsan source code was migrated from subversion to git(hub).
To reduce the repository size, the revision history was thoroughly pruned.
The old repository should still be accessible (read only) at:

**Repository URL:** https://flumes.iei.liu.se/svn/hopsan

**Username:** anonymous  
**Password:** hut6Opoj

In late February 2018, the issue tracker was also moved to GitHub.  
The old one can be found here: https://flumes.iei.liu.se/redmine/projects/hopsan

# Build instructions

Hopsan is a cross-platform supported application and it should be possible to build
on most GNU/Linux based distributions, Microsoft Windows (using MinGW) and Apple macOS.
*macOS building is not supported out of the box yet.*

**Note!** See the developer documentation for detailed instructions.
https://flumes.iei.liu.se/hopsan/docs/2.8/html/page_hopsandevelopment.html

## Cloning the source code
To get the source code clone it including dependencies using the following commands.
```
git clone https://github.com/Hopsan/hopsan.git
cd hopsan
git submodule update --init
```
If a submodule fails to clone then you must get a hold of it some other way.
**Note!** Not all of the dependencies are required. See the documentation for details.

## Build on a GNU/Linux system
TODO Write this  
For now, see the developer documentation linked above

## Build on a Microsoft Windows system
TODO Write this  
For now, see the developer documentation linked above
