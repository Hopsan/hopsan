#!/bin/bash
# Shell script to setup tclap

basedir=`pwd`
name=tclap
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

if [[ -d ${installdir} ]]; then
    rm -rf ${installdir}
fi
pushd ${codedir}
./configure --prefix ${installdir} --enable-doxygen=no
make -j$(getconf _NPROCESSORS_ONLN) install
popd

echo "setupTclap.sh done!"
