filename="output.csv"
runs=100
maxload=300
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

  load = maxload*random.random()
  hopsan.setSystemParameter("load", load)
  hopsan.setSystemParameter("sigma", load*0.2)

  hopsan.turnOffProgressBar()
  
  if multicore:
    hopsan.useSingleCore()
    multicore=False
  else:
    hopsan.useMultiCore()
    multicore=True
    
  hopsan.simulate()
  time = hopsan.getSimulationTime()
  
  flops=load*48
  
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