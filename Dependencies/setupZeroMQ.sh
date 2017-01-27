#!/bin/bash
# $Id$

# Shell script to build msgpack and zeromq dependencies automatically
# Author: Peter Nordin peter.nordin@liu.se

zeromqname="zeromq"

basepwd=`pwd`

# --------------------
# Build zeromq
# --------------------
cd $zeromqname                          # Enter dir
#./autogen.sh
#./configure --without-libsodium         # Configure
mkdir -p build
cd build
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=../build_install ../ 
make -j4                                # Build
make install
make test



echo "Done!"
