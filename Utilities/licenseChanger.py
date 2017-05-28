#!/usr/bin/python
# Script to change the license header of code files
# Author: Peter Nordin
# $Id$

import os
import sys
import argparse

verbose = False

def findFiles(rootDir, suffixes, excludeDirs):
    files = list()
    for dirpath, dirnames, filenames in os.walk(rootDir):
        #print(dirpath)
        enterDir = True
        for d in excludeDirs:
            if d in dirpath:
                print('Excluding: '+dirpath)
                enterDir=False
        if enterDir:
            for filename in filenames:
                name, ext = os.path.splitext(filename)
                if ext in suffixes:
                    filepath = os.path.join(dirpath,filename)
                    #print(filepath)
                    files.append(filepath)
    return files

def checkLicense(filename,  licenseTemplate):
    noLicense=None
    sameLicense=None
    otherLicense=None
    with open(filename,  'r') as file:
         # Read first char
        fileLicense = '';
        str = file.read(2);
        #print(str)
        if str == r'/*':
            c1=r''
            c2=file.read(1)
            while not (c1+c2)  == r'*/':
                # Read until */
                c1=c2
                c2=file.read(1)
            file.read(1) #Gobble newline, at least on linux
            e = file.tell()
            file.seek(0, 0)
            fileLicense = file.read(e)
            # Compare license
            if fileLicense == licenseTemplate:
                sameLicense=filename
            else:
                if verbose:
                    print("===== Other License =====")
                    print(filename)
                    print(licenseTemplate)
                    print(fileLicense)
                    print("=========================")
                    print('\n')
                otherLicense=filename
        else:
            noLicense=filename

    return noLicense,  sameLicense,  otherLicense

def replaceLicense(filename,  new_license):
    with open(filename,  'r+') as file:
        c1=r''
        c2=file.read(1)
        while not (c1+c2)  == r'*/':
            # Read until */
            c1=c2
            c2=file.read(1)
        file.read(1) #Gobble newline (at-least on linux)
        contents = file.read()

    with open(filename,  'w') as file:
        file.write(new_license)
        file.write(contents)
    
def setLicense(filename,  new_license):
    with open(filename,  'r+') as file:
        contents = file.read()

    with open(filename,  'w') as file:
        file.write(new_license)
        file.write('\n')
        file.write(contents)

def main(rootDir, licFile, exclude, setNew):
    suffixes = ('.c', '.cc', '.cpp', '.cci', '.h', '.hpp')

    with open(licFile) as f:
        newLicense = f.read()
    #print(newLicense)

    filesWithout = list()
    filesWithSame = list()
    filesWithOther = list()
 
    files = findFiles(rootDir, suffixes, exclude)
    for file in files:
        no,  same,  other = checkLicense(file,  newLicense)
        if no:
            filesWithout.append(no)
        if same:
            filesWithSame.append(same)
        if other:
            filesWithOther.append(other)

    print(r'Files without license header')
    print( filesWithout)
    print('\n')
    print(r'Files with other license header')
    print(filesWithOther)
    print('\n')
    print(r'Num files with same license header')
    print(len(filesWithSame))
    print('\n')

    if setNew:
        for file in filesWithout:
            setLicense(file,  newLicense)

        for file in filesWithOther:
            replaceLicense(file,  newLicense)

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Change license in Hopsan source code files.')
    parser.add_argument('rootDir', help='directory')
    parser.add_argument('license', help='License file')
    parser.add_argument('-e', '--exclude', action='append', nargs='+', help='Exclude')
    parser.add_argument('--set', help='Replace the licens', action='store_true', default=False)
    parser.add_argument('-v', '--verbose', action='store_true', default=False)

    args = vars(parser.parse_args())
    #print(args)
    verbose = args['verbose']
    exclude = list()
    if args['exclude'] is not None:
        exclude = [e for exclist in args['exclude'] for e in exclist]

    #print(args['rootDir'])
    #print(args['license'])
    #print(args['set'])
    #print(args['exclude'])
    #print(exclude)
    main(args['rootDir'], args['license'], exclude, args['set'])
