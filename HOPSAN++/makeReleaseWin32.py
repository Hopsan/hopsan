# $Id$

import subprocess
import os
import ctypes
import stat

devversion="0.6."
tbbversion="tbb30_20110704oss"
tempDir="C:\\temp_release"
scriptFile="HopsanReleaseInnoSetupScript.iss"
hopsanDir=os.getcwd()
dependecyBinFiles=".\\hopsan_bincontents_Qt484_MinGW44_Py275_OpenSSL101e.7z"

inkscapeDirList = ["C:\\Program Files\\Inkscape", "C:\\Program Files (x86)\\Inkscape"]
innoDirList = ["C:\\Program Files\\Inno Setup 5", "C:\\Program Files (x86)\\Inno Setup 5"]
qtlibDirList = ["C:\Qt\4.8.4"]
qtcreatorDirList = ["C:\Qt\qtcreator-2.6.0", "C:\Qt\qtcreator-2.6.1", "C:\Qt\qtcreator-2.6.2", "C:\Qt\qtcreator-2.7.1"]
mingwDirList = ["C:\mingw\bin", "C:\MinGW-gcc440_1\mingw\bin"]
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

def rawPath(text):
    return r'"'+raw(text)+r'"'

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
    if os.path.exists(os.path.dirname(raw(path))):
        if okMsg!="":
            printSuccess(okMsg)
        return True
    else:
        if failMsg!="":
            printError(failMsg)
        return False

def fileExists(file):
    return os.path.isfile(raw(file))
    
def selectPathFromList(list, failMsg, sucessMsg):
    selected=""
    for item in list:
        if pathExists(item+"\\"):
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


def writeDoNotSafeFileHereFileToAllDirectories(rootDir):
    for dirpath, dirnames, filenames in os.walk(rootDir,topdown=True):
        print "Adding DoNotSaveFile to directory: "+dirpath
        open(dirpath+"\---DO_NOT_SAVE_FILES_IN_THIS_DIRECTORY---", 'a+').close()

def svnExport(src, dst):
    print "Exporting from "+rawPath(src)+ " to "+rawPath(dst)
    os.system("svn export -q "+rawPath(src)+" "+rawPath(dst))

def callXcopy(src, dst):
    os.system("xcopy "+rawPath(src)+" "+rawPath(dst)+" /s /y")

def callMkdir(dst):
    os.system("mkdir "+rawPath(dst))

def callRd(tgt):
    os.system("rd "+rawPath(tgt)+" /s /q")

def callEXE(cmd, args):
    print "callEXE: "+rawPath(cmd)+" "+args
    os.system(r'"'+rawPath(cmd)+" "+args+r'"')

def callDel(tgt):
    os.system("del "+rawPath(tgt))
    

def verifyPaths():
    print "Verifying and selecting path variables..."

    global inkscapeDir
    global innoDir
    global qtDir
    global msvc2008Dir
    global msvc2010Dir
    global jomDir
    global qmakeDir
    global mingwDir
   
    #Check if Qt path exists
    qtDir=selectPathFromList(qtlibDirList, "Qt libs could not be found in one of the expected locations.", "Found Qt libs!")
    if qtDir == "":
        return False;    

    #Check if qtcreator path exist  
    creatorDir=selectPathFromList(qtcreatorDirList, "Qt Creator could not be found in one of the expected locations.", "Found Qt Creator!")
    if creatorDir == "":
        return False;
        
    jomDir=creatorDir+"\\bin"
    qmakeDir=qtDir+"\\bin"

    mingwDir=selectPathFromList(mingwDirList, "MinGW could not be found in one of the expected locations.", "Found MinGW!")
    if mingwDir == "":
        return False

    #Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
    innoDir=selectPathFromList(innoDirList, "Inno Setup 5 is not installed in expected place.", "Found Inno Setup!")
    if innoDir == "":
        return False;  
            
    #Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
    inkscapeDir=selectPathFromList(inkscapeDirList, "Inkscape is not installed in expected place.", "Found Inkscape!")
    if inkscapeDir == "":
        return False

    #Make sure Visual Studio 2008 is installed in correct location
    msvc2008Dir=selectPathFromList(msvc2008DirList, "Microsoft Visual Studio 2008 is not installed in expected place.", "Found location of Microsoft Visual Studio 2008!")
    if msvc2008Dir == "":
        return False

    #Make sure Visual Studio 2010 is installed in correct location
    msvc2010Dir=selectPathFromList(msvc2010DirList, "Microsoft Visual Studio 2010 is not installed in expected place.", "Found location of Microsoft Visual Studio 2010!")
    if msvc2010Dir == "":
        return False
    
    #Make sure the 3d party dependency file exists
    if not pathExists(dependecyBinFiles+"\\", "The "+ dependecyBinFiles + " file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan", "Found dependency binary files!"):
        return False
        
    #Make sure TBB is installed in correct location
    if not pathExists("HopsanCore\\Dependencies\\"+tbbversion+"\\", "Cannot find correct TBB version, you must use "+ tbbversion+"\\", "Found correct TBB version!"):
        return False
    
    printSuccess("Verification of path variables.")
    return True


