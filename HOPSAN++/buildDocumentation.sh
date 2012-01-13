#!/bin/sh

# Shell script building documentation using doxygen
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2011-11-28

cd doc
if [ $1 = 'user' ]; then
    doxygen userDoxyfile
elif [ $1 = 'dev' ]; then
    doxygen devDoxyfile
else
    doxygen userDoxyfile
    doxygen devDoxyfile
fi
cd $OLDPWD

