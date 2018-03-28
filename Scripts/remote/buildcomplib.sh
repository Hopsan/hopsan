#!/bin/bash

# Read libname and determine zip file name
name=$1
file=$name.zip
# Remember home directory
homedir=`pwd`
# Unpack
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

# Now go back to home dir and call build utility in CLI
cd $homedir
hopsancli --buildComponentLibrary $libdir/$name.xml
