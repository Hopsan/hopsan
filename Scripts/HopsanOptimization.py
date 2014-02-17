###########################################################
## Auxiliary functions for Hopsan optimization algorithm ##
###########################################################

import random
import math
import sys
      
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

#Returns the index of the maximum value in vector data
def indexOfMin(data):
  min = data[0]
  minId=0
  for i in range(len(data)):
    if data[i] < min:
      min=data[i]
      minId=i
  return minId

#Returns the index of the maximum value in vector data
def indexOfMaxN(data, N):
  maxIds = []
  for t in range(N):
    max = -sys.maxint - 1
    maxId=0
    for i in range(len(data)):
      if (data[i] > max) and (i not in maxIds):
        max=data[i]
        maxId=i
    maxIds.append(maxId)
  return maxIds

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

#Reflects the N worst points in vector through the centroid of the remaining points, with reflection coefficient alpha
def reflectWorstN(vector,worstIds,previousWorstIds,alpha,minValues,maxValues,beta):
  n = len(vector)		#Number of points
  k = len(vector[0])		#Number of parameters in each point
  
  x_c = []			#Centroid of remaining points	
  for i in range(k):
    sum = 0
    for j in range(n):
      if j not in worstIds:
        sum = sum+vector[j][i]
    x_c.append(1.0/(n-len(worstIds))*sum)

  for w in range(len(worstIds)):
    worstId = worstIds[w]
    x_w = vector[worstId]
    x_new = []
    for i in range(k):
      rand = beta*(maxPar(vector,i)-minPar(vector,i))*(random.random()-0.5)
      x_new.append(max(minValues[i], min(maxValues[i], x_c[i]+alpha*(x_c[i]-x_w[i])+rand)))          
    vector[worstId] = x_new 



def contract(vector,id,worstIds,minValues,maxValues):
  n = len(vector)  
  k = len(vector[0])  

  x_c = []			#Centroid of remaining points	
  for i in range(k):
    sum = 0
    for j in range(n):
      if j not in worstIds:
        sum = sum+vector[j][i]
    x_c.append(1.0/(n-len(worstIds))*sum)

  x_w = vector[id]
  x_new = []
  for i in range(k):
    x_new.append(max(minValues[i], min(maxValues[i], x_c[i]+0.5*(x_c[i]-x_w[i]))))     
  vector[id] = x_new 


def toLogSpace(vector):
  n = len(vector)
  for i in range(n):
    vector[i] = math.log10(vector[i])

def toLinearSpace(vector):
  n = len(vector)
  for i in range(n):
    vector[i] = 10**vector[i]

#Transforms a vector of points from linear to logarithmic space
def toLogSpace2(vector):
  n = len(vector)	        #Number of points
  k = len(vector[0])		#Number of parameters in each point
  for i in range(n):
    for j in range(k):
      vector[i][j] = math.log10(vector[i][j])

#Transforms a vector of points from logarithmic to linear space
def toLinearSpace2(vector):
  n = len(vector)	        #Number of points
  k = len(vector[0])		#Number of parameters in each point
  for i in range(n):
    for j in range(k):
      vector[i][j] = 10**vector[i][j]
