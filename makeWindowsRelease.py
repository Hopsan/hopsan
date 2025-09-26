import subprocess
import os
import ctypes
import stat
import shutil
import shlex
import tempfile
import re
import tarfile
import zipfile
import uuid
import time

# -------------------- Setup Start --------------------
# Version numbers
gBaseVersion = '2.23.1'
gReleaseRevision = ''
gFullVersion = gBaseVersion
gReleaseFileVersionName = gBaseVersion

# Global parameters
gDoDevRelease = False
gIncludeCompiler = False

# Build directory
gTemporaryBuildDir = r'C:\temp_release'

# External programs
inkscapeDirList = [r'C:\Program Files\Inkscape\bin', r'C:\Program Files (x86)\Inkscape\bin']
innoDirList = [r'C:\Program Files\Inno Setup 6', r'C:\Program Files (x86)\Inno Setup 6', r'C:\Program Files\Inno Setup 5', r'C:\Program Files (x86)\Inno Setup 5']
doxygenDirList = [r'C:\Program Files\doxygen\bin', r'C:\Program Files (x86)\doxygen\bin']
gsDirList = [r'C:\Program Files\gs\gs9.27\bin', r'C:\Program Files (x86)\gs\gs9.27\bin', r'C:\Program Files\gs\gs9.22\bin', r'C:\Program Files (x86)\gs\gs9.22\bin', r'C:\Program Files\gs\gs9.21\bin', r'C:\Program Files (x86)\gs\gs9.21\bin', r'C:\Program Files\gs\gs9.19\bin', r'C:\Program Files (x86)\gs\gs9.19\bin', r'C:\Program Files\gs\gs9.18\bin', r'C:\Program Files (x86)\gs\gs9.18\bin', r'C:\Program Files (x86)\gs\gs10.04.0\bin', ]

# Compilers and build tools
qtcreatorDirList = [r'C:\Qt\qtcreator-3.5.1', r'C:\Qt\qtcreator-3.6.0', r'C:\Qt\Tools\QtCreator']
msvc2008DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.0\Bin']
msvc2010DirList = [r'C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.1\Bin']

# Runtime binaries to copy to bin directory (Note! Path to qt/bin and mingw/bin and plugin directories is set by external script)
# Note! This list must be adapted to the actual version of Qt/MinGW that you are using when building the release
qtRuntimeBins = ['Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Network.dll', 'Qt5OpenGL.dll', 'Qt5Widgets.dll',
                 'Qt5Sql.dll', 'Qt5Svg.dll', 'Qt5WebKit.dll', 'Qt5Xml.dll', 'Qt5WebKitWidgets.dll',
                 'Qt5Test.dll', 'libicuin56.dll', 'libicuuc56.dll', 'libicudt56.dll', 'Qt5PrintSupport.dll', 'libeay32.dll', 'ssleay32.dll']
qtRuntimeBins32 = ['Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Network.dll', 'Qt5OpenGL.dll', 'Qt5Widgets.dll', 'Qt5Sensors.dll', 'Qt5Positioning.dll', 'Qt5Qml.dll', 'Qt5Quick.dll',
                 'Qt5Sql.dll', 'Qt5Svg.dll', 'Qt5Xml.dll', 'Qt5WebChannel.dll', 'Qt5Multimedia.dll', 'Qt5MultimediaWidgets.dll',
                 'Qt5Test.dll', 'Qt5PrintSupport.dll']
qtPluginBins  = [r'iconengines/qsvgicon.dll', r'imageformats/qjpeg.dll', r'imageformats/qsvg.dll', r'platforms/qwindows.dll']
mingwBins     = ['libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']
mingwBins32   = ['libgcc_s_dw2-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']
mingwOptBins  = []
mingwOptBins32  = ['libeay32.dll', 'ssleay32.dll']

dependencyFiles = ['qwt/lib/qwt.dll', 'zeromq/bin/libzmq.dll', 'hdf5/bin/hdf5_cpp.dll', 'hdf5/bin/hdf5.dll', 'fmilibrary/lib/libfmilib_shared.dll',
                   'discount/bin/libmarkdown.dll']

# -------------------- Setup End --------------------

# Internal global help variables
hopsan_bin_backup_dir = ''
hopsanDir = os.getcwd()


STD_OUTPUT_HANDLE = -11

