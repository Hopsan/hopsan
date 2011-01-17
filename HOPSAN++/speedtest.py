hopsan.turnOffProgressBar()

iterations = 100

hopsan.deactivateMultiCore()
totaltime = 0
for i in range(0, iterations):
	hopsan.simulate()
	totaltime += hopsan.getLastSimulationTime()
singleavgtime = totaltime / iterations
hopsan.printInfo("Ran " + str(iterations) + " single-threaded iterations, average time: " + str(singleavgtime))

hopsan.activateMultiCore()
totaltime = 0
for i in range(0, iterations):
	hopsan.simulate()
	totaltime += hopsan.getLastSimulationTime()
multiavgtime = totaltime / iterations
hopsan.printInfo("Ran " + str(iterations) + " multi-threaded iterations, average time: " + str(multiavgtime))