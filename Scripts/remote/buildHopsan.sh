#!/bin/bash

cd hopsantrunk
cd HopsanCore
qmake -qt=5
make -j4 -B
cd ..
cd componentLibraries/defaultLibrary
qmake -qt=5
make -j2 -B
cd ../../
cd dependencies
./setupZMQandMSGPACK.sh
./setupFMILibrary.sh
cd ..
cd SymHop
qmake -qt=5
make -j4 -B
cd ..
cd Ops
qmake -qt=5
make -j4 -B
cd ..
cd HopsanRemote
qmake -qt=5
make -j4 -B
cd ..
cd HopsanCLI
qmake -qt=5
make -j4 -B
cd ..
cd HopsanGenerator
sed "s|useqtgui=True|useqtgui=False|g" -i HopsanGenerator.pro
qmake -qt=5
make -j4 -B
cd ..
