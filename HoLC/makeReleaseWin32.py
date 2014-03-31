# $Id: makeReleaseWin32.py 6587 2014-02-17 09:12:10Z petno25 $

import subprocess
import os
import ctypes
import stat
from time import sleep

version='1.0'
tempDir=r'C:\temp_release_holc'
dependecyBinFile=r'holc_bincontents_Qt485_MinGW44.7z'

# Libs
qtlibDirList = [r'C:\Qt\4.8.5']
qtlib64DirList = [r'C:\Qt\Qt64-4.8.5']

# Compilers and build tools
qtcreatorDirList = [r'C:\Qt\qtcreator-2.8.1']
mingwDirList = [r'C:\Qt\MinGW-gcc440_1\mingw\bin', r'C:\Qt\mingw\bin', r'C:\mingw\bin']
mingw64DirList = [r'C:\Qt\mingw64\bin']

# Remember current working dir
holcDir=os.getcwd()

STD_OUTPUT_HANDLE= -11

escape_dict={'\a':r'\a',
           '\b':r'\b',
           '\c':r'\c',
           '\f':r'\f',
           '\n':r'\n',
           '\r':r'\r',
           '\t':r'\t',
           '\v':r'\v',
           '\'':r'\'',
           '\"':r'\"',
           '\0':r'\0',
           '\1':r'\1',
           '\2':r'\2',
           '\3':r'\3',
           '\4':r'\4',
           '\5':r'\5',
           '\6':r'\6',
           '\7':r'\7',
           '\8':r'\8',
           '\9':r'\9'}

def makeRaw(text):
    """Returns a raw string representation of text"""
    new_string=''
    for char in text:
        try: new_string+=escape_dict[char]
        except KeyError: new_string+=char
    return new_string

def quotePath(path):
    """Appends quotes around string if quotes are not already present"""
    if path[0] != r'"':
        path = r'"'+path
    if path[-1] != r'"':
        path = path+r'"'
    return path

class bcolors:
  WHITE = 0x07
  GREEN= 0x0A
  RED = 0x0C
  YELLOW = 0x0E
  BLUE = 0x0B

std_out_handle = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)

def setColor(color, handle=std_out_handle):
    bool = ctypes.windll.kernel32.SetConsoleTextAttribute(handle, color)
    return bool

setColor(bcolors.WHITE)

def runCmd(cmd):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    return process.communicate()

def printSuccess(text):
    setColor(bcolors.GREEN)
    print "Success: " + text
    setColor(bcolors.WHITE)

def printWarning(text):
    setColor(bcolors.YELLOW)
    print "Warning: " + text
    setColor(bcolors.WHITE)

def printError(text):
    setColor(bcolors.RED)
    print "Error: " + text
    setColor(bcolors.WHITE)

def printDebug(text):
    setColor(bcolors.BLUE)
    print "Debug: " + text
    setColor(bcolors.WHITE)

def pathExists(path, failMsg="", okMsg=""):
    # Add \ at end if not already present, else the dirname function below will take parent dir
    if path[-1] != "\\":
        path = path+"\\"
        
    if os.path.exists(os.path.dirname(path)):
        if okMsg!="":
            printSuccess(okMsg)
        return True
    else:
        if failMsg!="":
            printError(failMsg)
        return False

def fileExists(fileName):
    return os.path.isfile(fileName)
    
def selectPathFromList(list, failMsg, sucessMsg):
    selected=""
    for item in list:
        if pathExists(item):
            selected = item
    if selected=="":
        printError(failMsg)
    else:
        printSuccess(sucessMsg)
    return selected

def askYesNoQuestion(msg):
    # Returns true on yes
    while(True):
        ans = raw_input(msg)
        if ans=="y":
            return True
        elif ans=="n":
            return False

def setReadOnlyForAllFilesInDir(rootDir):
    for dirpath, dirnames, filenames in os.walk(rootDir,topdown=True):
        print "Setting files read-only in directory: "+ dirpath
        for filename in filenames:
            #print "Setting files read-only: "+ os.path.join(dirpath, filename)
            os.chmod(os.path.join(dirpath, filename), stat.S_IREAD)


