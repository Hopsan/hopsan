hopsan.turnOffProgressBar()

iterations = 40
pressure = 1e7

hopsan.deactivateMultiCore()
totaltime = 0
hopsan.simulate()
hopsan.plot("Translational Mass", "P2", "Position")
for i in range(0, iterations):
  pressure += 5e5
  hopsan.setParameter("Prescribed Pressure C", "P", pressure)
  hopsan.simulate()
#  if i>10:
#    hopsan.discardOldestPlotGeneration(0)
  totaltime += hopsan.getLastSimulationTime()
singleavgtime = totaltime / iterations
hopsan.printInfo("Ran " + str(iterations) + " single-threaded iterations, average time: " + str(singleavgtime))
