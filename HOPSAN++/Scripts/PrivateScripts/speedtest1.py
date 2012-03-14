hopsan.turnOffProgressBar()

hopsan.useSingleCore()
totaltime = 0
hopsan.simulate()
for i in range(0, iterations):
  hopsan.simulate()
  totaltime += hopsan.getSimulationTime()
avgtime = totaltime / iterations
hopsan.printInfo("Single-threaded simulation, " + str(iterations) + " iterations. Average time: " + str(avgtime))
print("Single-threaded simulation, " + str(iterations) + " iterations. Average time: " + str(avgtime))