import subprocess
import os
import ctypes

devversion="0.6."
tbbversion="tbb30_20110704oss"
tempDir="C:\\temp_release"
inkscapeDir="C:\\Program Files\\Inkscape"
inkscapeDir2="C:\\Program Files (x86)\\Inkscape"
innoDir="C:\\Program Files\\Inno Setup 5"
innoDir2="C:\\\"Program Files (x86)\"\\Inno Setup 5"
scriptFile="HopsanReleaseInnoSetupScript.iss"
hopsanDir=os.getcwd()
qtsdkDir="C:\\Qt"
qtsdkDir2="C:\\QtSDK"
msvc2008Dir="C:\\\"Program Files\"\\\"Microsoft SDKs\"\\Windows\\v7.0\\Bin"
msvc2010Dir="C:\\\"Program Files\"\\\"Microsoft SDKs\"\\Windows\\v7.1\\Bin"
dependecyBinFiles=".\\hopsan_bincontents_Qt474_MinGW_Py27.7z"

STD_OUTPUT_HANDLE= -11

class bcolors:
  WHITE = 0x07
  GREEN= 0x0A
  RED = 0x0C
  YELLOW = 0x0E
  BLUE = 0x09

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

def printTodo(text):
    setColor(bcolors.BLUE)
    print "Todo: " + text
    setColor(bcolors.WHITE)

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
    global jomDir
    global qmakeDir
    global mingwDir
   # print runCmd('cp dummy dummy2')

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
    if not pathExists(inkscapeDir):
        inkscapeDir = inkscapeDir2
        if not pathExists(inkscapeDir):
            printError("Inkscape is not installed in expected place.")
            return False

    #Make sure the 3d party dependency file exists
    if not pathExists(dependecyBinFiles):
        printError("The "+ dependecyBinFiles + " file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan")
        return False

    return True


def getRevision():

    dodevrelease=False
    version = raw_input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')    
    if version == "": 
        print "Building DEV release"
        print runCmd("getSvnRevision.bat")[0]
        revnum = raw_input('Enter the revnum shown above: ')
        version = devversion+"x_r"+revnum
        dodevrelease=True

    print "---------------------------------------"
    print "This is a DEV release: " + str(dodevrelease)
    print "Release version number: " + str(version)
    print "---------------------------------------"
    print "Is this OK?"
    ans = raw_input("Answer y or n: ")
    abort = (ans == "n")
    return (version,dodevrelease,abort)


if not verifyPaths():
    printTodo("To implement: Cleanup")

global version
(version, dodevrelease, abort) = getRevision()
if abort:
    printError("Aborted.")


if not dodevrelease:
	#Set version numbers (by changing .h files) BEFORE build
	runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION "+version+"|g\" -i HopsanCore\\include\\version.h")
	runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION "+version+"|g\" -i HopsanGUI\\version_gui.h")

    #Set splash screen version number
	runCmd("ThirdParty\\sed-4.2.1\\sed \"s|X\.X\.X|"+version+"|g\" -i HopsanGUI\\graphics\\splash2.svg")
	runCmd(inkscapeDir+"\\inkscape.exe HopsanGUI/graphics/splash2.svg --export-background=\"#ffffff\" --export-png HopsanGUI/graphics/splash.png")
	
	#Revert changes in svg
	runCmd("svn revert HopsanGUI\\graphics\\splash2.svg")

	#Make sure development flag is not defined
	runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|\" -i HopsanGUI\\HopsanGUI.pro")

#Make sure we compile defaultLibrary into core
runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g\" -i Common.prf")
runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc \\\\|\" -i HopsanCore\\HopsanCore.pro")
runCmd("ThirdParty\\sed-4.2.1\\sed \"s|componentLibraries||\" -i HopsanNG.pro")


#Rename TBB so it is not found when compiling with Visual Studio
if not pathExists("HopsanCore\\Dependencies\\"+tbbversion):
    printError("Cannot find correct TBB version, you must use "+ tbbversion)

os.rename(hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion, hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion+"_nope")


#BUILD HOPSANCORE WITH MSVC2008 32-bit

#Remove previous files
os.system("del "+hopsanDir+"\\bin\\HopsanCore*.*")

#Create build directory and enter it
os.system("rd \s\q "+hopsanDir+"\\HopsanCore_bd")
os.system("mkdir "+hopsanDir+"\\HopsanCore_bd")

os.chdir(hopsanDir+"\\HopsanCore_bd")

#Setup compiler and compile
os.system(str(msvc2010Dir+"\\SetEnv.cmd /Release /x86"))
print "debug1"
os.system(str(qmakeDir+"\\qtenv2.bat"))
print "debug2"
os.system(str(jomDir+"\\jom.exe clean"))
print "debug3"
os.system(str(qmakeDir+"\\qmake.exe "+hopsanDir+"\HopsanCore\HopsanCore.pro -r -spec win32-msvc2010 \"CONFIG+=release\" \"QMAKE_CXXFLAGS_RELEASE += -wd4251\""))
print "debug4"
os.system(str(jomDir+"\\jom.exe"))

#Remove build directory
os.system("rd /s/q "+hopsanDir+"\\HopsanCore_bd")

#cd bin
if not pathExists(hopsanDir+"\\bin\\HopsanCore.dll"):
	printError("Failed to build HopsanCore with Visual Studio 2010 32-bit")
