# $Id$

import subprocess
import os
import ctypes
import stat
import shutil
from time import sleep

# -------------------- Setup Start --------------------
# Version numbers
gBaseVersion = '0.8.x'
gReleaseRevision = ''
gFullVersionName = gBaseVersion

# Global parameters
gDoDevRelease = False
gIncludeCompiler = False

# Build directory
gTemporaryBuildDir = r'C:\temp_release'

# External programs
inkscapeDirList = [r'C:\Program Files\Inkscape', r'C:\Program Files (x86)\Inkscape']
innoDirList = [r'C:\Program Files\Inno Setup 5', r'C:\Program Files (x86)\Inno Setup 5']
doxygenDirList = [r'C:\Program Files\doxygen\bin', r'C:\Program Files (x86)\doxygen\bin']
gsDirList = [r'C:\Program Files\gs\gs9.19\bin', r'C:\Program Files\gs\gs9.18\bin', r'C:\Program Files (x86)\gs\gs9.18\bin']

# Compilers and build tools
qtcreatorDirList = [r'C:\Qt\qtcreator-3.5.1', r'C:\Qt\qtcreator-3.6.0', r'C:\Qt\Tools\QtCreator']
msvc2008DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.0\Bin']
msvc2010DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.1\Bin']

# Runtime binaries to copy to bin directory (Note! Path to qt/bin and mingw/bin and plugin dirs is set by external script)
# Note! This list must be adapted to the actual version of Qt/MinGW that you are using when build ithe release
qtRuntimeBins = ['Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Network.dll', 'Qt5OpenGL.dll', 'Qt5Widgets.dll', 'Qt5Sensors.dll', 'Qt5Positioning.dll', 'Qt5Qml.dll', 'Qt5Quick.dll',
                 'Qt5Sql.dll', 'Qt5Svg.dll', 'Qt5WebKit.dll', 'Qt5Xml.dll', 'Qt5WebKitWidgets.dll', 'Qt5WebChannel.dll', 'Qt5Multimedia.dll', 'Qt5MultimediaWidgets.dll',
                 'Qt5Test.dll', 'icuin56.dll', 'icuuc56.dll', 'icudt56.dll', 'Qt5PrintSupport.dll', 'libeay32.dll', 'ssleay32.dll']
qtRuntimeBins32 = ['Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Network.dll', 'Qt5OpenGL.dll', 'Qt5Widgets.dll', 'Qt5Sensors.dll', 'Qt5Positioning.dll', 'Qt5Qml.dll', 'Qt5Quick.dll',
                 'Qt5Sql.dll', 'Qt5Svg.dll', 'Qt5WebKit.dll', 'Qt5Xml.dll', 'Qt5WebKitWidgets.dll', 'Qt5WebChannel.dll', 'Qt5Multimedia.dll', 'Qt5MultimediaWidgets.dll',
                 'Qt5Test.dll', 'icuin53.dll', 'icuuc53.dll', 'icudt53.dll', 'Qt5PrintSupport.dll', 'libeay32.dll', 'ssleay32.dll']
qtPluginBins  = [r'iconengines/qsvgicon.dll', r'imageformats/qjpeg.dll', r'imageformats/qsvg.dll', r'platforms/qwindows.dll']
mingwBins     = ['libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']
mingwBins32   = ['libgcc_s_dw2-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']
mingwOptBins  = []

# -------------------- Setup End --------------------


# Remember current working dir
hopsanDir = os.getcwd()
STD_OUTPUT_HANDLE = -11


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
    if os.path.isdir(path):
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
    os.system("if exist "+quotePath(tgt)+" rd "+quotePath(tgt)+" /s /q")


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


def callSed(sedcommand):
    callEXE(hopsanDir+r'\ThirdParty\sed-4.2.1\sed.exe', sedcommand)


# Returns the last part of a path (split[1] or split[0] if only one part)
def lastpathelement(path):
    parts = os.path.split(path)
    if len(parts) == 1:
        return parts[0]
    elif len(parts) == 2:
        return parts[1]
    else:
        return None


