# $Id$

import subprocess
import os
import ctypes
import stat
from time import sleep

devversion="0.7."
tempDir=r'C:\temp_release'
scriptFile="HopsanReleaseInnoSetupScript.iss"
dependecyBinFile32=r'hopsan_bincontents_Qt485_MinGW44_Py275_OpenSSL101e.7z'
dependecyBinFile64=r'hopsan_bincontents64_Qt485_MinGW64_481_OpenSSL101e.7z'

# External programs
inkscapeDirList = [r'C:\Program Files\Inkscape', r'C:\Program Files (x86)\Inkscape']
innoDirList = [r'C:\Program Files\Inno Setup 5', r'C:\Program Files (x86)\Inno Setup 5']

# Libs
qtlibDirList = [r'C:\Qt\4.8.5']
qtlib64DirList = [r'C:\Qt\Qt64-4.8.5']
tbblibDirList = [r'.\HopsanCore\Dependencies\tbb30_20110704oss', r'.\HopsanCore\Dependencies\tbb41_20130613oss']
tbblib64DirList = [r'.\HopsanCore\Dependencies\tbb30_20110704oss_64', r'.\HopsanCore\Dependencies\tbb41_20130613oss_64']

# Compilers and build tools
qtcreatorDirList = [r'C:\Qt\qtcreator-2.8.1']
mingwDirList = [r'C:\Qt\MinGW-gcc440_1\mingw\bin', r'C:\Qt\mingw\bin', r'C:\mingw\bin']
mingw64DirList = [r'C:\Qt\mingw64\bin']
msvc2008DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.0\Bin']
msvc2010DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.1\Bin']

# Remember current working dir
hopsanDir=os.getcwd()

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


def writeDoNotSafeFileHereFileToAllDirectories(rootDir):
    for dirpath, dirnames, filenames in os.walk(rootDir,topdown=True):
        print "Adding DoNotSaveFile to directory: "+dirpath
        open(dirpath+"\---DO_NOT_SAVE_FILES_IN_THIS_DIRECTORY---", 'a+').close()

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
    callEXE(hopsanDir+r'\ThirdParty\7z\7z.exe', args)

def callSed(sedCommand):
    callEXE(hopsanDir+r'\ThirdParty\sed-4.2.1\sed.exe', sedCommand);
    

def makeMSVCDirName(version, arch):
    return "MSVC"+version+"_"+arch
    

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
    global tbbDir
    global dependecyBinFile

    isOk = True
   
    
    if do64BitRelease:
        qtlibsdirs=qtlib64DirList
        tbbdirs=tbblib64DirList
        mingwdirs=mingw64DirList
        dependecyBinFile=dependecyBinFile64
    else:
        qtlibsdirs=qtlibDirList
        tbbdirs=tbblibDirList
        mingwdirs=mingwDirList
        dependecyBinFile=dependecyBinFile32

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

    #Make sure Visual Studio 2008 is installed in correct location
    msvc2008Dir=selectPathFromList(msvc2008DirList, "Microsoft Windows SDK 7.0 (MSVC2008) is not installed in expected place.", "Found location of Microsoft Windows SDK 7.0 (MSVC2008)!")
    if msvc2008Dir == "":
        isOk = False

    #Make sure Visual Studio 2010 is installed in correct location
    msvc2010Dir=selectPathFromList(msvc2010DirList, "Microsoft Windows SDK 7.1 (MSVC2010) is not installed in expected place.", "Found location of Microsoft Windows SDK 7.1 (MSVC2010)!")
    if msvc2010Dir == "":
        isOk = False
    
    #Make sure the 3d party dependency file exists
    if not pathExists(dependecyBinFile, "The "+ dependecyBinFile + " file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan", "Found dependency binary files!"):
        isOk = False
        
    #Make sure TBB is installed in correct location
    tbbDir=selectPathFromList(tbbdirs, "Cannot find correct TBB version", "Found correct TBB version!")
    if tbbDir == "":
        isOk = False

    #Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
    innoDir=selectPathFromList(innoDirList, "Inno Setup 5 is not installed in expected place.", "Found Inno Setup!")
    if innoDir == "":
        isOk = False  
            
    #Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
    inkscapeDir=selectPathFromList(inkscapeDirList, "Inkscape is not installed in expected place.", "Found Inkscape!")
    if inkscapeDir == "":
        risOk = False

    if isOk:
        printSuccess("Verification of path variables.")

    return isOk


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

