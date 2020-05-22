#!/bin/bash

#Script that compiles a minimum HopsanCore defaultLibrary and HopsanCLI
qtver=4
#qtver=5

test $# -ne 1 && echo "Usage: `basename $0` <hopsanRootDirPath>" && exit -1

cd $1

mkdir -p HopsanCore_shb
cd HopsanCore_shb
qmake -qt=$qtver ../HopsanCore/HopsanCore.pro
make -j4 -B
cd ..
mkdir -p componentLibraries/defaultLibrary_shb
cd componentLibraries/defaultLibrary_shb
qmake -qt=$qtver ../defaultLibrary/defaultLibrary.pro
make -j2 -B
cd ../../
#cd dependencies
#./unpackAndBuildZMQandMSGPACK.sh
#cd ..
mkdir -p HopsanCLI_shb
cd HopsanCLI_shb
qmake -qt=$qtver ../HopsanCLI/HopsanCLI.pro
make -j4 -B
cd ..

echo "Compilation Finished"
