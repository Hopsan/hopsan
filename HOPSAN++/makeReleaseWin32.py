import subprocess
import os


devversion="0.6."
tbbversion="tbb30_20110704oss"
tempDir="C:\temp_release"
inkscapeDir="C:\Program Files\Inkscape"
inkscapeDir2="C:\Program Files (x86)\Inkscape"
innoDir="C:\Program Files\Inno Setup 5"
innoDir2="C:\Program Files (x86)\Inno Setup 5"
scriptFile="HopsanReleaseInnoSetupScript.iss"
hopsanDir=os.getcwd()
qtsdkDir="C:\Qt"
qtsdkDir2="C:\QtSDK"
msvc2008Dir="C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin"
msvc2010Dir="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin"
dependecyBinFiles="hopsan_bincontents_Qt474_MinGW_Py27.7z"


class bcolors:
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BLUE = '\033[94m'
    ENDC = '\033[0m'

def runCmd(cmd):
    splitcmd = cmd.split()
    process = subprocess.Popen(splitcmd, stdout=subprocess.PIPE)
    return process.communicate()

def printSuccess(text):
    print bcolors.GREEN + "Success: " + text + bcolors.ENDC

def printWarning(text):
    print bcolors.YELLOW + "Warning: " + text + bcolors.ENDC

def printError(text):
    print bcolors.RED + "Error: " + text + bcolors.ENDC

def printTodo(text):
    print bcolors.BLUE + "Todo: " + text + bcolors.ENDC

def pathExists(path):
    return os.path.exists(os.path.dirname(path))

def verifyPaths():
    global devversion
    global tbbversion
    global tempDir
    global inkscapeDir
    global inkscapeDir2
    global innoDir
    global innoDir2
    global scriptFile
    global hopsanDir
    global qtsdkDir
    global qtsdkDir2
    global msvc2008Dir
    global msvc2010Dir
    global dependecyBinFiles

    print runCmd('cp dummy dummy2')

    #Check if Qt path exists
    if not pathExists(qtsdkDir):
        qtsdkDir = qtsdkDir2;
        if not pathExists(qtsdkDir):
            printError("Qt SDK could not be found in one of the expected locations.")
            return False
    
    jomDir=qtsdkDir+"\\QtCreator\\bin"
    qmakeDir=qtsdkDir+"\\Desktop\\Qt\\4.7.4\\mingw\\bin"
    mingwDir=qtsdkDir+"\\mingw\\bin"

    #Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
    if not pathExists(innoDir):
        innoDir = innoDir2    
        if not pathExists(innoDir):
            printError("Inno Setup 5 is not installed in expected place.")
            return False

    #Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
    if not pathExist(inkscapeDir):
        inkscapeDir = inkscakeDir2
        if not pathExist(inkscakeDir):
            printError("Inkscape is not installed in expected place.")
            return False

    #Make sure the 3d party dependency file exists
    if not pathExist(dependecyBinFiles):
        printError("The %dependecyBinFiles% file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan")
        return False

    return True


def getRevision():

    dodevrelease=False
    version = raw_input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')    
    if version == "": 
        print "Building DEV release"
        runCmd("getSvnRevision")
        revnum = raw_input('Enter the revnum shown above: ')
        version = devversion+revnum
        dodevrelease=True

    print "---------------------------------------"
    print "This is a DEV release: " + str(dodevrelease)
    print "Release version number: " + str(version)
    print "---------------------------------------"
    print "Is this OK?"
    ans = raw_input("Answer y or n: ")
    abort = (ans == "n")
    return (version,dodevrelease,abort)

#echo.
#echo ---------------------------------------
#echo This is a DEV release: %dodevrelease%
#echo Release version number: %version%
#echo ---------------------------------------
#echo Is this OK?
#set /P ans="Answer y or n: "
#call :abortIfStrNotMatch "%ans%" "y"




if not verifyPaths():
    printTodo("To implement: Cleanup")

(version, dodevrelease, abort) = getRevision()
if abort:
    printError("Aborted.")




