#!/usr/bin/python
# Script to benchmark Hopsan Core simulation through the CLI based on different compiler options
# Author: Peter Nordin
# Date:  20150708
# $Id$

import shutil as sh
import sys
import os

class Results:
    def __init__(self):
        self.simtime = ''

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

    def printme(self):
        print('-----------------------------------------------------')
        print('TestCase:    '+self.id)
        print('Definitions: '+self.defines)
        print('Cflags:      '+self.cflags)
        print('Testcommand: '+self.testcommand)
        print("\n")

    def appendconfig(self, filepath):
        f = open(filepath, 'a+')
        if not f.closed:
            f.write('QMAKE_CXXFLAGS += '+self.cflags+"\n")
            f.write('DEFINES += '+self.defines+"\n")
            f.write("\n")
        f.close()


def createportdefcombinations(multidefs, singledefs):
    defcombs = list()

    # Append each def by itself
    defcombs += multidefs
    defcombs += singledefs

    # Append combinations
    for md in multidefs:
        for sd in singledefs:
            comb = md+' '+sd
            defcombs.append(comb)

    return defcombs

def parsetestresults(resultsfile):
    simtime = '-1'
    f = open(resultsfile, 'r')
    if not f.closed:
        for line in f.readlines():
            if line.startswith('SimulationTime'):
                fields = line.split(':')
                simtime = fields[2]
    return simtime


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print('Error: You must give at least two argument, the Hopsan root dir')
        exit()
    else:
        rootdir = sys.argv[1]

    # Find compile script
    compilescript = rootdir+'/Utilities/compileHopsanCore.sh'
    common_prf = rootdir+'Common.prf'
    if not os.path.isfile(compilescript):
        print('Can not find the Hopsan compile script')
        exit()
    if not os.path.isfile(common_prf):
        print('Can not find the Hopsan Common.prf')
        exit()



    # Definitions
    multiportdefs = list()
    portdefs = list()
    codedefs = list()

    # Compiler flags
    cflags = list()

    # Setup multiport defs
    multiportdefs.append('MULTIPORTNDPSTRUCT')
    #multiportdefs.append('MULTIPORTNDPSTRUCTMETHODS')
    #multiportdefs.append('MULTIPORTVALUESTRUCT')
    #multiportdefs.append('MULTIPORTREADWRITE')

    # Setup port defs
    #portdefs.append('PORTNDPSTRUCT')
    #portdefs.append('PORTNDPSTRUCTMETHODS')
    #portdefs.append('PORTVALUESTRUCT')
    #portdefs.append('PORTVALUES')
    #portdefs.append('PORTREADWRITE')

    # Setup compiler flags
    cflags.append('-O1')
    #cflags.append('-O2')
    #cflags.append('-O3')

    # Setup tests
    testcommands = list()
    testcommands.append(rootdir+'/bin/HopsanCLI -m /home/petno25/Dropbox/Work/Hopsan/Benchmark/multiportTest.hmf -s hmf')

    # Create definition combinations
    definitioncombs = createportdefcombinations(multiportdefs, portdefs)

    testcases = list()

    counter = 0
    for cf in cflags:
        for dc in definitioncombs:
            for cmd in testcommands:
                tc = TestCase(counter)
                tc.addCflags(cf)
                tc.addDefines(dc)
                tc.setTestcommand(cmd)
                testcases.append(tc)
                counter += 1

    # Backup Common.prf
    sh.copy2(common_prf, common_prf+'_org')

    # Run test cases
    for tc in testcases:
        # Restore configuration file
        sh.copy2(common_prf+'_org', common_prf)
        tc.printme()
        tc.appendconfig(common_prf)
        os.system(compilescript+' '+rootdir)
        # Run tests
        print('Running test: '+tc.testcommand)
        os.system(tc.testcommand+' > testresults')
        # Parse output
        st = parsetestresults('testresults')
        print('SimulationTime: '+st)
        tc.results.simulationtime = st

    # Restore configuration file
    sh.copy2(common_prf+'_org', common_prf)

    print('Done!')