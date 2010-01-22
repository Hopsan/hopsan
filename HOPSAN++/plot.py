#$Id$
# -*- coding: utf-8 -*-
import pylab;
import sys;

if len(sys.argv) < 2:
  filename = "output.txt"
else:
  filename = sys.argv[1]

print "Plotting " + filename

# Used for plotting Hydraulic nodes:
#pylab.plotfile(filename, (0,1,2), checkrows=0, delimiter=' ', names=['Time', "Flow", "Pressure"]);

# Used for plotting Signal nodes:
pylab.plotfile(filename, (0,1), checkrows=0, delimiter=' ', names=['Time', "Value"]);

# Used for plotting Mechanical nodes:
#pylab.plotfile(filename, (0,1,2,3), checkrows=0, delimiter=' ', names=['Time', "Velocity", "Force", "Position"]);

pylab.show();