def svnExport(src, dst):
    print "Exporting from "+quotePath(src)+" to "+quotePath(dst)
    os.system("svn export -q "+quotePath(src)+" "+quotePath(dst))

def callXcopy(src, dst):
    #print "xcopy "+quotePath(src)+" "+quotePath(dst)
    os.system("xcopy "+quotePath(src)+" "+quotePath(dst)+" /s /y")

def callCopyFile(src, dst):
    #print "copy "+quotePath(src)+" "+quotePath(dst)
    os.system("copy "+quotePath(src)+" "+quotePath(dst)+" /y")

def callMkdir(dst):
    #print "mkdir "+quotePath(dst)
    os.system("mkdir "+quotePath(dst))

def callRd(tgt):
    #print "rd "+quotePath(tgt)
    os.system("rd "+quotePath(tgt)+" /s /q")

def callEXE(cmd, args):
    #print "callEXE: " + quotePath(cmd)+r' '+args
    if fileExists(cmd):
        os.system(r'"'+quotePath(cmd)+r' '+args+r'"')
    else:
        printError(cmd+r' Does not exist!')

def callDel(tgt):
    #print "del "+quotePath(tgt)
    os.system("del "+quotePath(tgt))

def callMove(src, dst):
    print "move "+quotePath(src)+" "+quotePath(dst)
    os.system("move "+quotePath(src)+" "+quotePath(dst))

def call7z(args):
    callEXE(holcDir+r'\..\ThirdParty\7z\7z.exe', args)

def callSed(sedCommand):
    callEXE(holcDir+r'\ThirdParty\sed-4.2.1\sed.exe', sedCommand);
    
def verifyPaths():
    print "Verifying and selecting path variables..."
	
    global innoDir
    global qtDir
    global jomDir
    global qmakeDir
    global mingwDir

    isOk = True
   
    qtlibsdirs=qtlibDirList
    mingwdirs=mingwDirList
   
    qtlibsdirs=qtlibDirList
    
    #Check if Qt path exists
    qtDir=selectPathFromList(qtlibsdirs, "Qt libs could not be found in one of the expected locations.", "Found Qt libs!")
    if qtDir == "":
        isOk = False

    #Check if qtcreator path exist  
    creatorDir=selectPathFromList(qtcreatorDirList, "Qt Creator could not be found in one of the expected locations.", "Found Qt Creator!")
    if creatorDir == "":
        isOk = False
        
    jomDir=creatorDir+r'\bin'
    qmakeDir=qtDir+r'\bin'

    mingwDir=selectPathFromList(mingwdirs, "MinGW could not be found in one of the expected locations.", "Found MinGW!")
    if mingwDir == "":
        isOk = False

	#Make sure the 3d party dependency file exists
    if not pathExists(dependecyBinFile, "The "+ dependecyBinFile + " file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan", "Found dependency binary files!"):
        isOk = False
		
    if isOk:
        printSuccess("Verification of path variables.")

    return isOk


  
