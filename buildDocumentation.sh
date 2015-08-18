#!/bin/sh
# $Id$

# Shell script building documentation using doxygen
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2011-11-28

cd doc
if [ "$1" = "full" ]; then
    echo "Building full documentation"
    doxygen Doxyfile_full
else
    echo "Building user documentation"
    doxygen Doxyfile
fi
cd $OLDPWD
