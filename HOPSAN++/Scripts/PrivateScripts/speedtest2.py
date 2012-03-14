hopsan.turnOffProgressBar()

hopsan.useMultiCore()
hopsan.setNumberOfThreads(2)
totaltime = 0
hopsan.simulate()
for i in range(0, iterations):
  hopsan.simulate()
  totaltime += hopsan.getSimulationTime()
avgtime = totaltime / iterations
hopsan.printInfo("Multi-threaded simulation with 2 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))
print("Multi-threaded simulation with 2 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))