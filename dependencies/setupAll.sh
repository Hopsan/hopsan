#!/bin/bash

# First base libraries xerces depends on libzip
./setupAsio.sh
./setupLibzip.sh
./setupXerces.sh

# DCPLib depends on xerces and libzip
./setupDCPLib.sh
./setupDiscount.sh
./setupFmi4c.sh
./setupHDF5.sh
./setupKatex.sh
./setupMsgpack.sh
./setupQwt.sh
./setupTclap.sh
./setupZeroMQ.sh
