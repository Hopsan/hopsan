#!/bin/bash

# Read libname and determine zip file name
name=$1
file=$name.zip
# Remember bin directory
bindir=`pwd`
# Create the storage location for libraries if it does not already exist
comlibdir=componentLibraries
mkdir -p $comlibdir
# Move the library zip file into storage
mv $file $comlibdir
# Enter storage and unpack
cd $comlibdir
echo Unpacking $file
unzip -o $name.zip
# Enter libdir and remember path
cd $name
libdir=`pwd`

# --------------------------------------
# Do any library specific internal build work
# --------------------------------------

#cd libDFC
#make -B hotint

# --------------------------------------

# Now go back to bin dir and call build utility in CLI
cd $bindir
./HopsanCLI --buildComponentLibrary $libdir/$name.xml
# Script finished
echo buildcomplib.sh finished
