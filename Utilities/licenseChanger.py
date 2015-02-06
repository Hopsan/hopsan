#!/usr/bin/python
# Script to change the license header of code files
# Author: Peter Nordin
# Date:  20150206
# $Id$

import os
import sys

def findFiles(rootDir, suffixes, excludeDirs):
    files = list()
    for dirpath, dirnames, filenames in os.walk(rootDir):
        print(dirpath)
        enterDir = True
        for d in excludeDirs:
            if d in dirpath:
                enterDir=False
        if enterDir:
            for filename in filenames:
                name, ext = os.path.splitext(filename)
                if ext in suffixes:
                    filepath = os.path.join(dirpath,filename)
                    print(filepath)
                    files.append(filepath)
    return files

def checkLicense(filename,  licenseTemplate):
    noLicense=None
    sameLicense=None
    otherLicense=None
    file = open(filename,  'rb')
    if not file.closed:
        # Read first char
        fileLicense =r'';
        str = file.read(2);
        if str==r'/*':
            c1=r''
            c2=file.read(1)
            while not (c1+c2)  == r'*/':
                #Read until */
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
                print("===== Other License =====")
                print(filename)
                print(licenseTemplate)
                print(fileLicense)
                print("=========================")
                print('\n')
                otherLicense=filename
        else:
            noLicense=filename
    file.close()
    return noLicense,  sameLicense,  otherLicense

def replaceLicense(filename,  newLicense):
    file = open(filename,  'rb+')
    if not file.closed:
        c1=r''
        c2=file.read(1)
        while not (c1+c2)  == r'*/':
            #Read until */
            c1=c2
            c2=file.read(1)
        file.read(1) #Gobble newline (at-least on linux)
        contents = file.read()
        file.close()
        file = open(filename,  'wb')
        file.write(newLicense)
        file.write(contents)
    file.close()
    
def setLicense(filename,  license):
    file = open(filename,  'rb+')
    if not file.closed:
        contents = file.read()
        file.close()
        file = open(filename,  'wb')
        file.write(license)
        file.write('\n')
        file.write(contents)
    file.close()    

def main(rootDir,  setNew):
    suffixes = ('.cc', '.cpp', '.h', '.hpp')
    excludeDirs =  ('Dependencies', )
    
    nlf = open('./newLicense')
    newLicense=nlf.read()
    nlf.close()
    print(newLicense)
    
    filesWithout = list()
    filesWithSame = list()
    filesWithOther = list()
    
    files = findFiles(rootDir, suffixes, excludeDirs)
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
    if len(sys.argv) < 2:
        print('Error: You must give at least one argument, the root dir')
        exit()
    else:
        rootDir = sys.argv[1]
    
    setNew = False
    if len(sys.argv) == 3:
        if sys.argv[2] == r'set':
            setNew = True
    
    main(rootDir,  setNew)