def buildRelease():
    #========================================================
    # BUILD WITH MINGW32
    #========================================================
    print "Compiling with MinGW"

    #Create clean build directory
    holcBuildDir = holcDir+r'\HoLC_bd'
    callRd(holcBuildDir)
    callMkdir(holcBuildDir)
    
    # Generate compile script, setup compiler and compile
    mkspec="win32-g++"
    f = open('compileWithMinGW.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'SET PATH='+mingwDir+r';'+qmakeDir+r';%PATH%'+"\n")
    f.write(r'mingw32-make.exe clean'+"\n")
    f.write(r'qmake.exe '+quotePath(holcDir+r'\HoLC.pro')+r' -r -spec '+mkspec+r' "CONFIG+=release"'+"\n")
    f.write(r'mingw32-make.exe'+"\n")
    #f.write("pause\n")
    f.close();
    
    os.chdir(holcBuildDir)    
    os.system(r'..\compileWithMinGW.bat')

    if not fileExists(holcDir+r'\bin\HoLC.exe'):
        printError("Failed to build HoLC with MinGW.")
        return False
  
    printSuccess("Compilation using MinGW")

    return True
    
    
def copyFiles():
    global dodevrelease

    # Make sure we are in the HoLC root
    os.chdir(holcDir)
    
    #Create a temporary release directory
    callMkdir(tempDir)
    if not pathExists(tempDir):
        printError("Failed to create temporary directory")
        return False

    # Create directories    
    callMkdir(tempDir+r'\bin')

    # Common export dirs
    tempDirBin=tempDir+r'\bin'
    
    #Copy "bin" folder to temporary directory
    callXcopy(r'bin\*.*', tempDirBin)

    #Export "HoLC" SVN directory to "include" in temporary directory
    svnExport("HoLC", tempDir+r'\HoLC')
 
    #Copy "holcdefaults" file to temporary directory
    svnExport("holcdefaults", tempDir+r'\holcdefaults')

    #Set all files to read-only
    setReadOnlyForAllFilesInDir(tempDir)

    return True
    

def createCleanOutputDirectory():
    global holcDirOutput
    """Try to remove and recreate the output directory"""
    holcDirOutput=holcDir+r'\output'
    
    #Clear old output folder
    callRd(holcDirOutput)
    if pathExists(holcDirOutput):
        printWarning("Unable to clear old output folder.")
        if not askYesNoQuestion("Continue? (y/n): "):
            return False

    #Create new output folder
    callMkdir(holcDirOutput)
    if not pathExists(holcDirOutput):
        printError("Failed to create output folder.")
        return False

    return True

def renameBinFolder():
    # Move the bin folder to temp storage to avoid packagin dev junk into release
    if pathExists(holcDir+r'\bin'):
        callMove(holcDir+r'\bin', holcDir+r'\bin_build_backup')
        sleep(1)
    if pathExists(holcDir+r'\bin'):
        printError("Could not move the bin folder to temporary backup before build.")
        return False
        
    # Create clean bin directory
    callMkdir(holcDir+r'\bin')
    return True
    

def createInstallFiles():

    # Make sure we are in the HoLC root
    os.chdir(holcDir)

    zipFile=r'HoLC-'+version+r'-win32-zip.zip'

    #Create zip package
    print "Creating zip package..."
    call7z(r'a -tzip '+zipFile+r' '+tempDir+r'\*')
    callMove(zipFile, holcDirOutput)
    if not fileExists(holcDirOutput+r'/'+zipFile):
        printError("Failed to create zip package.")
        return False
    printSuccess("Created zip package!")
        
    return True
		
		
def cleanUp():
    print "Cleaning up..."
    #Remove temporary output directory
    callRd(tempDir)
    #Rename backup bin folder, remove build files
    if pathExists(holcDir+r'\bin_build_backup'):
        callRd(holcDir+r'\bin_last_build')
        callMove(holcDir+r'\bin', holcDir+r'\bin_last_build')
        callMove(holcDir+r'\bin_build_backup', holcDir+r'\bin')
    
    
#################################
# Execution of file begins here #
#################################

print "\n"
print "/------------------------------------------------------------\\"
print "| HOLC RELEASE COMPILATION SCRIPT                            |"
print "|                                                            |"
print "| Written by Robert Braun 2014-03-30                         |"
print "\\------------------------------------------------------------/"
print "\n"

success=True

if not verifyPaths():
    success = False
    cleanUp()
    printError("Compilation script failed while verifying paths.")
	
if success:
    pauseOnFailValidation = False;

    if not renameBinFolder():
        success = False
        cleanUp()
		
if success:
    if not buildRelease():
        success = False
        cleanUp()
        printError("Compilation script failed in compilation error.")
		
if success:
    #Unpack depedency bin files to bin folder without asking stupid questions, we do this in the build step to have a run-able compiled version before running tests
    call7z(r'x '+quotePath(holcDir+"\\"+dependecyBinFile)+r' -o'+quotePath(holcDir+r'\bin')+r' -y')

if success:
    if not createCleanOutputDirectory():
        success = False
        cleanUp()
		
if success:
    if not copyFiles():
        success = False
        cleanUp()
        printError("Compilation script failed when copying files.")
	
if success:
    if not createInstallFiles():
        success = False
        cleanUp()
        printError("Compilation script failed while generating install files.")
		
if success:
    cleanUp()
    printSuccess("Compilation script finished successfully.")    

raw_input("Press any key to continue...")
