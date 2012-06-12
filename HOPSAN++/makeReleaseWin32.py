import subprocess
import os
import ctypes

devversion="0.6."
tbbversion="tbb30_20110704oss"
tempDir="C:\\temp_release"
scriptFile=".\\HopsanReleaseInnoSetupScript.iss"
hopsanDir=os.getcwd()
dependecyBinFiles=".\\hopsan_bincontents_Qt474_MinGW_Py27.7z"

inkscapeDirList = ["C:\\Program Files\\Inkscape", "C:\\Program Files (x86)\\Inkscape"]
innoDirList = ["C:\\Program Files\\Inno Setup 5", "C:\\Program Files (x86)\\Inno Setup 5"]
qtsdkDirList = ["C:\Qt", "C:\QtSDK"]
msvc2008DirList = ["C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin", "C:\Program (x86)\Microsoft SDKs\Windows\v7.0\Bin"]
msvc2010DirList = ["C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin", "C:\Program (x86)\Microsoft SDKs\Windows\v7.1\Bin"]


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

def raw(text):
    """Returns a raw string representation of text"""
    new_string=''
    for char in text:
        try: new_string+=escape_dict[char]
        except KeyError: new_string+=char
    return new_string

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
    print cmd
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    print "Done!"
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

def pathExists(path):
    print "Checking path:"
    print raw(path)
    return os.path.exists(os.path.dirname(raw(path)))

def fileExists(file):
    print "Checking file:"
    print raw(file)
    return os.path.isfile(raw(file))
    
def verifyPaths():
    print "Verifying path variables..."

    global devversion
    global tbbversion
    global tempDir
    global inkscapeDir
    global inkscapeDir2
    global innoDir
    global innoDirList
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
   
    #Check if Qt path exists
    qtsdkDir=""
    for i in range(len(qtsdkDirList)):
        if pathExists(qtsdkDirList[i]+"\\"):
            qtsdkDir = qtsdkDirList[i]
    if qtsdkDir == "":
        printError("Qt SDK could not be found in one of the expected locations.")
        return False   
    printSuccess("Found Qt SDK!")
    
    jomDir=qtsdkDir+"\\QtCreator\\bin"
    qmakeDir=qtsdkDir+"\\Desktop\\Qt\\4.7.4\\mingw\\bin"
    mingwDir=qtsdkDir+"\\mingw\\bin"

    #Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
    innoDir=""
    for i in range(len(innoDirList)):
        if pathExists(innoDirList[i]+"\\"):
            innoDir = innoDirList[i]
    if innoDir == "":
        printError("Inno Setup 5 is not installed in expected place.")
        return False
    printSuccess("Found Inno Setup!")
            
    #Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
    inkscapeDir=""
    for i in range(len(inkscapeDirList)):
        if pathExists(inkscapeDirList[i]+"\\"):
            inkscapeDir = inkscapeDirList[i]
    if inkscapeDir == "":
        printError("Inkscape is not installed in expected place.")
        return False
    printSuccess("Found Inkscape!")

    #Make sure Visual Studio 2008 is installed in correct location
    msvc2008Dir=""
    for i in range(len(msvc2008DirList)):
        if pathExists(msvc2008DirList[i]+"\\"):
            msvc2008Dir = msvc2008DirList[i]
    if msvc2008Dir == "":
        printError("Microsoft Visual Studio 2008 is not installed in expected place.")
        return False
    printSuccess("Found location of Microsoft Visual Studio 2008!")

    #Make sure Visual Studio 2010 is installed in correct location
    msvc2010Dir=""
    for i in range(len(msvc2010DirList)):
        if pathExists(msvc2010DirList[i]+"\\"):
            msvc2010Dir = msvc2010DirList[i]
    if msvc2010Dir == "":
        printError("Microsoft Visual Studio 2010 is not installed in expected place.")
        return False
    printSuccess("Found location of Microsoft Visual Studio 2010!")
    
    #Make sure the 3d party dependency file exists
    if not pathExists(dependecyBinFiles+"\\"):
        printError("The "+ dependecyBinFiles + " file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan")
        return False
    printSuccess("Found dependency binary files!")
        
    #Make sure TBB is installed in correct location
    if not pathExists("HopsanCore\\Dependencies\\"+tbbversion+"\\"):
        printError("Cannot find correct TBB version, you must use "+ tbbversion+"\\")
        return False
    printSuccess("Found correct TBB version!")        
    
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
    while(True):
        ans = raw_input("Is this OK? (y/n): ")
        abort = (ans == "n")
        if ans == "y" or ans == "n":
            break
            
    return (version,dodevrelease,abort)

