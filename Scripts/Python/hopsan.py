class hopsan:
    def __init__(self):
        import ctypes
        import os
        if os.name == "posix":
            self.hdll = ctypes.cdll.LoadLibrary(os.getenv('HOPSANHOME')+"/bin/libhopsanc.so")
            libpath= os.getenv('HOPSANHOME')+"componentLibraries/defaultLibrary/libdefaultcomponentlibrary.so"
        elif os.name == "nt":
            self.hdll = ctypes.cdll.LoadLibrary(os.getenv('HOPSANHOME')+"/bin/hopsanc.dll")
            libpath= os.getenv('HOPSANHOME')+"componentLibraries/defaultLibrary/defaultcomponentlibrary.dll"
        self.hdll.printWaitingMessages()
        if os.path.exists(libpath):
            self.hdll.loadLibrary(libpath.encode())

    def loadModel(self, path):
        self.hdll.loadModel(path.encode())

    def simulate(self):
        self.hdll.simulate()

    def getTimeVector(self):
        samples = self.hdll.getNumberOfLogSamples()
        import ctypes
        arrayType = ctypes.c_double * samples
        t = arrayType(*[1] * samples)
        self.hdll.getTimeVector(t)
        return t

    def getDataVector(self, name):
        samples = self.hdll.getNumberOfLogSamples()
        import ctypes
        arrayType = ctypes.c_double * samples
        x = arrayType(*[1] * samples)
        self.hdll.getDataVector(name.encode(),x)
        return x

    def setParameter(self, name, value):
        self.hdll.setParameter(name.encode(), value.encode())

    def setStartTime(self, value):
        import ctypes
        self.hdll.setStartTime.argtypes = [ctypes.c_double]
        self.hdll.setStartTime(value)

    def setTimeStep(self, value):
        import ctypes
        self.hdll.setTimeStep.argtypes = [ctypes.c_double]
        self.hdll.setTimeStep(value)

    def setStopTime(self, value):
        import ctypes
        self.hdll.setStopTime.argtypes = [ctypes.c_double]
        self.hdll.setStopTime(value)

    def setLogSamples(self, value):
        import ctypes
        self.hdll.setNumberOfLogSamples.argtypes = [ctypes.c_int]
        self.hdll.setNumberOfLogSamples(value)
