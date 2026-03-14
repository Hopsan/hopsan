#!/bin/bash
./setupAsio.sh
./setupDiscount.sh
./setupFmi4c.sh
./setupHDF5.sh
./setupKatex.sh
./setupLibzip.sh
./setupMsgpack.sh
./setupQwt.sh
./setupTclap.sh
./setupXerces.sh
./setupZeroMQ.sh
# DCPLib must come after xerces and libzip
./setupDCPLib.sh
