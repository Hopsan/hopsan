#!/bin/sh
# $Id$

# Shell script building documentation using doxygen
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2011-11-28

cd doc
if [ "$1" = "user" ]; then
    echo "Building user documentation"
    doxygen userDoxyfile
elif [ "$1" = "dev" ]; then
    echo "Building developer documentation"
    doxygen devDoxyfile
else
    echo "Building user and developer documentation"
    doxygen userDoxyfile
    doxygen devDoxyfile
fi
cd $OLDPWD
