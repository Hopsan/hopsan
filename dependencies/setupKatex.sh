#!/bin/bash
# Shell script to setup katex

basedir=`pwd`
name=katex
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

if [[ -d ${installdir} ]]; then
    rm -rf ${installdir}
fi
cp -a ${codedir} ${installdir}

echo "setupKatex.sh done!"
