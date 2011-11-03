###########################################################
## Auxiliary functions for Hopsan optimization algorithm ##
###########################################################

import random

      
##### Auxiliary optimization funcions #####

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
def sum(vector,i):
  retval=0
  for j in range(len(vector)):
    retval = retval + vector[j][i]
  return retval
  
def minPar(vector, i):
  min=vector[0][i]
  for j in range(len(vector)):
    if vector[j][i] < min:
      min = vector[j][i]
  return min
  
def maxPar(vector, i):
  max=vector[0][i]
  for j in range(len(vector)):
    if vector[j][i] > max:
      max = vector[j][i]
  return max
  
#Reflects the worst point in vector through the centroid of the remaining points, with reflection coefficient alpha
def reflectWorst(vector,worstId,alpha,minValues,maxValues,beta):
  n = len(vector)
  k = len(vector[0])
  x_w = vector[worstId]
  x_c = []
  for i in range(k):
    x_c.append(1.0/(n-1.0)*(sum(vector,i)-x_w[i]))
  x_new = []
  for i in range(k):
    rand = beta*(maxPar(vector,i)-minPar(vector,i))*(random.random()-0.5)
    x_new.append(max(minValues[i], min(maxValues[i], x_c[i]+alpha*(x_c[i]-x_w[i])+rand)))
  vector[worstId] = x_new 
