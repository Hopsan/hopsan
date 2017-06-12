# Hopsan

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
https://www.iei.liu.se/flumes/system-simulation/hopsan/download?l=en  
http://flumes.iei.liu.se/hopsan/files/

## Links

https://www.iei.liu.se/flumes/system-simulation/hopsan?l=en  
https://github.com/Hopsan


## Migration notice

In may 2017 the Hopsan source code was migrated from subversion to git(hub).
To reduce the repository size, the revision history was thoroughly pruned.
The old repository should still be accessible (read only) at:

**Repository URL:** https://flumes.iei.liu.se/svn/hopsan

**Username:** anonymous  
**Password:** hut6Opoj

# Build instructions

Hopsan is a cross-platform supported application and it should be possible to build
on most GNU/Linux based distributions, Microsoft Windows (using MinGW) and Apple Mac OS.
*Mac building is not supported out of the box yet.*

**Note!** See the "old" developer documentation for detailed instructions.
http://flumes.iei.liu.se/hopsan/docs/0.7/html/page_hopsandevelopment.html

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
For now, see the old developer documentation

## Build on a Microsoft Windows system
TODO Write this  
For now, see the old developer documentation