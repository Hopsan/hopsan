#!/usr/bin/env python
__author__ = 'Peter Nordin'
__email__ = "peter.nordin@liu.se"

import math

class Results:
    def __init__(self):
        self.inittimes = list()
        self.simtimes = list()
        self.avginittime = 0
        self.avgsimtime = 0
        self.stddevinittime = 0
        self.stddevsimtime = 0

    def calcAverageStdDev(self):
        sum = 0
        for it in self.inittimes:
            sum += float(it)
        self.avginittime = sum/len(self.inittimes)

        sum = 0
        my = self.avginittime
        for it in self.inittimes:
            sum += math.pow(float(it)-my, 2)
        self.stddevinittime = math.sqrt(sum/len(self.inittimes))

        sum = 0
        for st in self.simtimes:
            sum += float(st)
        self.avgsimtime = sum/len(self.simtimes)

        sum = 0
        my = self.avgsimtime
        for st in self.simtimes:
            sum += math.pow(float(st)-my, 2)
        self.stddevsimtime = math.sqrt(sum/len(self.simtimes))

    def add(self, inittime, simtime):
        self.inittimes.append(str(inittime))
        self.simtimes.append(str(simtime))
        self.calcAverageStdDev()

    def append(self, other):
        self.inittimes += other.inittimes
        self.simtimes += other.simtimes
        self.calcAverageStdDev()

    def numSamples(self):
        return len(self.simtimes)

    def print2text(self, full=False):
        text = str()
        if full:
            text += 'InitializationTimes:'
            for it in self.inittimes:
                text += ' '+it
            text += '\n'
            text += 'SimulationTimes:'
            for st in self.simtimes:
                text += ' '+st
            text += '\n'
        text += 'Average Initialization Time: '+str(self.avginittime)+' '+str(self.stddevinittime)+'\n'
        text += 'Average Simulation Time: '+str(self.avgsimtime)+' '+str(self.stddevsimtime)+'\n'
        return text

    def printme(self):
        print(self.print2text())


class TestCase:
    def __init__(self, id):
        self.id = str(id)
        self.cflags = ''
        self.defines = ''
        self.testcommand = ''
        self.results = Results()

    def addCflags(self, lst):
        for item in lst:
            self.cflags += item

    def addDefines(self, lst):
        for item in lst:
            self.defines += item

    def setTestcommand(self, cmd):
        self.testcommand = cmd

    def print2text(self):
        text = str()
        text += '-----------------------------------------------------\n'
        text += 'TestCase:    '+self.id+'\n'
        text += 'Definitions: '+self.defines+'\n'
        text += 'Cflags:      '+self.cflags+'\n'
        text += 'Testcommand: '+self.testcommand+'\n'
        text += self.results.print2text(full=True)
        text += '\n'
        return text

    def printme(self):
        print(self.print2text())
        print('\n')

    def appendconfig(self, filepath):
        f = open(filepath, 'a+')
        if not f.closed:
            f.write('\n')
            f.write(r'#This line and the lines below are not supposed to be here, REMOVE THEM'+'\n')
            f.write('QMAKE_CXXFLAGS += '+self.cflags+"\n")
            f.write('DEFINES += '+self.defines+"\n")
            f.write('\n')
        f.close()
