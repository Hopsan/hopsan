import time

modelPath = "../Models/Example Models/Load Sensing System.hmf"

hopsan.closeAllModels()
for i in range(nModels):
  hopsan.loadModel(modelPath)

hopsan.useSingleCore()

startSingleThreaded = time.time()
for a in range(nRuns):
  hopsan.simulateAllOpenModelsSequencially(False)
elapsedSingleThreaded = (time.time() - startSingleThreaded)

hopsan.useMultiCore()

startSequencially = time.time()
hopsan.simulateAllOpenModelsSequencially(False)
for a in range(nRuns):
  hopsan.simulateAllOpenModelsSequencially(True)
elapsedSequencially = (time.time() - startSequencially)

startWithoutSplit = time.time()
hopsan.simulateAllOpenModelsWithoutSplit(False)
for a in range(nRuns-1):
  hopsan.simulateAllOpenModelsWithoutSplit(True)
elapsedWithoutSplit = (time.time() - startWithoutSplit)

startWithSplit = time.time()
hopsan.simulateAllOpenModelsWithSplit(False)
for a in range(nRuns-1):
  hopsan.simulateAllOpenModelsWithSplit(True)
elapsedWithSplit = (time.time() - startWithSplit)

s1 = 'Time single-threaded: ' + repr(elapsedSingleThreaded) + ' ms'
s2 = 'Time sequencially: ' + repr(elapsedSequencially) + ' ms'
s3 = 'Time without split: ' + repr(elapsedWithoutSplit) + ' ms'
s4 = 'Time with split: ' + repr(elapsedWithSplit) + ' ms'
print s1
print s2
print s3
print s4