def copyFileToDir(srcDir, srcFile, dstDir):
    if not srcDir[-1] == '/':
        srcDir = srcDir+'/'
    if not dstDir[-1] == '/':
        dstDir = dstDir+'/'
    src = srcDir+srcFile
    #print(src)
    if fileExists(src):
        src_dirname = os.path.dirname(srcFile)
        if not src_dirname == '':
            #print(src_dirname)
            dstDir=dstDir+src_dirname
        #print(dstDir)
        if not os.path.exists(dstDir):
            print('Creating dst: '+dstDir)
            os.makedirs(dstDir)
        shutil.copy2(src, dstDir)
    else:
        print('Error: Source file '+src+' does not exist!')


#  Copy srcDir into dstDir, creating dstDir if necessary
def copyDirTo(srcDir, dstDir):
    srcDir = os.path.normpath(srcDir)
    dstDir = os.path.normpath(dstDir)
    if os.path.exists(srcDir):
        # Create destination if it does not exist
        if not os.path.exists(dstDir):
            os.makedirs(dstDir)
        tgtDir = os.path.join(dstDir, lastpathelement(srcDir))
        if os.path.exists(tgtDir):
            print('Error: tgtDir '+tgtDir+' already exists')
            return False
        print('Copying: '+srcDir+' to: '+tgtDir)
        shutil.copytree(srcDir, tgtDir)
        return True
    else:
        print('Error: Src directory '+srcDir+' does not exist!')
        return False


def makeMSVCOutDirName(version, arch):
    return "MSVC"+version+"_"+arch
    

def verifyPaths():
    print "Verifying and selecting path variables..."

    global inkscapeDir
    global innoDir
    global qtDir
    global msvc2008Path
    global msvc2010Path
    global jomDir
    global qmakeDir

    isOk = True
   
    #Check if Qt path exists
    qtDir = selectPathFromList(qmakeDir, "Qt libs could not be found in one of the expected locations.", "Found Qt libs!")
    if qtDir == "":
        isOk = False
        

    #Check if qtcreator path exist  
    creatorDir = selectPathFromList(qtcreatorDirList, "Qt Creator could not be found in one of the expected locations.", "Found Qt Creator!")
    if creatorDir == "":
        isOk = False
        
    jomDir = creatorDir+r'\bin'
    qmakeDir = qmakeDir

    #Make sure Visual Studio 2008 is installed in correct location
    msvc2008Path = selectPathFromList(msvc2008DirList, "Microsoft Windows SDK 7.0 (MSVC2008) is not installed in expected place.", "Found location of Microsoft Windows SDK 7.0 (MSVC2008)!")
    #if msvc2008Path == "":
    #    isOk = False

    #Make sure Visual Studio 2010 is installed in correct location
    msvc2010Path = selectPathFromList(msvc2010DirList, "Microsoft Windows SDK 7.1 (MSVC2010) is not installed in expected place.", "Found location of Microsoft Windows SDK 7.1 (MSVC2010)!")
    #if msvc2010Path == "":
    #    isOk = False
    
    #Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
    innoDir = selectPathFromList(innoDirList, "Inno Setup 5 is not installed in expected place.", "Found Inno Setup!")
    if innoDir == "":
        isOk = False  
            
    #Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
    inkscapeDir = selectPathFromList(inkscapeDirList, "Inkscape is not installed in expected place.", "Found Inkscape!")
    if inkscapeDir == "":
        risOk = False

    #Make sure that doxygen is present for documentation build, but we dont care about result just print error if missing
    selectPathFromList(doxygenDirList, "Doxygen is not installed in expected place.", "Found Doxygen!")

    #Make sure that ghostscript is present for documentation build, but we dont care about result just print error if missing
    selectPathFromList(gsDirList, "Ghostscript is not installed in expected place.", "Found Ghostscript!")
    
    if isOk:
        printSuccess("Verification of path variables.")

    return isOk


def askForVersion():
    dodevrelease = False
    version = raw_input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')
    print runCmd("getSvnRevision.bat")[0]
    revnum = raw_input('Enter the revision number shown above: ')
    if version == "": 
        dodevrelease = True
    return version, revnum, dodevrelease


