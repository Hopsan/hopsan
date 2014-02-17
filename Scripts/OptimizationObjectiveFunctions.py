###########################################################
## Objective functions for Hopsan optimization algorithm ##
###########################################################

## Functions must also be described in OptimizationObjectiveFunctions.xml to be used in Hopsan

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
  
#Returns the minimum value of vector data  
def minValue(data):
  min = data[0]
  for i in range(len(data)):
    if data[i] < min:
      min = data[i]
  return min
  
#Returns the amount of overshoot above specified limit
def overShoot(data, limit):
  os=0;
  for i in range(len(data)):
    if data[i]-limit > os:
      os=data[i]-limit
  return os
   
#Returns the absolute difference in vector data from specified value x at time t
def diffFromValueAtTime(data,time,x,t):
  for i in range(len(data)):
    if time[i]>=t:
      return abs(data[i]-x)	

#Returns the average absolute value
def averageAbsoluteValue(data):
  total=0
  for i in range(len(data)):
    total += abs(data[i])
  return total/len(data)
      
#Returns the average absolute difference between vectors data1 and data2
def averageAbsoluteDifference(data1, data2):
  total=0
  for i in range(len(data1)):
    d1 = data1[i]
    d2 = data2[i]
    total = total+abs(d1-d2)
  return total/len(data1)
   
#Returns the average absolute difference between vectors data1 and data2
def averageAbsoluteDifferenceSquared(data1, data2):
  total=0
  for i in range(len(data1)):
    d1 = data1[i]
    d2 = data2[i]
    total = total+abs(d1-d2)**2
  return total/len(data1)
