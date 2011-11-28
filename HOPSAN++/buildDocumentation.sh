#!/bin/sh

# Shell script building documentation using doxygen
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2011-11-28
# TODO: Tak input argument and select user or dev or both

cd doc
doxygen userDoxyfile
doxygen devDoxyfile
cd $OLDPWD