def quotePath(path):
    """Appends quotes around string if quotes are not already present"""
    if path[0] != r'"':
        path = r'"'+path
    if path[-1] != r'"':
        path = path+r'"'
    return path

def slashAtEnd(path):
    """ Append / at the end of a string (for paths) if not alrady present"""
    if len(path) > 0:
        if path[-1] != '/':
            return path+'/'
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
    print('Error: '+text)
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


def selectPathFromList(list_of_paths, failMsg, sucessMsg):
    selected=""
    for item in list_of_paths:
        if pathExists(item):
            selected = item
            break
    if selected=="":
        printError(failMsg)
    else:
        printSuccess(sucessMsg)
    return selected


def findFileInDirTree(root_dir, file_name):
    for dirpath, dirnames, filenames in os.walk(root_dir,topdown=True):
        for filename in filenames:
            if file_name == filename:
                return True
    return False


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


def replace_pattern(filepath, re_pattern, replacement):
    data = None
    with open(filepath, 'r+') as f:
        data = re.sub(re_pattern, replacement, f.read())
    with open(filepath, 'w+') as f:
        f.write(data)


def replace_line_with_pattern(filepath, re_pattern, replacement):
    data = str()
    with open(filepath, 'r+') as f:
        for line in f:
            if re.search(re_pattern, line) is not None:
                if replacement != '':
                    data += replacement + '\n'
            else:
                data += line
    with open(filepath, 'w+') as f:
        f.write(data)

def prepend_append_line_with_pattern(filepath, re_pattern, prepend_text, append_text):
    data = str()
    with open(filepath, 'r+') as f:
        for line in f:
            if re.search(re_pattern, line) is not None:
                new_line = line;
                new_line = prepend_text + new_line
                new_line = new_line[:-1] + append_text
                data += new_line + '\n'
            else:
                data += line
    with open(filepath, 'w+') as f:
        f.write(data)

def is_git_repo(dir):
    if os.path.isdir(dir):
        dotgitfile = os.path.join(dir, '.git')
        return os.path.isdir(dotgitfile)
    return False

def is_git_submodule(dir):
    if os.path.isdir(dir):
        dotgitfile = os.path.join(dir, '.git')
        return os.path.isfile(dotgitfile)
    return False

def find_repo_root(path):
    if is_git_repo(path) or is_git_submodule(path):
        return path, True
    else:
        parts = os.path.split(path)
        if len(path) > 1:
            return find_repo_root(parts[0])
        else:
            return parts[0], False

