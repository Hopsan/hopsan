#!/usr/bin/python
# -*- coding: utf-8 -*-
#$Id$
import pylab;
import sys;

if len(sys.argv) < 2:
  filename = "output.txt"
else:
  filename = sys.argv[1]

print "Plotting " + filename

captions=[]
colums=()

#Decide what kind of node to plot
f = open(filename, 'r')
header = f.readline().rstrip('\n')
f.close()

if header == "NodeHydraulic":
  captions=['Time', "Flow", "Pressure"]
  colums=(0,1,2)
  print "Found HydraulicNode data"
elif header == "NodeSignal":
  captions=['Time', "Value"]
  colums=(0,1)
  print "Found SignalNode data"
elif header == "NodeMechanical":
  captions=['Time', "Velocity", "Force", "Position"]
  colums=(0,1,2,3)
  print "Found MechanicalNode data"
else:
  print "Unknown node type: " + header

#PLot the file, skipping header
pylab.plotfile(filename, cols=colums, skiprows=1, checkrows=0, delimiter=' ', names=captions);

pylab.show();
