#!/usr/bin/python
# Script to evaluate benchmark results created by benchmarkHopsanCli.py
# Author: Peter Nordin
# Date:  20150714
# $Id: evalBenchmark.py 8194 2015-07-08 14:34:05Z petno25 $

import matplotlib.pyplot as plt
import sys
import pickle
from benchmarkClasses.classes import *


def addtolist(alist, value):
    out = list()
    for item in alist:
        item += value
        out.append(item)
    return out


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print('Error: You must give at least one argument, the pickled results file to evaluate')
        exit()
    else:
        pickledresultsfile = sys.argv[1]

    f = open(pickledresultsfile, 'r')
    if f.closed:
        print('Error: Could not open file: '+pickledresultsfile)
        exit()

    tclist = pickle.load(f)
    f.close()

    # Generate plots
    barwidth=0.75
    ctr = 0
    xvals=list()
    xlabels=list()
    inittimes=list()
    inittimedevs=list()
    simtimes=list()
    simtimedevs=list()
    for tc in tclist:
        #tc = TestCase(tc)
        tc.printme()
        xvals.append(ctr)
        xlabels.append('TC'+str(tc.id))
        inittimes.append(tc.results.avginittime)
        inittimedevs.append(tc.results.stddevinittime)
        simtimes.append(tc.results.avgsimtime)
        simtimedevs.append(tc.results.stddevsimtime)
        ctr += 1
    #print(xvals)
    #print(simtimes)
    #print(simtimedevs)

    fig, ax = plt.subplots()
    ax.bar(xvals, inittimes, barwidth, color='y', yerr=inittimedevs)
    ax.set_ylabel('Initialization Time')
    ax.set_title('Initialization Time for Each Tested Configuration')
    ax.set_xticks(addtolist(xvals, barwidth/2))
    ax.set_xticklabels(xlabels)

    fig, ax = plt.subplots()
    ax.bar(xvals, simtimes, barwidth, color='y', yerr=simtimedevs)
    ax.set_ylabel('Simulation Time')
    ax.set_title('Simulation Time for Each Tested Configuration')
    ax.set_xticks(addtolist(xvals, barwidth/2))
    ax.set_xticklabels(xlabels)

    # Now figure out the best speed by sorting the list
    sortedidx = sorted(range(len(simtimes)), key=lambda k: simtimes[k])

    for idx in sortedidx:
        print('Id: '+xlabels[idx]+' Simtime: '+str(simtimes[idx]))

    plt.show()