def msvcCompile(version, architecture):
    print "Compiling HopsanCore with Microsoft Visual Studio "+version+" "+architecture+"..."
    
    #Find correct path (perhaps we should test if it exists first, or this will crash)
    exec "path = msvc"+version+"Dir"
    
    #Remove previous files
    os.system("del "+hopsanDir+"\\bin\\HopsanCore*.*")

    print "Debug 1"
    
    #Create build directory and enter it
    os.system("rd \s\q "+hopsanDir+"\\HopsanCore_bd")
    os.system("mkdir "+hopsanDir+"\\HopsanCore_bd")

    print "Debug 2"
    
    os.chdir(hopsanDir+"\\HopsanCore_bd")
    
    print "Debug 3"
    
    
    #Setup compiler and compile (using auxiliary batch script)
    os.chdir(hopsanDir)
    os.system("compileMSVC.bat "+version+" "+architecture+" \""+raw(path)+"\" \""+raw(qmakeDir)+"\" \""+raw(hopsanDir)+"\"\\HopsanCore_bd \""+raw(jomDir)+"\" \""+raw(hopsanDir)+"\"")

    print "Debug 4"
    
    printDebug(os.environ["PATH"])
    
    #Remove build directory
    os.system("rd /s/q \""+raw(hopsanDir)+r'\HopsanCore_bd"')

    print "Debug 5"
    
    if not fileExists(hopsanDir+"\\bin\\HopsanCore.dll"):
        printError("Failed to build HopsanCore with Visual Studio "+version+" "+architecture)
        return False
    
    print "Debug 6"
    
    #Move files to correct MSVC directory
    targetDir = hopsanDir+"\\bin\\MSVC"+version+"_"+architecture
    os.system("mkdir "+targetDir)
    os.system("del /q "+targetDir+"\\*.*")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.dll "+targetDir+"\\HopsanCore.dll")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.lib "+targetDir+"\\HopsanCore.lib")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.exp "+targetDir+"\\HopsanCore.exp")
    
    return True
    
    
def compile():
    if not verifyPaths():
        return False
    else:
        printSuccess("Verification of path variables.")
        
    global version
    (version, dodevrelease, abort) = getRevision()
    if abort:
        printError("Aborted by user.")
        return False


    if not dodevrelease:
        #Set version numbers (by changing .h files) BEFORE build
        runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION  \\\""+version+"\\\"|g\" -i HopsanCore\\include\\version.h")
        runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \\\""+version+"\\\"|g\" -i HopsanGUI\\version_gui.h")

        #Set splash screen version number
        runCmd("ThirdParty\\sed-4.2.1\\sed \"s|X\.X\.X|"+version+"|g\" -i HopsanGUI\\graphics\\splash2.svg")
        runCmd("\""+inkscapeDir+"\\inkscape.exe\" HopsanGUI/graphics/splash2.svg --export-background=\"#ffffff\" --export-png HopsanGUI/graphics/splash.png")
        
        #Revert changes in svg
        runCmd("svn revert HopsanGUI\\graphics\\splash2.svg")

        #Make sure development flag is not defined
        runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|\" -i HopsanGUI\\HopsanGUI.pro")

    #Make sure we compile defaultLibrary into core
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g\" -i Common.prf")
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc \\\\|\" -i HopsanCore\\HopsanCore.pro")
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|componentLibraries||\" -i HopsanNG.pro")

    #Rename TBB so it is not found when compiling with Visual Studio
    os.rename(hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion, hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion+"_nope")

    #BUILD HOPSANCORE WITH MSVC
    if not msvcCompile("2008", "x86"):
        return False
    if not msvcCompile("2008", "x64"):
        return False
    if not msvcCompile("2010", "x86"):
        return False
    if not msvcCompile("2010", "x64"):
        return False

    #Rename TBB back again (to activate it)
    os.rename(hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion+"_nope", hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion)

    #BUILD WITH MINGW32

    #Remove previous files
    os.system("del "+hopsanDir+"\\bin\\HopsanCore*.*")
    os.system("del "+hopsanDir+"\\bin\\HopsanGUI*.*")
    os.system("del "+hopsanDir+"\\bin\\HopsanCLI*.*")

    #Create build directory and enter it
    os.system("rd \s\q "+hopsanDir+"\\HopsanNG_bd")
    os.system("mkdir "+hopsanDir+"\\HopsanNG_bd")
    
    #Setup compiler and compile
    os.chdir(hopsanDir)    
    os.system("compileMinGW.bat \""+raw(mingwDir)+"\" \""+raw(qmakeDir)+"\" \""+raw(hopsanDir)+"\"")

    if not pathExists(hopsanDir+"\\bin\\HopsanCore.dll") or not pathExists(hopsanDir+"\\bin\\HopsanGUI.exe") or not pathExists(hopsanDir+"\\bin\\HopsanCLI.exe"):
        printError("Failed to build Hopsan with MinGW.")
        return False
    
    return True
    
    
def cleanUp():
    print "Cleaning up..."
    printDebug("Todo: Implement cleanup function.")
    
    #Remove temporary output directory
    os.system("rd /s/q "+tempDir)
     
    #Rename TBB back again (if not done already)
    if pathExists(hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion+"_nope\\"):
        os.rename(hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion+"_nope", hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion)
    
    
#################################
# Execution of file begins here #
#################################

print "\n"
print "/------------------------------------------------------------\\"
print "| HOPSAN RELEASE COMPILATION SCRIPT                          |"
print "|                                                            |"
print "| Written by Robert Braun 2011-10-30                         |"
print "| Revised by Robert Braun and Peter Nordin 2012-03-05        |"
print "| Revised and converted to Python by Robert Braun 2012-06-09 |"
print "\\------------------------------------------------------------/"
print "\n"

if compile():
    cleanUp()
    printSuccess("Compilation script finished successfully.")
else:
    cleanUp()
    print "Compilation script finished with errors."
    
