#!/usr/bin/python
# Script to benchmark Hopsan Core simulation through the CLI based on different compiler options
# Author: Peter Nordin
# Date:  20150708
# $Id$

import shutil as sh
import sys
import os
import pickle
from benchmarkClasses.classes import *


def createportdefcombinations(multidefs, singledefs):
    defcombs = list()

    # Append each def by itself even if it is empty (use default in Hopsan)
    defcombs += multidefs
    defcombs += singledefs

    # Append combinations
    for md in multidefs:
        if md != '':
            for sd in singledefs:
                if sd != '':
                    comb = md+' '+sd
                    defcombs.append(comb)

    return defcombs


def parsetestresults(resultsfile):
    it = ''
    st = ''
    f = open(resultsfile, 'r')
    if not f.closed:
        for line in f.readlines():
            if line.startswith('InitializeTime'):
                fields = line.split(':')
                if len(fields) > 1:
                    it = fields[1].strip()
            elif line.startswith('SimulationTime'):
                fields = line.split(':')
                if len(fields) > 1:
                    st = fields[1].strip()
    else:
        print('Error: The file '+resultsfile+' could not be opened')
    re = Results()
    re.add(it, st)
    return re


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print('Error: You must give at least one argument, the Hopsan root dir')
        exit()
    else:
        rootdir = sys.argv[1]

    print('Using root dir: '+rootdir)

    # Find necessary files
    clipath = os.path.join(rootdir, 'bin/hopsancli')
    compilescript = os.path.join(rootdir, 'Utilities/compileHopsanCore.sh')
    common_prf = os.path.join(rootdir, 'Common.prf')
    if not os.path.isfile(compilescript):
        print('Can not find the Hopsan compile script')
        exit()
    if not os.path.isfile(common_prf):
        print('Can not find the Hopsan Common.prf')
        exit()
    if not os.path.isfile(clipath):
        print('Can not find the HopsanCLI program')
        exit()

    # Setup multiport defs
    multiportdefs = list()
    multiportdefs.append('')
    multiportdefs.append('MULTIPORTNDPSTRUCT')
    multiportdefs.append('MULTIPORTNDPSTRUCTMETHODS')
    multiportdefs.append('MULTIPORTVALUESTRUCT')
    multiportdefs.append('MULTIPORTREADWRITE')

    # Setup port defs
    portdefs = list()
    #portdefs.append('PORTNDPSTRUCT')
    #portdefs.append('PORTNDPSTRUCTMETHODS')
    #portdefs.append('PORTVALUESTRUCT')
    #portdefs.append('PORTVALUES')
    #portdefs.append('PORTREADWRITE')

    # Setup code defs
    codedefs = list()

    # Setup compiler flags
    cflags = list()
    cflags.append('')
    cflags.append('-O1')
    cflags.append('-O2')
    cflags.append('-O3')

    # Setup tests
    clipath = os.path.join(rootdir, 'bin/hopsancli')
    testcommands = list()
    testcommands.append(clipath+r' -m $HOME/Dropbox/Work/Hopsan/Benchmark/multiportTest.hmf -s hmf')

    # Setup variables
    numtestitterations = 10

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
        # tc.printme()
        tc.appendconfig(common_prf)
        os.system(compilescript+' '+rootdir)
        # Run tests
        for ctr in range(numtestitterations):
            print('Running test itter('+str(ctr)+'): '+tc.testcommand)
            os.system(tc.testcommand+' > testresults')
            # Parse output
            res = parsetestresults('testresults')
            res.printme()
            tc.results.append(res)

    # Print results
    print('\n')
    print('Results:')
    bresfile = open('benchmarkResults', 'w+')
    bresfilepickle = open('benchmarkResultsPickle', 'w+')
    pickle.dump(testcases, bresfilepickle)
    bresfilepickle.close()
    for tc in testcases:
        tc.printme()
        if not bresfile.closed:
            text = tc.print2text()
            bresfile.write(text)
    bresfile.close()

    # Restore configuration file
    sh.copy2(common_prf+'_org', common_prf)

    print('Done!')
