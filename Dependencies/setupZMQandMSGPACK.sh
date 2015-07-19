#!/bin/bash
# $Id$

# Shell script to build msgpack and zeromq dependencies automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

zeromqname="zeromq4-1-4.1.2"
msgpackname="msgpack-c-cpp-1.0.1"
cppzeromqname="cppzmq-master"

msgpackfile="$msgpackname.zip"
zeromqfile="$zeromqname.zip"
cppzeromqfile="$cppzeromqname.zip"

basepwd=`pwd`

# If arg 1 is --force then override regardless
if [ "$1" != "--force" ]; then
    # Abort if dir already exist. When running release build script we dont want to build twice
    if [ -d $msgpackname ]; then
        echo "Directory $msgpackname already exist. Remove it or give argument --force if you want (re)build using this script."
        exit 0
    fi
    if [ -d $zeromqname ]; then
        echo "Directory $zeromqname already exist. Remove it or give argument --force if you want (re)build using this script."
        exit 0
    fi
fi

# --------------------
# No need to build msgpack
# --------------------
rm -rf $msgpackname                     # Clean old files
unzip -q $msgpackfile                   # Unpack
#patch --binary -p0 < $msgpack.patch    # Patch (if any)
cd $msgpackname                         # Enter dir
# No build required
cd $basepwd                             # Return

# --------------------
# Build zeromq
# --------------------
rm -rf $zeromqname                      # Clean old files
unzip -q $zeromqfile                    # Unpack
#patch --binary -p0 < $zeromq.patch     # Patch (if any)
cd $zeromqname                          # Enter dir
./autogen.sh
./configure --without-libsodium         # Configure
make -j4                                # Build
cd $basepwd                             # Return

# --------------------
# Build cppzeromq
# --------------------
rm -rf $cppzeromqname                   # Clean old files
unzip -q $cppzeromqfile                 # Unpack
#patch --binary -p0 < $zeromq.patch     # Patch (if any)
cd $cppzeromqname                       # Enter dir
# No build required
cd $basepwd                             # Return

echo "Done!"