def msvcCompile(msvcVersion, architecture):
    print "Compiling HopsanCore with Microsoft Visual Studio "+msvcVersion+" "+architecture+"..."
    
    #Find correct path (perhaps we should test if it exists first, or this will crash)
    exec "msvcPath = msvc"+msvcVersion+"Dir"
    
    #Remove previous files
    callDel(hopsanDir+r'\bin\HopsanCore*.*')

    #Create clean build directory
    hopsanBuildDir=hopsanDir+r'\HopsanCore_bd'
    callRd(hopsanBuildDir)
    callMkdir(hopsanBuildDir)
      
    #Create compilation script and compile
    os.chdir(hopsanDir)
    # Generate compile script, setup compiler and compile
    mkspec="win32-msvc"+msvcVersion
    jom=quotePath(jomDir+r'\jom.exe')
    qmake=quotePath(qmakeDir+r'\qmake.exe')
    hopcorepro=quotePath(hopsanDir+r'\HopsanCore\HopsanCore.pro')
    f = open('compileWithMSVC.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'setlocal enabledelayedexpansion'+"\n")
    f.write(r'call '+quotePath(msvcPath+r'\SetEnv.cmd')+r' /Release /'+architecture+"\n")
    f.write(r'COLOR 07'+"\n")
    f.write(r'cd '+quotePath(hopsanBuildDir)+"\n")
    f.write(r'call '+jom+r' clean'+"\n")
    f.write(r'call '+qmake+r' '+hopcorepro+r' -r -spec '+mkspec+r' "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"'+"\n")
    f.write(r'call '+jom+"\n")
    f.write(r'cd ..'+"\n")
    #f.write("pause\n")
    f.close();

    # Compile
    os.system("compileWithMSVC.bat")
    #printDebug(os.environ["PATH"])
    
    #Remove build directory
    callRd(hopsanBuildDir)

    hopsanDirBin = hopsanDir+r'\bin'
    if not fileExists(hopsanDirBin+r'\HopsanCore.dll'):
        printError("Failed to build HopsanCore with Visual Studio "+msvcVersion+" "+architecture)
        return False

    #Move files to correct MSVC directory
    targetDir = hopsanDirBin+"\\"+makeMSVCDirName(msvcVersion,architecture)
    callRd(targetDir)
    callMkdir(targetDir)
    callMove(hopsanDirBin+r'\HopsanCore.dll', targetDir+r'\HopsanCore.dll')
    callMove(hopsanDirBin+r'\HopsanCore.lib', targetDir+r'\HopsanCore.lib')
    callMove(hopsanDirBin+r'\HopsanCore.exp', targetDir+r'\HopsanCore.exp')
    
    return True
   
def buildRelease():
    if not dodevrelease:
        #Set version numbers (by changing .h files) BEFORE build
        callSed(r'"s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"'+version+r'\"|g" -i HopsanCore\include\version.h')
        callSed(r'"s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"'+version+r'\"|g" -i HopsanGUI\version_gui.h')

        #Deactivate Groups icon as it is not working yet
        callSed(r'"/Group.xml/d" -i HopsanGUI\Resources.qrc')

        #Set splash screen version number
        callCopyFile(r'HopsanGUI\graphics\splash2.svg', r'HopsanGUI\graphics\tempdummysplash.svg')
        callSed(r'"s|X\.X\.X|'+version+r'|g" -i HopsanGUI\graphics\tempdummysplash.svg')
        callEXE(inkscapeDir+r'\inkscape.exe', r'HopsanGUI\graphics\tempdummysplash.svg --export-background="#ffffff" --export-png HopsanGUI/graphics/splash.png')
        callDel(r'HopsanGUI\graphics\tempdummysplash.svg')

        #Make sure development flag is not defined
        callSed(r'"s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|" -i HopsanGUI\HopsanGUI.pro')

    #Make sure we compile defaultLibrary into core
    callSed(r'"s|.*DEFINES \*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g" -i Common.prf')
    callSed(r'"s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\|g" -i HopsanCore\HopsanCore.pro')
    callSed(r'"/.*<lib>.*/d" -i componentLibraries\defaultLibrary\defaultComponentLibrary.xml')
    callSed(r'"s|componentLibraries||" -i HopsanNG.pro')

    #Make sure we undefine MAINCORE, so that MSVC dlls do not try to access the log file
    callSed(r'"s|.*DEFINES \*= MAINCORE|#DEFINES *= MAINCORE|g" -i HopsanCore\HopsanCore.pro')

    #Disable TBB so it is not found when compiling with Visual Studio
    callSed(r'"s|.*equals(foundTBB|    equals(NOTBB|g" -i HopsanCore\HopsanCore.pro')

    #========================================================
    # Build HOPSANCORE with MSVC, else remove those folders
    #========================================================
    if buildVCpp:
        if not msvcCompile("2008", "x86"):
            return False
        if not msvcCompile("2008", "x64"):
            return False
        if not msvcCompile("2010", "x86"):
            return False
        if not msvcCompile("2010", "x64"):
            return False
    else:
        hopsanBinDir=hopsanDir+"\\bin\\"
        callRd(hopsanBinDir+makeMSVCDirName("2008", "x86"))
        callRd(hopsanBinDir+makeMSVCDirName("2008", "x64"))
        callRd(hopsanBinDir+makeMSVCDirName("2010", "x86"))
        callRd(hopsanBinDir+makeMSVCDirName("2010", "x64"))
    
    #Make sure the MinGW compilation uses the MAINCORE define, so that log file is enabled
    callSed(r'"s|.*DEFINES \*= MAINCORE|DEFINES *= MAINCORE|" -i HopsanCore\HopsanCore.pro')
     
    #Reactivate TBB
    callSed(r'"s|.*equals(NOTBB|    equals(foundTBB|" -i HopsanCore\HopsanCore.pro')

    #========================================================
    # BUILD WITH MINGW32
    #========================================================
    print "Compiling with MinGW"

    #Remove previous files
    #callDel(hopsanDir+r'\bin\HopsanCore*.*')
    #callDel(hopsanDir+r'\bin\HopsanGUI*.*')
    #callDel(hopsanDir+r'\bin\HopsanCLI*.*')

    #Create clean build directory
    hopsanBuildDir = hopsanDir+r'\HopsanNG_bd'
    callRd(hopsanBuildDir)
    callMkdir(hopsanBuildDir)
    
    # Generate compile script, setup compiler and compile
    if do64BitRelease:
        mkspec="win32-g++-4.6"
    else:
        mkspec="win32-g++"
    f = open('compileWithMinGW.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'SET PATH='+mingwDir+r';'+qmakeDir+r';%PATH%'+"\n")
    f.write(r'mingw32-make.exe clean'+"\n")
    f.write(r'qmake.exe '+quotePath(hopsanDir+r'\HopsanNG.pro')+r' -r -spec '+mkspec+r' "CONFIG+=release"'+"\n")
    f.write(r'mingw32-make.exe'+"\n")
    #f.write("pause\n")
    f.close();
    
    os.chdir(hopsanBuildDir)    
    os.system(r'..\compileWithMinGW.bat')

    if not fileExists(hopsanDir+r'\bin\HopsanCore.dll') or not fileExists(hopsanDir+r'\bin\HopsanGUI.exe') or not fileExists(hopsanDir+r'\bin\HopsanCLI.exe'):
        printError("Failed to build Hopsan with MinGW.")
        return False
  
    printSuccess("Compilation using MinGW")

    return True
    

def runValidation():
    print "Running validation tests"
    os.chdir(hopsanDir)
    return subprocess.call("runValidationTests.bat nopause") == 0
    
    
def copyFiles():
    global dodevrelease

    # Make sure we are in the hopsan root
    os.chdir(hopsanDir)
    
    #Create a temporary release directory
    callMkdir(tempDir)
    if not pathExists(tempDir):
        printError("Failed to create temporary directory")
        return False

    # Create directories    
    callMkdir(tempDir+r'\Models')
    callMkdir(tempDir+r'\Scripts')
    callMkdir(tempDir+r'\bin')
    callMkdir(tempDir+r'\componentLibraries')
    callMkdir(tempDir+r'\doc\user\html')
    callMkdir(tempDir+r'\doc\graphics')
    callMkdir(tempDir+r'\ThirdParty')

    # Common export dirs
    tempDirBin=tempDir+r'\bin'
    tempDirScripts=tempDir+r'\Scripts'
    
    #Copy "bin" folder to temporary directory
    callXcopy(r'bin\*.*', tempDirBin)

    #Build user documentation
    os.system("buildUserDocumentation")
    if not fileExists(hopsanDir+r'\doc\user\html\index.html'):
        printError("Failed to build user documentation")

    #Export "HopsanCore" SVN directory to "include" in temporary directory
    svnExport("HopsanCore", tempDir+r'\HopsanCore')
 
    #Copy the svnrevnum.h file Assume it exist, ONLY for DEV builds
    callXcopy(r'HopsanCore\include\svnrevnum.h', tempDir+r'\HopsanCore\include')

    #Export "Example Models" SVN directory to temporary directory
    svnExport(r'Models\Example Models', tempDir+r'\Models\Example Models')
    
    #Export "Test Models" SVN directory to temporary directory
    svnExport(r'Models\Component Test', tempDir+r'\Models\Component Test')

    #Export "Benchmark Models" SVN directory to temporary directory
    svnExport(r'Models\Benchmark Models', tempDir+r'\Models\Benchmark Models')

    #Export defaultLibrary" SVN directory to temporary directory
    svnExport(r'componentLibraries\defaultLibrary', tempDir+r'\componentLibraries\defaultLibrary')

    #Export "exampleComponentLib" SVN directory to temporary directory
    svnExport(r'componentLibraries\exampleComponentLib', tempDir+r'\componentLibraries\exampleComponentLib')

    #Export "Scripts" folder to temporary directory
    svnExport(r'Scripts\HopsanOptimization.py', tempDirScripts)
    svnExport(r'Scripts\OptimizationObjectiveFunctions.py', tempDirScripts)
    svnExport(r'Scripts\OptimizationObjectiveFunctions.xml', tempDirScripts)
    svnExport(r'Scripts\HCOM', tempDirScripts+r'\HCOM')

    #Copy "hopsandefaults" file to temporary directory
    svnExport("hopsandefaults", tempDir+r'\hopsandefaults')
    
    #Copy "release notes" file to temporary directory
    svnExport("Hopsan-release-notes.txt", tempDir+r'\Hopsan-release-notes.txt')
    
    #Copy 7zip to temporary directory
    svnExport("ThirdParty\\7z", tempDir+r'\ThirdParty\7z')

    #Copy fmi to temporary directory
    svnExport("ThirdParty\\fmi", tempDir+r'\ThirdParty\fmi')

    #Copy documentation to temporary directory
    callXcopy(r'doc\user\html\*', tempDir+r'\doc\user\html')
    callXcopy(r'doc\graphics\*', tempDir+r'\doc\graphics')

    #Write the do not save files here file 
    writeDoNotSafeFileHereFileToAllDirectories(tempDir)

    #Set all files to read-only
    setReadOnlyForAllFilesInDir(tempDir)

    return True
    
    
def createInstallFiles():

    # Make sure we are in the hopsan root
    os.chdir(hopsanDir)

    if do64BitRelease:
        zipFile=r'Hopsan-'+version+r'-win64-zip.zip'
        exeFileName=r'Hopsan-'+version+r'-win64-installer'
        innoArch=r'x64'
    else:
        zipFile=r'Hopsan-'+version+r'-win32-zip.zip'
        exeFileName=r'Hopsan-'+version+r'-win32-installer'
        innoArch=r'' #Should be empty for 32-bit

    exeFile=exeFileName+r'.exe'
    
    #Create zip package
    print "Creating zip package..."
    call7z(r'a -tzip '+zipFile+r' '+tempDir+r'\*')
    callMove(zipFile, "output")
    if not fileExists(r'output/'+zipFile):
        printError("Failed to create zip package.")
        return False
    printSuccess("Created zip package!")
        
    #Execute Inno compile script
    print "Generating install executable..."
    innocmd=r' /o"output" /f"'+exeFileName+r'" /dMyAppVersion="'+version+r'" /dMyArchitecture="'+innoArch+r'" '+scriptFile  
    #print innocmd
    callEXE(innoDir+r'\iscc.exe', innocmd)
    if not fileExists(r'output/'+exeFile):
        printError("Failed to create installer executable.")
        return False
    printSuccess("Generated install executable!")

    #Copy release notes to output directory
    callCopyFile('Hopsan-release-notes.txt', 'output')
    
    return True

def createCleanOutputDirectory():
    """Try to remove and recreate the output directory"""
    hopsanDirOutput=hopsanDir+r'\output'

    #Clear old output folder
    callRd(hopsanDirOutput)
    if pathExists(hopsanDirOutput):
        printWarning("Unable to clear old output folder.")
        if not askYesNoQuestion("Continue? (y/n): "):
            return False

    #Create new output folder
    callMkdir(hopsanDirOutput)
    if not pathExists(hopsanDirOutput):
        printError("Failed to create output folder.")
        return False

    return True

def renameBinFolder():
    # Move the bin folder to temp storage to avoid packagin dev junk into release
    if pathExists(hopsanDir+r'\bin'):
        callMove(hopsanDir+r'\bin', hopsanDir+r'\bin_build_backup')
        sleep(1)
    if pathExists(hopsanDir+r'\bin'):
        printError("Could not move the bin folder to temporary backup before build.")
        return False
        
    # Create clean bin directory
    callMkdir(hopsanDir+r'\bin')
    return True
    
        
def cleanUp():
    print "Cleaning up..."
    #Remove temporary output directory
    callRd(tempDir)
    #Rename backup bin folder, remove build files
    if pathExists(hopsanDir+r'\bin_build_backup'):
        callRd(hopsanDir+r'\bin_last_build')
        callMove(hopsanDir+r'\bin', hopsanDir+r'\bin_last_build')
        callMove(hopsanDir+r'\bin_build_backup', hopsanDir+r'\bin')
    
    
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
global do64BitRelease
do64BitRelease = askYesNoQuestion("Do you want to build a 64Bit release? (y/n): ")

if not verifyPaths():
    success = False
    cleanUp()
    printError("Compilation script failed while verifying paths.")

if success:
    global dodevrelease
    global version
    global buildVCpp
    (version, dodevrelease) = askForVersion()

    pauseOnFailValidation = False;
    buildVCpp = askYesNoQuestion("Do you want to build VC++ HopsanCore? (y/n): ")

    print "---------------------------------------"
    print "This is a DEV release: " + str(dodevrelease)
    print "This is a 64-bit release: " + str(do64BitRelease)
    print "Release version number: " + str(version)
    print "Build VC++ HopsanCore: " + str(buildVCpp)
    print "Pause on faild validation: " + str(pauseOnFailValidation)
    print "---------------------------------------"
    if not askYesNoQuestion("Is this OK? (y/n): "):
        printError("Aborted by user.")
        success = False
        cleanUp()

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
    call7z(r'x '+quotePath(hopsanDir+"\\"+dependecyBinFile)+r' -o'+quotePath(hopsanDir+r'\bin')+r' -y')

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
    if (not runValidation()) and pauseOnFailValidation:
        printWarning("Compilation script failed in model validation.")
        askYesNoQuestion("Press enter to continue!")
        
if success:
    cleanUp()
    printSuccess("Compilation script finished successfully.")    

raw_input("Press any key to continue...")