def askForVersion():
    dodevrelease=False
    version = raw_input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')    
    if version == "": 
        print "Building DEV release"
        print runCmd("getSvnRevision.bat")[0]
        revnum = raw_input('Enter the revnum shown above: ')
        version = devversion+"x_r"+revnum
        dodevrelease=True

    return (version,dodevrelease)

def msvcCompile(version, architecture):
    print "Compiling HopsanCore with Microsoft Visual Studio "+version+" "+architecture+"..."
    
    #Find correct path (perhaps we should test if it exists first, or this will crash)
    exec "path = msvc"+version+"Dir"
    
    #Remove previous files
    callDel(hopsanDir+"\\bin\\HopsanCore*.*")

    #Create build directory and enter it
    callRd(hopsanDir+"\\HopsanCore_bd")
    callMkdir(hopsanDir+"\\HopsanCore_bd")
    
    os.chdir(hopsanDir+"\\HopsanCore_bd")
      
    #Setup compiler and compile (using auxiliary batch script)
    os.chdir(hopsanDir)
    os.system("compileMSVC.bat "+version+" "+architecture+" "+rawPath(path)+" "+rawPath(qmakeDir)+" "+rawPath(hopsanDir+"\\HopsanCore_bd")+" "+rawPath(jomDir)+" "+rawPath(hopsanDir))

    printDebug(os.environ["PATH"])
    
    #Remove build directory
    callRd(hopsanDir+"\\HopsanCore_bd")

    if not fileExists(hopsanDir+"\\bin\\HopsanCore.dll"):
        printError("Failed to build HopsanCore with Visual Studio "+version+" "+architecture)
        return False

    #Move files to correct MSVC directory
    targetDir = hopsanDir+"\\bin\\MSVC"+version+"_"+architecture
    os.system("mkdir "+targetDir)
    os.system("del /q "+targetDir+"\\*.*")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.dll "+targetDir+"\\HopsanCore.dll")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.lib "+targetDir+"\\HopsanCore.lib")
    os.system("move "+hopsanDir+"\\bin\\HopsanCore.exp "+targetDir+"\\HopsanCore.exp")
    
    return True
   
def buildRelease():
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
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\\\|\" -i HopsanCore\\HopsanCore.pro")
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|componentLibraries||\" -i HopsanNG.pro")

    #Make sure we undefine MAINCORE, so that MSVC dlls do not try to access the log file
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= MAINCORE|#DEFINES *= MAINCORE|\" -i HopsanCore\\HopsanCore.pro")

    #Rename TBB so it is not found when compiling with Visual Studio
    os.rename(hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion, hopsanDir+"\HopsanCore\Dependencies\\"+tbbversion+"_nope")
    
    #BUILD HOPSANCORE WITH MSVC
    if buildVCpp:
        if not msvcCompile("2008", "x86"):
            return False
        if not msvcCompile("2008", "x64"):
            return False
        if not msvcCompile("2010", "x86"):
            return False
        if not msvcCompile("2010", "x64"):
            return False
    
    #Make sure the MinGW compilation uses the MAINCORE define, so that log file is enabled
    runCmd("ThirdParty\\sed-4.2.1\\sed \"s|.*DEFINES \\*= MAINCORE|DEFINES *= MAINCORE|\" -i HopsanCore\\HopsanCore.pro")
     
    #Rename TBB back again (to activate it)
    os.rename(hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion+"_nope", hopsanDir+"\\HopsanCore\\Dependencies\\"+tbbversion)

    #BUILD WITH MINGW32

    #Remove previous files
    os.system("del "+hopsanDir+"\\bin\\HopsanCore*.*")
    os.system("del "+hopsanDir+"\\bin\\HopsanGUI*.*")
    os.system("del "+hopsanDir+"\\bin\\HopsanCLI*.*")

    #Create build directory and enter it
    callRd(hopsanDir+"\\HopsanNG_bd")
    callMkdir(hopsanDir+"\\HopsanNG_bd")
    
    #Setup compiler and compile
    os.chdir(hopsanDir+"\\HopsanNG_bd")    
    os.system("..\\compileMinGW.bat \""+raw(mingwDir)+"\" \""+raw(qmakeDir)+"\" \""+raw(hopsanDir)+"\"")

    if not fileExists(hopsanDir+"\\bin\\HopsanCore.dll") or not fileExists(hopsanDir+"\\bin\\HopsanGUI.exe") or not fileExists(hopsanDir+"\\bin\\HopsanCLI.exe"):
        printError("Failed to build Hopsan with MinGW.")
        return False
    
    printSuccess("Compilation using MinGW")
    
    return True
    