def git_export(rel_src_dir, dst_dir, repo_dir=None, allow_dirty=True):
    if repo_dir is None:
        repo_dir = os.getcwd()

    src = rel_src_dir.rstrip('/')
    dst = dst_dir

    # Check if src is subdir or file under a submodule of the current repo
    # if so, then export from the submodule instead
    repo_root, found = find_repo_root(os.path.join(repo_dir,src))
    src_rel_root = os.path.relpath(os.path.abspath(src), repo_root)
    #        print('rr '+repo_root)
    #        print('rd '+repo_dir)
    #        print('srr '+src_rel_root)
    if repo_root != repo_dir and src_rel_root != '.':
        print('Exporting "'+src+'", a member of submodule '+os.path.relpath(repo_root,repo_dir))
        return git_export(src_rel_root, dst_dir, repo_dir=os.path.abspath(repo_root), allow_dirty=allow_dirty )

    if is_git_submodule(rel_src_dir):
        print('Exporting submodule "'+rel_src_dir+'" to "'+dst_dir+'"')
    else:
        print('Exporting "'+rel_src_dir+'" to "'+dst_dir+'"')

    temp_dir = tempfile.mkdtemp()
    time.sleep(1)
    temp_file_name = str(uuid.uuid4())
    temp_file_path = os.path.join(temp_dir, temp_file_name).replace(' ','_')+'.tar'
    temp_file_path_bash = temp_file_path.replace('\\','/')
    src_bash = src.replace('\\','/')

    src_is_file = os.path.isfile(os.path.join(repo_dir,src))

    if src_is_file:
        wd = repo_dir
    elif is_git_submodule(src):
        wd = src
    else:
        wd = repo_dir

    # If a dirty state is allowed, we must first create a stash commit that we then export
    commit_hash = 'HEAD'
    was_stashed_ok = True
    if allow_dirty:
        args = ['git', 'stash', 'create', 'dirty_git_export_state']
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=wd)
        stdout, stderr = p.communicate()
        if p.returncode != 0:
            print(args)
            print('Stash create failed with return code: '+str(p.returncode))
            print(stdout)
            print(stderr)
            was_stashed_ok = False
        else:
            if stdout != '':
                # Remove newline from stdout containing the commit hash
                commit_hash = stdout[:-1]

    if not was_stashed_ok:
        # Cleanup
        shutil.rmtree(temp_dir)
        return False


    if src_is_file:
        src_dir, src_file = os.path.split(src_bash)
        if src_dir != '':
            src_dir = ':'+src_dir
        args = ['git', 'archive', '--format=tar', commit_hash+src_dir, src_file, '-o', temp_file_path_bash]
    else:
        # Append dirname to destination path
        dirname = lastpathelement(src_bash)
        dst = os.path.join(dst_dir, dirname)
        if is_git_submodule(src):
            #print('"'+rel_src_dir+'" is a submodule')
            args = ['git', 'archive', '--format=tar', commit_hash, '-o', temp_file_path_bash]
        else:
            args = ['git', 'archive', '--format=tar', commit_hash+':'+src_bash, '-o', temp_file_path_bash]

    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=wd)
    stdout, stderr = p.communicate()
    was_export_ok = False
    if p.returncode != 0:
        print(args)
        print('Failed with return code: '+str(p.returncode))
        print(stdout)
        print(stderr)
    else:
        tf = tarfile.open(temp_file_path)
        tf.extractall(path=dst)
        tf.close()
        was_export_ok = True

    # Cleanup
    shutil.rmtree(temp_dir)

    if was_export_ok:
        # Now recurse into subdirectories if they are submodules
        for currdir, subdirst, files in os.walk(src):
            if currdir != src:
                currdirdst = os.path.split(os.path.join(dst_dir, currdir))[0]
                if is_git_submodule(currdir):
                    if not git_export(currdir, currdirdst, repo_dir=None, allow_dirty=allow_dirty):
                        was_export_ok = False
                        break

    return was_export_ok

def copy_file(src, dst):
    if fileExists(src):
        shutil.copy(src, dst)
    else:
        print('Could not copy file: '+src+' it does not exist')


def mkdirs(path):
    try:
        os.makedirs(path)
    except OSError:
        if not os.path.isdir(path):
            printError('Could not create directory path: '+path)

