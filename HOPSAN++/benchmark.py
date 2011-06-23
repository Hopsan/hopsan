filename="output.csv"
runs=1000
maxload=1000
balance=0.95
time=10
timestep=0.001

import random


hopsan.setTimeStep(timestep)
hopsan.setStartTime(0)
hopsan.setFinishTime(time)

sfile = open("output_singlethreaded.xml","w")
mfile = open("output_multithreaded.xml","w")

sfile.write("<hopsanbenchmarkdata>\n")
mfile.write("<hopsanbenchmarkdata>\n")

multicore=True

for i in range(runs):

  load1 = maxload*random.random()
  load2 = load1 + 2*(1-balance)*load1*(random.random()-0.5)
  load3 = load1 + 2*(1-balance)*load1*(random.random()-0.5)
  load4 = load1 + 2*(1-balance)*load1*(random.random()-0.5)

  flops=load1*12 + load2*12 + load3*12 + load4*12

  hopsan.setParameter("Source1", "y", load1)
  hopsan.setParameter("Source2", "y", load2)
  hopsan.setParameter("Source3", "y", load3)
  hopsan.setParameter("Source4", "y", load4)

  if multicore:
    hopsan.useSingleCore()
    multicore=False
  else:
    hopsan.useMultiCore()
    multicore=True
    
  hopsan.simulate()
  time = hopsan.getSimulationTime()
  
  if multicore:
    mfile.write("  <measurement>\n")
    mfile.write("    <load>")
    mfile.write(str(flops))
    mfile.write("</load>\n")
    mfile.write("    <time>")
    mfile.write(str(time))
    mfile.write("</time>\n")
    mfile.write("  </measurement>\n")
  else:
    sfile.write("  <measurement>\n")
    sfile.write("    <load>")
    sfile.write(str(flops))
    sfile.write("</load>\n")
    sfile.write("    <time>")
    sfile.write(str(time))
    sfile.write("</time>\n")
    sfile.write("  </measurement>\n")
  
sfile.write("</hopsanbenchmarkdata>")
mfile.write("</hopsanbenchmarkdata>")

sfile.close()
mfile.close()