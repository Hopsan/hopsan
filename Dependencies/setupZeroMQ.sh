#!/bin/bash
# $Id$

# Shell script to build msgpack and zeromq dependencies automatically
# Author: Peter Nordin peter.nordin@liu.se

zeromqname="zeromq"
msgpackname="msgpack-c"
cppzeromqname="cppzmq"

basepwd=`pwd`

# --------------------
# Build zeromq
# --------------------
cd $zeromqname                          # Enter dir
./autogen.sh
./configure --without-libsodium         # Configure
make -j4                                # Build
cd $basepwd                             # Return


echo "Done!"