def del_rw(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

def callRd(tgt):
    if pathExists(tgt):
        shutil.rmtree(tgt, onerror=del_rw)


def callEXE(cmd, args):
    #print "callEXE: " + quotePath(cmd)+r' '+args
    if fileExists(cmd):
        os.system(r'"'+quotePath(cmd)+r' '+args+r'"')
    else:
        printError(cmd+r' Does not exist!')



def callDel(tgt):
    if fileExists(tgt):
        os.remove(tgt)


def zip_directory(dir_path, zip_file_path):
    print('Compressing directory: '+dir_path+' into: '+zip_file_path)
    file_basepath = os.path.splitext(zip_file_path)[0]
    dir_parts = os.path.split(dir_path)
    if dir_parts[0] != '':
        root = dir_parts[0]
        base = dir_parts[1]
        shutil.make_archive(file_basepath, 'zip', root_dir=root, base_dir=base)
    else:
        base = dir_parts[1]
        shutil.make_archive(file_basepath, 'zip', base_dir=base)


# Returns the last part of a path (split[1] or split[0] if only one part)
def lastpathelement(path):
    parts = os.path.split(path)
    if len(parts) == 1:
        return parts[0]
    elif len(parts) == 2:
        return parts[1]
    else:
        return None


def copyFileToDir(srcDir, srcFile, dstDir, keep_relative_path=True):
    if not srcDir[-1] == '/':
        srcDir = srcDir+'/'
    if not dstDir[-1] == '/':
        dstDir = dstDir+'/'
    src = srcDir+srcFile
    #print(src)
    if fileExists(src):
        if keep_relative_path:
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


def checkFilesExistInDir(root_dir, list_of_files):
    did_find_all = True;
    for f in list_of_files:
        file_path = os.path.join(root_dir, f)
        if not fileExists(file_path):
            printError(file_path+' does not exist!')
            did_find_all = False
    return did_find_all

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
            printError('tgtDir '+tgtDir+' already exists')
            return False
        print('Copying: '+srcDir+' to: '+tgtDir)
        shutil.copytree(srcDir, tgtDir)
        return True
    else:
        printError('Src directory '+srcDir+' does not exist!')
        return False

def move(src, dst):
    print 'moving '+quotePath(src)+' to '+quotePath(dst)
    if src != dst:
        shutil.move(src, dst)

def move_backup(src, dst):
    dst_date=dst+time.strftime('%Y%m%d_%H%M%S')
    move(src, dst_date)
    return dst_date

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

    # Make sure that Ghostscript is present for documentation build, Doxygen seems to require 32-bit version
    gs_dir = selectPathFromList(gsDirList, "Ghostscript 32-bit is not installed in expected place.", "Found Ghostscript!")
    if not findFileInDirTree(gs_dir, 'gswin32.exe'):
        printError('You must install the 32-bit version of Ghostscipt, Doxygen is apparently hard-coded for that version')

    if isOk:
        printSuccess("Verification of path variables.")

    return isOk


def askForVersion():
    dodevrelease = False
    version = raw_input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')
    if version == '':
        dodevrelease = True

    # Get date and time stamp of last commit used instead of "revision number"
    revnum = '19700101.0000'
    p = subprocess.Popen(['getGitInfo.bat', 'date.time', '.'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if p.returncode == 0:
        revnum = stdout[:-1]

    return version, revnum, dodevrelease


def msvcCompile(msvcVersion, architecture, msvcpath):
    print "Compiling HopsanCore with Microsoft Visual Studio "+msvcVersion+" "+architecture+"..."

    if msvcpath == "":
        print(r'Error: msvcpath not set!')
        return False

    # Remove previous files
    callDel(hopsanDir+r'\bin\hopsancore*.*')

    # Create clean build directory
    hopsanBuildDir = hopsanDir+r'\HopsanCore_bd'
    callRd(hopsanBuildDir)
    mkdirs(hopsanBuildDir)

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
    if not fileExists(hopsanDirBin+r'\hopsancore.dll'):
        printError("Failed to build HopsanCore with Visual Studio "+msvcVersion+" "+architecture)
        return False

    #Move files to correct MSVC directory
    targetDir = hopsanDirBin+"\\"+makeMSVCOutDirName(msvcVersion, architecture)
    callRd(targetDir)
    mkdirs(targetDir)
    move(hopsanDirBin+r'\hopsancore.dll', targetDir)
    move(hopsanDirBin+r'\hopsancore.lib', targetDir)
    move(hopsanDirBin+r'\hopsancore.exp', targetDir)

    return True


def prepareSourceCode(versionnumber, revisionnumber, dodevrelease):
    # Regenerate default library
    hopsanDefaultLibraryDir = hopsanDir+r'\componentLibraries\defaultLibrary'
    os.chdir(hopsanDefaultLibraryDir)
    os.system(r'generateLibraryFiles.bat -nopause')
    os.chdir(hopsanDir)

    copy_file(r'HopsanGUI\graphics\splash.svg', r'HopsanGUI\graphics\tempdummysplash.svg')

    fullversion = versionnumber+'.'+revisionnumber
    if not dodevrelease:
        # Set version numbers (by changing .h files)
        replace_pattern('HopsanCore/include/HopsanCoreVersion.h', r'#define HOPSANCOREVERSION .*', r'#define HOPSANCOREVERSION "{}"'.format(fullversion))
        replace_pattern(r'HopsanGUI/version_gui.h', r'#define HOPSANGUIVERSION .*', r'#define HOPSANGUIVERSION "{}"'.format(fullversion))
        replace_pattern(r'HopsanCLI/version_cli.h', r'#define HOPSANCLIVERSION .*', r'#define HOPSANCLIVERSION "{}"'.format(fullversion))

        # Hide splash screen development warning
        replace_pattern(r'HopsanGUI/graphics/tempdummysplash.svg', r'Development version', '')

        # Make sure development flag is not defined
        replace_pattern(r'HopsanGUI/HopsanGUI.pro', r'.*?DEFINES \*= DEVELOPMENT', r'#DEFINES *= DEVELOPMENT')

    # Set the release version definition
    replace_pattern(r'HopsanGUI/version_gui.h', r'#define HOPSANRELEASEVERSION .*', r'#define HOPSANRELEASEVERSION "{}"'.format(fullversion))

    # Set splash screen version and revision number
    replace_pattern(r'HopsanGUI/graphics/tempdummysplash.svg', r'0\.00\.0', versionnumber)
    replace_pattern(r'HopsanGUI/graphics/tempdummysplash.svg', r'20170000\.0000', revisionnumber)
    # Regenerate splash screen
    callEXE(inkscapeDir+r'\inkscape.com', r'HopsanGUI\graphics\tempdummysplash.svg --export-background="#ffffff" --export-dpi=90 --export-type=png --export-filename=HopsanGUI/graphics/splash.png')
    callDel(r'HopsanGUI\graphics\tempdummysplash.svg')

    # Make sure we compile defaultLibrary into core
    replace_pattern('Common.prf', r'.*?DEFINES \*= HOPSAN_INTERNALDEFAULTCOMPONENTS', r'DEFINES *= HOPSAN_INTERNALDEFAULTCOMPONENTS')
    replace_pattern(r'HopsanCore/HopsanCore.pro', r'#INTERNALCOMPLIB.CPP#', r'../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cpp \\')
    replace_pattern(r'HopsanCore/HopsanCore.pro', r'#INTERNALCOMPLIB_FMI4C_DEPENDENCY#', r'include($${PWD}/../dependencies/fmi4c.pri)')
    prepend_append_line_with_pattern('componentLibraries/defaultLibrary/defaultComponentLibrary.xml', '<lib.*?>', '<!-- The lib element is removed here since the default library code is built into the Hopsan Core -->\n<!--', '  -->')
    replace_pattern('componentLibraries/componentLibraries.pro', 'defaultLibrary', '')


def buildRelease():

    # Make sure we undefine HOPSANCORE_WRITELOG, so that MSVC dlls do not try to access the log file
    replace_pattern('HopsanCore/HopsanCore.pro', r'.*?DEFINES \*= HOPSANCORE_WRITELOG', r'#DEFINES *= HOPSANCORE_WRITELOG')

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

    # Make sure the MinGW compilation uses the HOPSANCORE_WRITELOG define, so that log file is enabled
    replace_pattern('HopsanCore/HopsanCore.pro',r'.*?DEFINES \*= HOPSANCORE_WRITELOG', 'DEFINES *= HOPSANCORE_WRITELOG')

    # ========================================================
    #  BUILD WITH MINGW32
    # ========================================================
    print "Compiling with MinGW"

    # Remove previous files
    #callDel(hopsanDir+r'\bin\hopsancore*.*')
    #callDel(hopsanDir+r'\bin\hopsangui*.*')
    #callDel(hopsanDir+r'\bin\hopsancli*.*')

    # Create clean build directory
    hopsanBuildDir = hopsanDir+r'\HopsanNG_bd'
    callRd(hopsanBuildDir)
    mkdirs(hopsanBuildDir)

    # Generate compile script, setup compiler and compile
    mkspec = "win32-g++"
    f = open('compileWithMinGW.bat', 'w')
    f.write(r'echo off'+"\n")
    f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    f.write(r'SET PATH='+mingwDir+r';'+qmakeDir+r';%PATH%'+"\n")
    f.write(r'mingw32-make.exe clean'+"\n")
    f.write(r'qmake.exe '+quotePath(hopsanDir+r'\HopsanNG.pro')+r' -r -spec '+mkspec+r' "CONFIG+=release"'+"\n")
    f.write(r'mingw32-make.exe -j8'+"\n")
    #f.write("pause\n")
    f.close()

    os.chdir(hopsanBuildDir)
    os.system(r'..\compileWithMinGW.bat')

    if not fileExists(hopsanDir+r'\bin\hopsancore.dll') or not fileExists(hopsanDir+r'\bin\hopsangui.exe') or not fileExists(hopsanDir+r'\bin\hopsancli.exe'):
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
    mkdirs(gTemporaryBuildDir)
    if not pathExists(gTemporaryBuildDir):
        printError("Failed to create temporary directory")
        return False

    # Create directories
    mkdirs(gTemporaryBuildDir+'/Models')
    mkdirs(gTemporaryBuildDir+'/componentLibraries')
    mkdirs(gTemporaryBuildDir+'/doc')
    mkdirs(gTemporaryBuildDir+'/dependencies/tools')
    mkdirs(gTemporaryBuildDir+'/hopsanc')

    # Copy "bin" folder to temporary directory
    copyDirTo(r'bin', gTemporaryBuildDir)

    # Build user documentation
    os.system("buildUserDocumentation")
    if not fileExists(hopsanDir+r'\doc\html\index.html'):
        printError("Failed to build user documentation")

    # Export HopsanCore including all source code, then cleanup unneeded file
    git_export('HopsanCore', gTemporaryBuildDir)
    callRd(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/config')
    callRd(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/doc')
    callRd(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/examples')
    callRd(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/test')
    callDel(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/INSTALL_GUIDE.pdf')
    callDel(gTemporaryBuildDir+r'/HopsanCore/dependencies/sundials/CMakeLists.txt')

    # Export HopsanC include directory
    git_export('hopsanc/include', gTemporaryBuildDir+r'/hopsanc')

    # Export needed core code dependencies
    copyDirTo(r'dependencies/katex',                gTemporaryBuildDir+r'/dependencies')

    # Copy 3pdependency installations
    if not copyDirTo(r'dependencies/fmilibrary', gTemporaryBuildDir+r'/dependencies'):
        return False
    if not copyDirTo(r'dependencies/fmi4c', gTemporaryBuildDir+r'/dependencies'):
        return False

    # Copy 7zip to temporary directory
    git_export(r'dependencies/tools/7z', gTemporaryBuildDir+'/dependencies/tools')

    # Export "Example Models" SVN directory to temporary directory
    git_export(r'Models\Example Models', gTemporaryBuildDir+r'\Models')

    # Export "Test Models" SVN directory to temporary directory
    git_export(r'Models\Component Test', gTemporaryBuildDir+r'\Models')

    # Export "Benchmark Models" SVN directory to temporary directory
    git_export(r'Models\Benchmark Models', gTemporaryBuildDir+r'\Models')

    # Export defaultLibrary" SVN directory to temporary directory
    git_export(r'componentLibraries\defaultLibrary', gTemporaryBuildDir+r'\componentLibraries')

    # Export "exampleComponentLib" SVN directory to temporary directory
    git_export(r'componentLibraries\exampleComponentLib', gTemporaryBuildDir+r'\componentLibraries')
    copyFileToDir('componentLibraries/exampleComponentLib', 'exampleComponentLib.dll', gTemporaryBuildDir+r'\componentLibraries\exampleComponentLib')

    # Export ModelicaExmpleLibrary
    git_export(r'componentLibraries\ModelicaExampleLib', gTemporaryBuildDir+r'\componentLibraries')

    # Export "extensionLibrary" Git directory to temporary directory
    git_export(r'componentLibraries\extensionLibrary', gTemporaryBuildDir+r'\componentLibraries')
    copyFileToDir('componentLibraries/extensionLibrary', 'extensionLibrary.dll', gTemporaryBuildDir+r'\componentLibraries\extensionLibrary')

    # Export "autoLibs" SVN directory to temporary directory
    git_export(r'componentLibraries\autoLibs', gTemporaryBuildDir+r'\componentLibraries')

    # Export "Scripts" folder to temporary directory
    git_export(r'Scripts', gTemporaryBuildDir)

    # Copy "hopsan-default-configuration.xml" file to temporary directory
    git_export("hopsan-default-configuration.xml", gTemporaryBuildDir)

    # Copy "release notes" file to temporary directory
    git_export("Hopsan-release-notes.txt", gTemporaryBuildDir)

    # Copy "README.md" file to temporary directory
    git_export("README.md", gTemporaryBuildDir)

    # Copy documentation to temporary directory
    copyDirTo(r'doc\html', gTemporaryBuildDir+r'\doc')
    copyDirTo(r'doc\graphics', gTemporaryBuildDir+r'\doc')

    # Write the do not save files here file
    writeDoNotSafeFileHereFileToAllDirectories(gTemporaryBuildDir)

    # Set all files to read-only
    setReadOnlyForAllFilesInDir(gTemporaryBuildDir)

    return True

def createZipInstaller(zipFile, outputDir):
    print('Creating zip package: '+zipFile+'...')
    zip_directory(gTemporaryBuildDir, zipFile)
    move(zipFile, outputDir)
    if not fileExists(os.path.join(outputDir, zipFile)):
        printError('Failed to create zip package: '+zipFile)
        return False
    printSuccess('Created zip package: '+zipFile+' successfully!')
    return True

def createInnoInstaller(exeFileName, innoArch, outputDir):
    exeFile=exeFileName+'.exe'
    print 'Generating install executable: '+exeFile+'...'
    innocmd=r' /o"'+outputDir+r'" /f"'+exeFileName+r'" /dMyAppVersion="'+gFullVersion+r'" /dMyArchitecture="'+innoArch+r'" /dMyFilesSource="'+gTemporaryBuildDir+r'" packaging/windows/HopsanReleaseInnoSetupScript.iss'
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
        zipFile=r'Hopsan-'+gReleaseFileVersionName+r'-win64-zip.zip'
        zipWithCompilerFile=r'Hopsan-'+gReleaseFileVersionName+r'-win64-with_compiler-zip.zip'
        exeFileName=r'Hopsan-'+gReleaseFileVersionName+r'-win64-installer'
        exeWithCompilerFileName=r'Hopsan-'+gReleaseFileVersionName+r'-win64-with_compiler-installer'
        innoArch=r'x64'
    else:
        zipFile=r'Hopsan-'+gReleaseFileVersionName+r'-win32-zip.zip'
        zipWithCompilerFile=r'Hopsan-'+gReleaseFileVersionName+r'-win32-with_compiler-zip.zip'
        exeFileName=r'Hopsan-'+gReleaseFileVersionName+r'-win32-installer'
        exeWithCompilerFileName=r'Hopsan-'+gReleaseFileVersionName+r'-win32-with_compiler-installer'
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
        mingwDirName = os.path.basename(os.path.normpath(mingwDir+r'/../'))
        if gDo64BitRelease:
            move(os.path.join(gTemporaryBuildDir, mingwDirName), os.path.join(gTemporaryBuildDir, 'mingw64'))
        else:
            move(os.path.join(gTemporaryBuildDir, mingwDirName), os.path.join(gTemporaryBuildDir, 'mingw'))
        #print('Removing /opt')
        #callRd(gTemporaryBuildDir+r'\mingw64\opt')
        # Now build zip and installer with compiler included
        if not createZipInstaller(zipWithCompilerFile, hopsanDirOutput):
            return False
        if not createInnoInstaller(exeWithCompilerFileName, innoArch, hopsanDirOutput):
            return False

    # Copy release notes to output directory
    copy_file('Hopsan-release-notes.txt', hopsanDirOutput)

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
    mkdirs(hopsanDirOutput)
    if not pathExists(hopsanDirOutput):
        printError("Failed to create output folder.")
        return False

    return True


def renameBinFolder():
    global hopsan_bin_backup_dir
    # Move the bin folder to temp storage to avoid packagin dev junk into release
    hopsan_bindir = hopsanDir+r'/bin'
    hopsan_bin_backup_dir = ''
    if pathExists(hopsan_bindir):
        hopsan_bin_backup_dir = move_backup(hopsan_bindir, hopsanDir+r'/bin_build_backup')
        time.sleep(1)
    if pathExists(hopsan_bindir):
        printError("Could not move the bin folder to temporary backup before build.")
        return False

    # Create clean bin directory
    mkdirs(hopsan_bindir)
    return True


def cleanUp():
    print "Cleaning up..."
    #Remove temporary output directory
    callRd(gTemporaryBuildDir)
    #Rename backup bin folder, remove build files
    hopsan_bindir = hopsanDir+r'\bin'
    if pathExists(hopsan_bin_backup_dir):
        callRd(hopsanDir+r'\bin_last_build')
        move(hopsan_bindir, hopsanDir+r'\bin_last_build')
        move(hopsan_bin_backup_dir, hopsan_bindir)


def extractHopsanBuildPath(arch, path_name):
    # Ok this wil run the script for every variable we call, but it is fast so who cares
    p = subprocess.Popen([r'dependencies\setHopsanBuildPaths.bat', arch], shell=True, stdout=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if p.returncode == 0:  # is 0 if success
        for line in stdout.splitlines():
            #print(line)
            if line.startswith(path_name):
                substrs = line.split(':', 1)
                #print(substrs)
                if len(substrs) == 2:
                    #print(substrs)
                    #print(substrs[1])
                    return substrs[1].strip()
    else:
        return 'Failed to run setHopsanBuildPaths.bat script'
    return path_name+' path Not Found!'

#################################
# Execution of file begins here #
#################################

print "\n"
print "/------------------------------------------------------------\\"
print "| HOPSAN RELEASE BUILD AND PACKAGING SCRIPT                  |"
print "|                                                            |"
print "\\------------------------------------------------------------/"
print "\n"

success = True
pauseOnFailValidation = False
doBuild = True

gARCH = 'x64'
gDo64BitRelease = True
do32BitRelease = askYesNoQuestion("Do you want to build a 32Bit release? (y/n): ")
if do32BitRelease:
    gARCH = 'x86'
    gDo64BitRelease = False

mingwDir = extractHopsanBuildPath(gARCH, 'mingw')
qmakeDir = extractHopsanBuildPath(gARCH, 'qmake')
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
    gFullVersion = gBaseVersion+"."+gReleaseRevision
    gReleaseFileVersionName = gBaseVersion
    if gDoDevRelease:
        gReleaseFileVersionName = gFullVersion

    pauseOnFailValidation = False
    buildVCpp = askYesNoQuestion("Do you want to build VC++ HopsanCore? (y/n): ")
    gIncludeCompiler = askYesNoQuestion("Do you want to include the compiler? (y/n): ")

    print "---------------------------------------"
    print "This is a DEV release: " + str(gDoDevRelease)
    print "This is a 64-bit release: " + str(gDo64BitRelease)
    print "Release file version name: " + str(gReleaseFileVersionName)
    print "Release revision number: " + str(gReleaseRevision)
    print "Release full version number: " + str(gFullVersion)
    print "Build VC++ HopsanCore: " + str(buildVCpp)
    print "Include compiler: " + str(gIncludeCompiler)
    print "Pause on failed validation: " + str(pauseOnFailValidation)
    print "---------------------------------------"
    if askYesNoQuestion("Is this OK? (y/n): "):
        success = renameBinFolder()
    else:
        printError("Aborted by user.")
        success = False

if gDo64BitRelease:
    gTemporaryBuildDir += r'\Hopsan-'+gReleaseFileVersionName+r'-win64'
else:
    qtRuntimeBins = qtRuntimeBins32
    mingwBins = mingwBins32
    mingwOptBins = mingwOptBins32
    gTemporaryBuildDir += r'\Hopsan-'+gReleaseFileVersionName+r'-win32'
print("Using TempDir: "+gTemporaryBuildDir)

qt_bins_ok = checkFilesExistInDir(qmakeDir, qtRuntimeBins)
qt_plugins_ok = checkFilesExistInDir(qmakeDir+'/../plugins',qtPluginBins)
mingw_bins_ok = checkFilesExistInDir(mingwDir, mingwBins)
mingw_optbins_ok = checkFilesExistInDir(mingwDir+'/../opt/bin', mingwOptBins)
deps_ok = checkFilesExistInDir(hopsanDir+'/dependencies', dependencyFiles)

success = success and qt_bins_ok and qt_plugins_ok and mingw_bins_ok and mingw_optbins_ok and deps_ok

if not success:
    cleanUp()
    printError("Could not find all needed files.")

if success:
    prepareSourceCode(gBaseVersion, gReleaseRevision, gDoDevRelease)
    if doBuild:
        if not buildRelease():
            success = False
            cleanUp()
            printError("Compilation script failed in compilation error.")

if success:
    #Copy dependency bin files to bin directory
    for f in qtRuntimeBins:
        copyFileToDir(qmakeDir, f, hopsanDir+'/bin')
    for f in qtPluginBins:
        copyFileToDir(qmakeDir+'/../plugins', f, hopsanDir+'/bin')
    for f in mingwBins:
        copyFileToDir(mingwDir, f, hopsanDir+'/bin')
    for f in mingwOptBins:
        copyFileToDir(mingwDir+'/../opt/bin', f, hopsanDir+'/bin')
    for f in dependencyFiles:
        copyFileToDir(hopsanDir+'/dependencies', f, hopsanDir+'/bin', keep_relative_path=False)

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