def runValidation():
    os.chdir(hopsanDir)
    return subprocess.call("runValidationTests.bat nopause") == 0
    
    
def copyFiles():
    global dodevrelease
    
    #Create a temporary release directory
    callMkdir(tempDir)
    if not pathExists(tempDir):
        printError("Failed to create temporary directory")
        return False
    
    callMkdir(tempDir+"\\models")
    callMkdir(tempDir+"\\Scripts")
    callMkdir(tempDir+"\\bin")
    callMkdir(tempDir+"\\componentLibraries")
    callMkdir(tempDir+"\\doc\\user\\html")
    callMkdir(tempDir+"\\doc\\graphics")
    callMkdir(tempDir+"\\ThirdParty")
    
    #Unpack depedency bin files to bin folder without asking stupid questions
    #os.system("\""+raw(hopsanDir)+"\"\\ThirdParty\\7z\\7z.exe -y x "+dependecyBinFiles+" -o"+tempDir+"\\bin")
    callEXE(hopsanDir+"\\ThirdParty\\7z\\7z.exe", "x "+dependecyBinFiles+" -o"+rawPath(tempDir+"\\bin")+" -y")

    #Clear old output folder
    callRd(hopsanDir+"\\output")
    if pathExists("\""+raw(hopsanDir)+"\"\\output"):
        printWarning("Unable to clear old output folder.")
        if not askYesNoQuestion("Continue? (y/n): "):
            return False
        
    #Create new output folder
    callMkdir(hopsanDir+"\output")
    if not pathExists(raw(hopsanDir)+"\\output"):
        printError("Failed to create output folder.")
        return False

    #Copy "bin" folder to temporary directory
    callXcopy("bin\\*.exe", tempDir+"\\bin")
    callXcopy("bin\\*.dll", tempDir+"\\bin")
    callXcopy("bin\\*.a", tempDir+"\\bin")
    callXcopy("bin\\*.lib", tempDir+"\\bin")
    callXcopy("bin\\*.exp", tempDir+"\\bin")
