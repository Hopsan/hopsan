set HOPSAN_BUILD_SCRIPT_NOPAUSE=true
start /wait cmd /c setupDiscount.bat
start /wait cmd /c setupFmi4c.bat
start /wait cmd /c setupFMILibrary.bat
start /wait cmd /c setupQwt.bat
start /wait cmd /c setupMsgpack.bat
start /wait cmd /c setupZeroMQ.bat
start /wait cmd /c setupKatex.bat
start /wait cmd /c setupTclap.bat
start /wait cmd /c setupHDF5.bat
start /wait cmd /c setupXerces.bat
start /wait cmd /c setupLibzip.bat
start /wait cmd /c setupDCPLib.bat
