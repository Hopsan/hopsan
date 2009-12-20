# -*- coding: utf-8 -*-
import pylab;
import sys;

if len(sys.argv) < 2:
  filename = "output.txt"
else:
  filename = sys.argv[1]
print "Plotting " + filename
pylab.plotfile(filename, (0,1,2), checkrows=0, delimiter=' ');
pylab.show();