def msvcCompile(msvcVersion, architecture, msvcpath):
    print "Compiling HopsanCore with Microsoft Visual Studio "+msvcVersion+" "+architecture+"..."
    
    if msvcpath == "":
        print(r'Error: msvcpath not set!')
        return False
    
    # Remove previous files
    callDel(hopsanDir+r'\bin\HopsanCore*.*')

    # Create clean build directory
    hopsanBuildDir = hopsanDir+r'\HopsanCore_bd'
    callRd(hopsanBuildDir)
    callMkdir(hopsanBuildDir)
      
    # Create compilation script and compile
    os.chdir(hopsanDir)
    # Generate compile script, setup compiler and compile
    mkspec = "win32-msvc"+msvcVersion
    jom = quotePath(jomDir+r'\jom.exe')
    qmake = quotePath(qmakeDir+r'\qmake.exe')
    hopcorepro = quotePath(hopsanDir+r'\HopsanCore\HopsanCore.pro')
    f = open('compileWithMSVC.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'setlocal enabledelayedexpansion'+"\n")
    f.write(r'call '+quotePath(msvcpath+r'\SetEnv.cmd')+r' /Release /'+architecture+"\n")
    f.write(r'COLOR 07'+"\n")
    f.write(r'cd '+quotePath(hopsanBuildDir)+"\n")
    f.write(r'call '+jom+r' clean'+"\n")
    f.write(r'call '+qmake+r' '+hopcorepro+r' -r -spec '+mkspec+r' "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"'+"\n")
    f.write(r'call '+jom+"\n")
    f.write(r'cd ..'+"\n")
    #f.write("pause\n")
    f.close()

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
    targetDir = hopsanDirBin+"\\"+makeMSVCOutDirName(msvcVersion, architecture)
    callRd(targetDir)
    callMkdir(targetDir)
    callMove(hopsanDirBin+r'\HopsanCore.dll', targetDir+r'\HopsanCore.dll')
    callMove(hopsanDirBin+r'\HopsanCore.lib', targetDir+r'\HopsanCore.lib')
    callMove(hopsanDirBin+r'\HopsanCore.exp', targetDir+r'\HopsanCore.exp')
    
    return True


def prepareSourceCode(versionnumber, revisionnumber, dodevrelease):
    # Regenerate default library
    hopsanDefaultLibraryDir = hopsanDir+r'\componentLibraries\defaultLibrary'
    os.chdir(hopsanDefaultLibraryDir)
    os.system(r'generateLibraryFiles.bat -nopause')
    os.chdir(hopsanDir)

    callCopyFile(r'HopsanGUI\graphics\splash2.svg', r'HopsanGUI\graphics\tempdummysplash.svg')

    if not dodevrelease:
        # Set version numbers (by changing .h files) BEFORE build
        fullversion = versionnumber+'_r'+revisionnumber
        callSed(r'"s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"'+versionnumber+r'\"|g" -i HopsanCore\include\HopsanCoreVersion.h')
        callSed(r'"s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"'+versionnumber+r'\"|g" -i HopsanGUI\version_gui.h')
        callSed(r'"s|#define HOPSANCLIVERSION.*|#define HOPSANCLIVERSION \"'+versionnumber+r'\"|g" -i HopsanCLI\version_cli.h')

        # Hide splash screen development warning
        callSed(r'"s|Development version||g" -i HopsanGUI\graphics\tempdummysplash.svg')

        # Make sure development flag is not defined
        callSed(r'"s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|" -i HopsanGUI\HopsanGUI.pro')

    # Set splash screen version and revision number
    callSed(r'"s|X\.X\.X|'+versionnumber+r'|g" -i HopsanGUI\graphics\tempdummysplash.svg')
    callSed(r'"s|R\.R\.R|r'+revisionnumber+r'|g" -i HopsanGUI\graphics\tempdummysplash.svg')
    # Regenerate splash screen
    callEXE(inkscapeDir+r'\inkscape.exe', r'HopsanGUI\graphics\tempdummysplash.svg --export-background="#ffffff" --export-png HopsanGUI/graphics/splash.png')
    callDel(r'HopsanGUI\graphics\tempdummysplash.svg')

    # Make sure we compile defaultLibrary into core
    callSed(r'"s|.*DEFINES \*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g" -i Common.prf')
    callSed(r'"s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\|g" -i HopsanCore\HopsanCore.pro')
    callSed(r'"/.*<lib.*>.*/d" -i componentLibraries\defaultLibrary\defaultComponentLibrary.xml')
    callSed(r'"s|componentLibraries||" -i HopsanNG_remote.pro')


def buildRelease():

    # Make sure we undefine MAINCORE, so that MSVC dlls do not try to access the log file
    callSed(r'"s|.*DEFINES \*= MAINCORE|#DEFINES *= MAINCORE|g" -i HopsanCore\HopsanCore.pro')

    # Disable TBB so it is not found when compiling with Visual Studio
    callSed(r'"s|.*equals(foundTBB|    equals(NOTBB|g" -i HopsanCore\HopsanCore.pro')

    # ========================================================
    #  Build HOPSANCORE with MSVC, else remove those folders
    # ========================================================
    if buildVCpp:
        if msvc2008Path != "":
            if not msvcCompile("2008", "x86", msvc2008Path):
                return False
            if not msvcCompile("2008", "x64", msvc2008Path):
                return False
        if msvc2010Path != "":
            if not msvcCompile("2010", "x86", msvc2010Path):
                return False
            if not msvcCompile("2010", "x64", msvc2010Path):
                return False
    else:
        hopsanBinDir = hopsanDir+"\\bin\\"
        callRd(hopsanBinDir+makeMSVCOutDirName("2008", "x86"))
        callRd(hopsanBinDir+makeMSVCOutDirName("2008", "x64"))
        callRd(hopsanBinDir+makeMSVCOutDirName("2010", "x86"))
        callRd(hopsanBinDir+makeMSVCOutDirName("2010", "x64"))
    
    # Make sure the MinGW compilation uses the MAINCORE define, so that log file is enabled
    callSed(r'"s|.*DEFINES \*= MAINCORE|DEFINES *= MAINCORE|" -i HopsanCore\HopsanCore.pro')
     
    # Reactivate TBB
    callSed(r'"s|.*equals(NOTBB|    equals(foundTBB|" -i HopsanCore\HopsanCore.pro')

    # ========================================================
    #  BUILD WITH MINGW32
    # ========================================================
    print "Compiling with MinGW"

    # Remove previous files
    #callDel(hopsanDir+r'\bin\HopsanCore*.*')
    #callDel(hopsanDir+r'\bin\HopsanGUI*.*')
    #callDel(hopsanDir+r'\bin\HopsanCLI*.*')

    # Create clean build directory
    hopsanBuildDir = hopsanDir+r'\HopsanNG_bd'
    callRd(hopsanBuildDir)
    callMkdir(hopsanBuildDir)
    
    # Generate compile script, setup compiler and compile
    mkspec = "win32-g++"
    f = open('compileWithMinGW.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'SET PATH='+mingwDir+r';'+qmakeDir+r';%PATH%'+"\n")
    f.write(r'mingw32-make.exe clean'+"\n")
    f.write(r'qmake.exe '+quotePath(hopsanDir+r'\HopsanNG_remote.pro')+r' -r -spec '+mkspec+r' "CONFIG+=release"'+"\n")
    f.write(r'mingw32-make.exe -j4'+"\n")
    #f.write("pause\n")
    f.close()
    
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

    # Make sure we are in the hopsan root
    os.chdir(hopsanDir)
    
    # Create a temporary release directory
    callMkdir(gTemporaryBuildDir)
    if not pathExists(gTemporaryBuildDir):
        printError("Failed to create temporary directory")
        return False

    # Create directories    
    callMkdir(gTemporaryBuildDir+r'\Models')
    callMkdir(gTemporaryBuildDir+r'\bin')
    callMkdir(gTemporaryBuildDir+r'\componentLibraries')
    callMkdir(gTemporaryBuildDir+r'\doc\html')
    callMkdir(gTemporaryBuildDir+r'\doc\graphics')
    callMkdir(gTemporaryBuildDir+r'\ThirdParty')
    callMkdir(gTemporaryBuildDir+r'\Dependencies')

    # Common export dirs
    tempDirBin = gTemporaryBuildDir+r'\bin'
    
    # Copy "bin" folder to temporary directory
    callXcopy(r'bin\*.*', tempDirBin)

    # Build user documentation
    os.system("buildUserDocumentation")
    if not fileExists(hopsanDir+r'\doc\html\index.html'):
        printError("Failed to build user documentation")

    # Export "HopsanCore" SVN directory to "include" in temporary directory
    svnExport("HopsanCore", gTemporaryBuildDir+r'\HopsanCore')

    # Export needed dependencies SVN directories to "include" in the release
    svnExport(r'Dependencies\katex',                gTemporaryBuildDir+r'\Dependencies\katex')
    svnExport(r'Dependencies\IndexingCSVParser',    gTemporaryBuildDir+r'\Dependencies\IndexingCSVParser')
    svnExport(r'Dependencies\libNumHop',            gTemporaryBuildDir+r'\Dependencies\libNumHop')
    svnExport(r'Dependencies\rapidxml-1.13',        gTemporaryBuildDir+r'\Dependencies\rapidxml-1.13')

    # Copy the FMILibrary include files
    FMILibraryDir = r'./Dependencies/FMILibrary-2.0.1'
    if not copyDirTo(FMILibraryDir+r'/install', gTemporaryBuildDir+FMILibraryDir):
        return False

    # Export "Example Models" SVN directory to temporary directory
    svnExport(r'Models\Example Models', gTemporaryBuildDir+r'\Models\Example Models')
    
    # Export "Test Models" SVN directory to temporary directory
    svnExport(r'Models\Component Test', gTemporaryBuildDir+r'\Models\Component Test')

    # Export "Benchmark Models" SVN directory to temporary directory
    svnExport(r'Models\Benchmark Models', gTemporaryBuildDir+r'\Models\Benchmark Models')

    # Export defaultLibrary" SVN directory to temporary directory
    svnExport(r'componentLibraries\defaultLibrary', gTemporaryBuildDir+r'\componentLibraries\defaultLibrary')
    
    # Export "exampleComponentLib" SVN directory to temporary directory
    svnExport(r'componentLibraries\exampleComponentLib', gTemporaryBuildDir+r'\componentLibraries\exampleComponentLib')
    
    # Export "autoLibs" SVN directory to temporary directory
    svnExport(r'componentLibraries\autoLibs', gTemporaryBuildDir+r'\componentLibraries\autoLibs')
   
    # Export "Scripts" folder to temporary directory
    svnExport(r'Scripts', gTemporaryBuildDir+r'\Scripts')

    # Copy "hopsandefaults" file to temporary directory
    svnExport("hopsandefaults", gTemporaryBuildDir+r'\hopsandefaults')
    
    # Copy "release notes" file to temporary directory
    svnExport("Hopsan-release-notes.txt", gTemporaryBuildDir+r'\Hopsan-release-notes.txt')
    
    # Copy 7zip to temporary directory
    svnExport(r'ThirdParty\7z', gTemporaryBuildDir+r'\ThirdParty\7z')

    # Copy fmi to temporary directory
    svnExport(r'ThirdParty\fmi', gTemporaryBuildDir+r'\ThirdParty\fmi')

    # Copy documentation to temporary directory
    callXcopy(r'doc\html\*', gTemporaryBuildDir+r'\doc\html')
    callXcopy(r'doc\graphics\*', gTemporaryBuildDir+r'\doc\graphics')

    # Write the do not save files here file
    writeDoNotSafeFileHereFileToAllDirectories(gTemporaryBuildDir)

    # Set all files to read-only
    setReadOnlyForAllFilesInDir(gTemporaryBuildDir)

    return True

def createZipInstaller(zipFile, outputDir):
    print('Creating zip package: '+zipFile+'...')
    call7z(r'a '+zipFile+r' '+gTemporaryBuildDir)
    callMove(zipFile, outputDir)
    if not fileExists(outputDir+'/'+zipFile):
        printError('Failed to create zip package: '+zipFile)
        return False
    printSuccess('Created zip package: '+zipFile+' successfully!')
    return True

def createInnoInstaller(exeFileName, innoArch, outputDir):
    exeFile=exeFileName+'.exe'
    print 'Generating install executable: '+exeFile+'...'
    innocmd=r' /o"'+outputDir+r'" /f"'+exeFileName+r'" /dMyAppVersion="'+gFullVersionName+r'" /dMyArchitecture="'+innoArch+r'" /dMyFilesSource="'+gTemporaryBuildDir+r'" HopsanReleaseInnoSetupScript.iss'
    #print innocmd
    callEXE(innoDir+r'\iscc.exe', innocmd)
    if not fileExists(outputDir+'/'+exeFile):
        printError('Failed to create installer executable: '+exeFile)
        return False
    printSuccess('Generated install executable: '+exeFile)
    return True

    
    
def createInstallFiles():

    # Make sure we are in the hopsan root
    os.chdir(hopsanDir)

    if gDo64BitRelease:
        zipFile=r'Hopsan-'+gFullVersionName+r'-win64-zip.zip'
        zipWithCompilerFile=r'Hopsan-'+gFullVersionName+r'-win64-with_compiler-7z.7z'
        exeFileName=r'Hopsan-'+gFullVersionName+r'-win64-installer'
        exeWithCompilerFileName=r'Hopsan-'+gFullVersionName+r'-win64-with_compiler-installer'
        innoArch=r'x64'
    else:
        zipFile=r'Hopsan-'+gFullVersionName+r'-win32-zip.zip'
        zipWithCompilerFile=r'Hopsan-'+gFullVersionName+r'-win32-with_compiler-7z.7z'
        exeFileName=r'Hopsan-'+gFullVersionName+r'-win32-installer'
        exeWithCompilerFileName=r'Hopsan-'+gFullVersionName+r'-win32-with_compiler-installer'
        innoArch=r'' #Should be empty for 32-bit

    # Create zip package
    if not createZipInstaller(zipFile, hopsanDirOutput):
        return False
        
    # Execute Inno compile script
    if not createInnoInstaller(exeFileName, innoArch, hopsanDirOutput):
        return False


    # Copy the compiler
    if gIncludeCompiler:
        print('Copying compiler...')
        copyDirTo(mingwDir+r'/../', gTemporaryBuildDir)
        #print('Removing /opt')
        #callRd(gTemporaryBuildDir+r'\mingw64\opt')
        # Now build zip and installer with compiler included
        if not createZipInstaller(zipWithCompilerFile, hopsanDirOutput):
            return False
        if not createInnoInstaller(exeWithCompilerFileName, innoArch, hopsanDirOutput):
            return False

    # Copy release notes to output directory
    callCopyFile('Hopsan-release-notes.txt', hopsanDirOutput)
    
    return True


def createCleanOutputDirectory():
    global hopsanDirOutput
    """Try to remove and recreate the output directory"""
    if gDo64BitRelease:
        hopsanDirOutput=hopsanDir+r'\output64'
    else:
        hopsanDirOutput=hopsanDir+r'\output'
    
    # Clear old output folder
    callRd(hopsanDirOutput)
    if pathExists(hopsanDirOutput):
        printWarning("Unable to clear old output folder.")
        if not askYesNoQuestion("Continue? (y/n): "):
            return False

    # Create new output folder
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
    callRd(gTemporaryBuildDir)
    #Rename backup bin folder, remove build files
    if pathExists(hopsanDir+r'\bin_build_backup'):
        callRd(hopsanDir+r'\bin_last_build')
        callMove(hopsanDir+r'\bin', hopsanDir+r'\bin_last_build')
        callMove(hopsanDir+r'\bin_build_backup', hopsanDir+r'\bin')


def extractHopsanBuildPath(version, arch, pathName):
    # Ok this wil run the script for every variable we call, but it is fast so who cares
    p = subprocess.Popen([r'Dependencies\setHopsanBuildPaths.bat', version, arch], shell=True, stdout=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if p.returncode == 0:  # is 0 if success
        for line in stdout.splitlines():
            if line.startswith(pathName):
                substr = line.split(':', 1)
                substr = line.split(':', 1)
                if len(substr) == 2:
                    #print(substr)
                    #print(substr[1])
                    return substr[1].strip()
    else:
        return 'Failed to run setHopsanBuildPaths.bat script'
    return pathName+' path Not Found!'
    
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

success = True
pauseOnFailValidation = False

gARCH = 'x64'
gDo64BitRelease = True
do32BitRelease = askYesNoQuestion("Do you want to build a 32Bit release? (y/n): ")
if do32BitRelease:
    gARCH = 'x86'
    gDo64BitRelease = False

mingwDir = extractHopsanBuildPath('0.7.x', gARCH, 'MinGW')
qmakeDir = extractHopsanBuildPath('0.7.x', gARCH, 'QMake')
print('MinGW path: '+mingwDir)
print('Qmake path: '+qmakeDir)

if not verifyPaths():
    success = False
    # cleanUp()
    printError("Compilation script failed while verifying paths.")

if success:
    (baseversion, gReleaseRevision, gDoDevRelease) = askForVersion()
    if baseversion != '':
        gBaseVersion = baseversion
    gFullVersionName = gBaseVersion+"_r"+gReleaseRevision

    pauseOnFailValidation = False
    buildVCpp = askYesNoQuestion("Do you want to build VC++ HopsanCore? (y/n): ")
    gIncludeCompiler = askYesNoQuestion("Do you want to include the compiler? (y/n): ")

    print "---------------------------------------"
    print "This is a DEV release: " + str(gDoDevRelease)
    print "This is a 64-bit release: " + str(gDo64BitRelease)
    print "Release version name: " + str(gFullVersionName)
    print "Release revision number: " + str(gReleaseRevision)
    print "Build VC++ HopsanCore: " + str(buildVCpp)
    print "Include compiler: " + str(gIncludeCompiler)
    print "Pause on failed validation: " + str(pauseOnFailValidation)
    print "---------------------------------------"
    if not askYesNoQuestion("Is this OK? (y/n): "):
        printError("Aborted by user.")
        success = False
        cleanUp()

    if not renameBinFolder():
        success = False
        cleanUp()

if gDo64BitRelease:
    gTemporaryBuildDir += r'\Hopsan-'+gFullVersionName+r'-win64'
else:
    qtRuntimeBins = qtRuntimeBins32
    mingwBins = mingwBins32
    gTemporaryBuildDir += r'\Hopsan-'+gFullVersionName+r'-win32'
print("Using TempDir: "+gTemporaryBuildDir)
        
if success:
    prepareSourceCode(gBaseVersion, gReleaseRevision, gDoDevRelease)
    if not buildRelease():
        success = False
        cleanUp()
        printError("Compilation script failed in compilation error.")
    
if success:
    #Copy depedency bin files to bin diirectory
    for f in qtRuntimeBins:
        copyFileToDir(qmakeDir, f, hopsanDir+'/bin')
    for f in qtPluginBins:
        copyFileToDir(qmakeDir+'/../plugins', f, hopsanDir+'/bin')
    for f in mingwBins:
        copyFileToDir(mingwDir, f, hopsanDir+'/bin')
    for f in mingwOptBins:
        copyFileToDir(mingwDir+'/../opt/bin', f, hopsanDir+'/bin')

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
    if not runValidation() and pauseOnFailValidation:
        printWarning("Compilation script failed in model validation.")
        askYesNoQuestion("Press enter to continue!")
        
if success:
    cleanUp()
    printSuccess("Compilation script finished successfully.")    

raw_input("Press any key to continue...")
