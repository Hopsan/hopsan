############################################
## Complex Optimization Script for Hopsan ##
############################################

import random
from time import sleep
from types import FloatType



##### AUXILIARY FUNCTIONS #####

#Help functions for objectives

#Returns the first time where vector data is at value x
def firstTimeAt(data,time, x):
  for i in range(len(data)):
    if data[i] > x:
      return time[i]
  return time[i]

#Returns the maximum value of vector data
def maxValue(data):
  max = data[0]
  for i in range(len(data)):
    if data[i] > max:
      max = data[i]
  return max
  
#Returns the amount of overshoot above specified limit
def overShoot(data, limit):
  os=0;
  for i in range(len(data)):
    if data[i]-limit > os:
      os=data[i]-limit
  return os
   
def diffFromValueAtTime(data,time,x,t):
  for i in range(len(data)):
    if time[i]>=t:
      return abs(data[i]-x)	
   
#Auxiliary optimization funcions

#Returns the index of the maximum value in vector data
def indexOfMax(data):
  max = data[0]
  maxId=0
  for i in range(len(data)):
    if data[i] > max:
      max=data[i]
      maxId=i
  return maxId

#Sums the elements with index i in each subvector in a vector of vectors
def sum(vector, i):
  retval=0
  for j in range(len(vector)):
    retval = retval + vector[j][i]
  return retval
  
#Reflects the worst point in vector through the centroid of the remaining points, with reflection coefficient alpha
def reflectWorst(vector, worstId, alpha):
  n = len(vector)
  k = len(vector[0])
  x_w = vector[worstId]
  x_c = []
  for i in range(k):
    x_c.append(1.0/(n-1.0)*(sum(vector,i)-x_w[i]))
  x_new = []
  for i in range(k):
    x_new.append(max(minValues[i], min(maxValues[i], x_c[i]+alpha*(x_c[i]-x_w[i]))))
  vector[worstId] = x_new  



##### Simulation settings #####

timestep = 0.001
time=5
iterations=500
hopsan.setTimeStep(timestep)
hopsan.setStartTime(0)
hopsan.setFinishTime(time)
hopsan.turnOffProgressBar()
alpha=1.3



##### Optimization parameters #####

parameters = [[0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0],
              [0.0, 0.0]]
componentNames = ["GainP", "GainI"]   #Names of components where parameters are located
parameterNames = ["k", "k"]           #Names of parameters to optimize
minValues = [0.0, 0.0]                    #Minimum value for each parameter
maxValues = [0.01, 0.01]                #Maximum value for each parameter



##### Objective function #####

obj = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
def getObjective():
  e1=0.05
  e2=0.55
  e3=0.45
  data1=hopsan.component("Translational Mass").port("P2").getDataVector("Position")
  time1=hopsan.component("Translational Mass").port("P2").getTimeVector()
  data2=hopsan.component("Translational Mass").port("P2").getDataVector("Position")
  data3=hopsan.component("Translational Mass").port("P2").getDataVector("Position")
  time3=hopsan.component("Translational Mass").port("P2").getTimeVector()
  return e1*firstTimeAt(data1,time1,0.65) + e2*overShoot(data2, 0.7) + e3*diffFromValueAtTime(data3,time3,0.7,2.5) 



##### Starting points #####

for i in range(len(parameters)):
  for j in range(len(parameterNames)):
    parameters[i][j] = minValues[j]+(maxValues[j]-minValues[j])*random.random()



##### Execute optimization #####

hopsan.simulate()
hopsan.plot("Translational Mass","P2","Position")

previousWorstId = -1
for k in range(iterations):
#def iterate():
  for i in range(len(parameters)):
    for j in range(len(parameterNames)):
      hopsan.component(componentNames[j]).setParameter(parameterNames[j], parameters[i][j])
    hopsan.simulate()
    #hopsan.closeLastPlotWindow()
    #hopsan.plot("Translational Mass","P2","Position")
    #hopsan.refreshLastPlotWindow()
    obj[i] = getObjective()
  worstId = indexOfMax(obj)
  if worstId == previousWorstId:
    reflectWorst(parameters,worstId,alpha/2.0)
  else:
    reflectWorst(parameters,worstId,alpha)
  previousWorstId=worstId
  
