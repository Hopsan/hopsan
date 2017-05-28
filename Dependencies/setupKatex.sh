#!/bin/bash

# Shell script to extract pre buikt files from katex release tag
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
name=katex
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

cp -a ${codedir}/dist ${installdir}

echo "setupKatex.sh done!"