##    os.system("xcopy bin\\python27.zip \""+raw(tempDir)+"\"\\bin /s /y")
##    pythonFailed=True
##    if not fileExists(tempDir+"\\bin\\python27.zip"):
##        printError("Failed to find python27.zip.")
##        return False
        
    #Delete unwanted (debug) files from temporary directory
    callDel(tempDir+"\\bin\\*_d.exe")
    callDel(tempDir+"\\bin\\*_d.a")
    callDel(tempDir+"\\bin\\*_d.dll")
    callDel(tempDir+"\\bin\\tbb_debug.dll")
    callDel(tempDir+"\\bin\\qwtd.dll")

    #Build user documentation
    os.system("buildUserDocumentation")
    if not fileExists(hopsanDir+"\\doc\\user\\html\\index.html"):
        printError("Failed to build user documentation")

    #Export "HopsanCore" SVN directory to "include" in temporary directory
    svnExport("HopsanCore", tempDir+"\\HopsanCore")
 
    #Copy the svnrevnum.h file Assume it exist, ONLY for DEV builds
    if dodevrelease:
        callXcopy("HopsanCore\\include\\svnrevnum.h", tempDir+"\\HopsanCore\\include")

    #Export "Example Models" SVN directory to temporary directory
    svnExport("Models\\Example Models", tempDir+"\\models\\Example Models")
    
    #Export "Test Models" SVN directory to temporary directory
    svnExport("Models\\Component Test", tempDir+"\\models\\Component Test")

    #Export "Benchmark Models" SVN directory to temporary directory
    svnExport("Models\\Benchmark Models", tempDir+"\\models\\Benchmark Models")

    #Export and copy "componentData" SVN directory to temporary directory
    svnExport("componentLibraries\\defaultLibrary", tempDir+"\\componentLibraries\\defaultLibrary")

    #Export "exampleComponentLib" SVN directory to temporary directory
    svnExport("componentLibraries\\exampleComponentLib", tempDir+"\\componentLibraries\\exampleComponentLib")

    #Export "Scripts" folder to temporary directory
    svnExport("Scripts\\HopsanOptimization.py", tempDir+"\\Scripts")
    svnExport("Scripts\\OptimizationObjectiveFunctions.py", tempDir+"\\Scripts")
    svnExport("Scripts\\OptimizationObjectiveFunctions.xml", tempDir+"\\Scripts")
    svnExport("Scripts\\HCOM", tempDir+"\\Scripts\\HCOM")

    #Copy "hopsandefaults" file to temporary directory
    svnExport("hopsandefaults", tempDir+"\\hopsandefaults")
    
    #Copy "release notes" file to temporary directory
    svnExport("Hopsan-release-notes.txt", tempDir+"\\Hopsan-release-notes.txt")
    
    #Copy 7zip to temporary directory
    svnExport("ThirdParty\\7z", tempDir+"\\ThirdParty\\7z")

    #Copy fmi to temporary directory
    svnExport("ThirdParty\\fmi", tempDir+"\\ThirdParty\\fmi")

    #Copy documentation to temporary directory
    callXcopy("doc\\user\\html\\*", tempDir+"\\doc\\user\\html")
    callXcopy("doc\\graphics\\*", tempDir+"\\doc\\graphics")

    #Write the do not save files here file 
    writeDoNotSafeFileHereFileToAllDirectories(tempDir)

    #Set all files to read-only
    setReadOnlyForAllFilesInDir(tempDir)

    return True
    
    
def createInstallFiles():
    
    #Create zip package
    print "Creating zip package..."
    callEXE(hopsanDir+"\\ThirdParty\\7z\\7z.exe", "a -tzip Hopsan-"+version+"-win32-zip.zip "+rawPath(tempDir+"\\*"))
    os.system(r'move Hopsan-'+version+r'-win32-zip.zip output/')
    if not fileExists("output/Hopsan-"+version+"-win32-zip.zip"):
        printError("Failed to create zip package.")
        return False
    printSuccess("Created zip package!")
        
    #Execute Inno compile script
    print "Generating install executable..."
    print r'"'+raw(innoDir)+r'\iscc.exe" /o"output" /f"Hopsan-'+version+r'-win32-installer" /dMyAppVersion="'+version+'r" "'+scriptFile+r'"'
    os.system(r'""'+raw(innoDir)+r'\iscc.exe" /o"output" /f"Hopsan-'+version+r'-win32-installer" /dMyAppVersion="'+version+r'" "'+scriptFile+r'""')
    if not fileExists("output/Hopsan-"+version+"-win32-installer.exe"):
        printError("Failed to create installer executable.")
        return False
    printSuccess("Generated install executable!")

    #Move release notes to output directory
    os.system("copy Hopsan-release-notes.txt \"output/\"")
    
    return True
    
    
def cleanUp():
    print "Cleaning up..."
    
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

success=True

if not verifyPaths():
    success = False
    cleanUp()
    printError("Compilation script failed while verifying paths.")

if success:
    global dodevrelease
    global version
    global buildVCpp
    (version, dodevrelease) = askForVersion()

    abortOnFailValidation = False;
    buildVCpp = askYesNoQuestion("Do you want to build VC++ HopsanCore? (y/n): ")

    print "---------------------------------------"
    print "This is a DEV release: " + str(dodevrelease)
    print "Release version number: " + str(version)
    print "Build VC++ HopsanCore: " + str(buildVCpp)
    print "Abort on faild validation: " + str(abortOnFailValidation)
    print "---------------------------------------"
    if not askYesNoQuestion("Is this OK? (y/n): "):
        printError("Aborted by user.")
        success = False
        cleanUp()
    
if success:
    if not buildRelease():
        success = False
        cleanUp()
        printError("Compilation script failed in compilation error.")

if success:
    if (not runValidation()) and abortOnFailValidation:
        success = False
        cleanUp()
        printError("Compilation script failed in model validation.")
    
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
