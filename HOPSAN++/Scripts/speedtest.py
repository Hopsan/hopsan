hopsan.turnOffProgressBar()

iterations = 20

hopsan.useSingleCore()
totaltime = 0
hopsan.simulate()
for i in range(0, iterations):
  hopsan.simulate()
  totaltime += hopsan.getSimulationTime()
avgtime = totaltime / iterations
hopsan.printInfo("Single-threaded simulation, " + str(iterations) + " iterations. Average time: " + str(avgtime))
print("Single-threaded simulation, " + str(iterations) + " iterations. Average time: " + str(avgtime))

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

#hopsan.useMultiCore()
#hopsan.setNumberOfThreads(4)
#totaltime = 0
#hopsan.simulate()
#for i in range(0, iterations):
  #hopsan.simulate()
  #totaltime += hopsan.getSimulationTime()
#avgtime = totaltime / iterations
#hopsan.printInfo("Multi-threaded simulation with 4 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))
#print("Multi-threaded simulation with 4 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))

#hopsan.useMultiCore()
#hopsan.setNumberOfThreads(8)
#totaltime = 0
#hopsan.simulate()
#for i in range(0, iterations):
  #hopsan.simulate()
  #totaltime += hopsan.getSimulationTime()
#avgtime = totaltime / iterations
#hopsan.printInfo("Multi-threaded simulation with 8 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))
#print("Multi-threaded simulation with 8 threads, " + str(iterations) + " iterations. Average time: " + str(avgtime))