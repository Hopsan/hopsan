TEMPLATE = subdirs

SUBDIRS = HopsanCore componentLibraries SymHop Ops HopsanGenerator hopsangeneratorgui hopsanremote hopsanhdf5exporter HopsanGUI HopsanCLI HoLC UnitTests hopsanc

componentLibraries.depends = HopsanCore
HopsanGenerator.depends = HopsanCore SymHop
HopsanCLI.depends = HopsanCore HopsanGenerator hopsanhdf5exporter Ops
HopsanGUI.depends = HopsanCore hopsangeneratorgui hopsanhdf5exporter hopsanremote Ops
hopsanc.depends = HopsanCore
hopsanremote.depends = HopsanCore
UnitTests.depends = HopsanCore HopsanGenerator componentLibraries
