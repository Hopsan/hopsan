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
from pathlib import Path
from enum import IntEnum

# -------------------- Setup Start --------------------
# Version numbers
gBaseVersion = '2.23.1'
gReleaseRevision = ''
gFullVersion = gBaseVersion
gReleaseFileVersionName = gBaseVersion

# Build directory
g_temporary_build_root = Path("C:/temp_release")

# External programs
inkscapeDirList = [r'C:\Program Files\Inkscape\bin', r'C:\Program Files (x86)\Inkscape\bin']
innoDirList = [r'C:\Program Files\Inno Setup 6', r'C:\Program Files (x86)\Inno Setup 6', r'C:\Program Files\Inno Setup 5', r'C:\Program Files (x86)\Inno Setup 5']
doxygenDirList = [r'C:\Program Files\doxygen\bin', r'C:\Program Files (x86)\doxygen\bin']
gsDirList = [r'C:\Program Files\gs\gs9.27\bin', r'C:\Program Files (x86)\gs\gs9.27\bin', r'C:\Program Files\gs\gs9.22\bin', r'C:\Program Files (x86)\gs\gs9.22\bin', r'C:\Program Files\gs\gs9.21\bin', r'C:\Program Files (x86)\gs\gs9.21\bin', r'C:\Program Files\gs\gs9.19\bin', r'C:\Program Files (x86)\gs\gs9.19\bin', r'C:\Program Files\gs\gs9.18\bin', r'C:\Program Files (x86)\gs\gs9.18\bin', r'C:\Program Files (x86)\gs\gs10.04.0\bin', ]

# Compilers and build tools
qtcreatorDirList = [r'C:\Qt\Tools\QtCreator']
msvc2022DirList = [r'C:\Program Files\Microsoft Visual Studio\2022\Community', r'C:\Program (x86)\Microsoft SDKs\Windows\v7.1\Bin']

# Runtime binaries to copy to bin directory (Note! Path to qt/bin and mingw/bin and plugin directories is set by external script)
# Note! This list must be adapted to the actual version of Qt/MinGW that you are using when building the release
qtRuntimeBins = ['Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Network.dll', 'Qt5OpenGL.dll', 'Qt5Widgets.dll',
                 'Qt5Sql.dll', 'Qt5Svg.dll', 'Qt5WebKit.dll', 'Qt5Xml.dll', 'Qt5WebKitWidgets.dll',
                 'Qt5Test.dll', 'libicuin56.dll', 'libicuuc56.dll', 'libicudt56.dll', 'Qt5PrintSupport.dll', 'libeay32.dll', 'ssleay32.dll']
qtPluginBins  = [r'iconengines/qsvgicon.dll', r'imageformats/qjpeg.dll', r'imageformats/qsvg.dll', r'platforms/qwindows.dll']
mingwBins     = ['libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll']
mingwOptBins  = []

dependencyFiles = ['qwt/lib/qwt.dll', 'zeromq/bin/libzmq.dll', 'hdf5/bin/libhdf5_cpp.dll', 'hdf5/bin/libhdf5.dll', 'discount/bin/libmarkdown.dll']

# -------------------- Setup End --------------------

# ----- Input / Output help functions -----

class BColors(IntEnum):
    """Windows console color codes."""
    WHITE = 0x07
    GREEN = 0x0A
    RED = 0x0C
    YELLOW = 0x0E
    BLUE = 0x0B


def set_color(color: BColors, handle=None) -> bool:
    """Set console text color (Windows only)."""
    if handle is None:
        STD_OUTPUT_HANDLE = -11
        handle = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    return ctypes.windll.kernel32.SetConsoleTextAttribute(handle, color)


def print_success(text: str) -> None:
    """Print a success message in green."""
    set_color(BColors.GREEN)
    print(f"Success: {text}")
    set_color(BColors.WHITE)


def print_warning(text: str) -> None:
    """Print a warning message in yellow."""
    set_color(BColors.YELLOW)
    print(f"Warning: {text}")
    set_color(BColors.WHITE)


def print_error(text: str) -> None:
    """Print an error message in red."""
    set_color(BColors.RED)
    print(f"Error: {text}")
    set_color(BColors.WHITE)


def print_debug(text: str) -> None:
    """Print a debug message in blue."""
    set_color(BColors.BLUE)
    print(f"Debug: {text}")
    set_color(BColors.WHITE)


def quote_path(path):
    """Appends quotes around string if quotes are not already present."""
    path = str(path)
    if not path.startswith('"'):
        path = '"' + path
    if not path.endswith('"'):
        path += '"'
    return path


def askYesNoQuestion(msg):
    """Returns True on yes, False on no."""
    while True:
        ans = input(msg).lower()
        if ans in ("y", "yes"):
            return True
        elif ans in ("n", "no"):
            return False


def askForVersion():
    dodevrelease = False
    version = input('Enter release version number on the form a.b.c or leave blank for DEV build release: ')
    if version == '':
        dodevrelease = True

    # Get date and time stamp of last commit used instead of "revision number"
    revnum = '19700101.0000'
    p = subprocess.Popen(['getGitInfo.bat', 'date.time', '.'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout, stderr = p.communicate()
    if p.returncode == 0:
        revnum = stdout.strip()

    return version, revnum, dodevrelease


# ----- Finding files or directories help functions -----

def select_path_from_list(list_of_paths: list[str | Path], fail_msg: str, success_msg: str) -> Path | None:
    """Select the first existing directory from a list of paths.

    Args:
        list_of_paths: List of paths to check.
        fail_msg: Error message if no valid path is found.
        success_msg: Success message if a valid path is found.

    Returns:
        Path object of the first existing directory, or None if no valid path exists.
    """
    for path in list_of_paths:
        path_obj = Path(path)
        if path_obj.is_dir():
            print_success(success_msg)
            print(f"         {path_obj}")
            return path_obj

    print_error(fail_msg)
    return None


def find_file_in_dir_tree(root_dir: str, file_name: str) -> bool:
    root = Path(root_dir)
    for file in root.rglob('*'):
        if file.is_file() and file.name == file_name:
            return True
    return False


def check_files_exist_in_dir(root_dir: str | Path, list_of_files: list[str]) -> bool:
    """Check if all files exist in the given directory.

    Args:
        root_dir: Root directory path.
        list_of_files: List of filenames to check.

    Returns:
        True if all files exist, False otherwise.
    """
    root_path = Path(root_dir)
    did_find_all = True

    for filename in list_of_files:
        file_path = root_path / filename
        if not file_path.exists():
            print_error(f"{file_path} does not exist!")
            did_find_all = False

    return did_find_all

# ----- File system operations help functions -----

def copy_file(src, dst):
    if Path(src).is_file():
        shutil.copy(src, dst)
    else:
        print('Could not copy file: '+src+' it does not exist')


def copy_dir_to(src_dir: str, dst_dir: str) -> bool:
    """Copy a source directory into a destination directory.

    Creates the destination directory if it doesn't exist. The source
    directory name becomes a subdirectory within the destination.

    Args:
        src_dir: Path to the source directory to copy.
        dst_dir: Path to the destination directory.

    Returns:
        True if successful, False otherwise.
    """
    src_path = Path(src_dir).resolve()
    dst_path = Path(dst_dir).resolve()

    if not src_path.is_dir():
        print_error(f"Source directory {src_path} does not exist!")
        return False

    # Create destination if it does not exist
    dst_path.mkdir(parents=True, exist_ok=True)

    tgt_path = dst_path / src_path.name
    if tgt_path.exists():
        print_error(f"Target directory {tgt_path} already exists")
        return False

    print(f"Copying: {src_path} to: {tgt_path}")
    shutil.copytree(src_path, tgt_path)
    return True


def move(src, dst):
    print(f"Moving {quote_path(src)} to {quote_path(dst)}")
    src_path = Path(src)
    dst_path = Path(dst)
    if src_path != dst_path:
        shutil.move(str(src_path), str(dst_path))


def mkdirs(path):
    try:
        os.makedirs(path)
    except OSError:
        if not os.path.isdir(path):
            print_error('Could not create directory path: '+path)

def remove_dir(tgt):
    def del_rw(action, name, exc):
        os.chmod(name, stat.S_IWRITE)
        os.remove(name)

    tgt_path = Path(tgt)
    if tgt_path.exists():
        shutil.rmtree(tgt_path, onerror=del_rw)

def remove_file(tgt):
    """Delete a file if it exists."""
    tgt_path = Path(tgt)
    if tgt_path.is_file():
        tgt_path.unlink()

def set_read_only_for_all_files_in_dir(root_dir):
    root_path = Path(root_dir)
    for dirpath in sorted(root_path.rglob('*')):
        if dirpath.is_dir():
            print(f"Setting files read-only in directory: {dirpath}")
            for file in dirpath.iterdir():
                if file.is_file():
                    file.chmod(stat.S_IREAD)


def write_do_not_save_file_to_all_directories(root_dir):
    root_path = Path(root_dir)
    for dirpath in sorted(root_path.rglob('*')):
        if dirpath.is_dir():
            marker_file = dirpath / '---DO_NOT_SAVE_FILES_IN_THIS_DIRECTORY---'
            print(f"Adding DoNotSaveFile to directory: {dirpath}")
            marker_file.touch()


# ----- Various help functions -----

def replace_pattern(filepath, re_pattern, replacement):
    data = None
    with open(filepath, 'r+') as f:
        data = re.sub(re_pattern, replacement, f.read())
    if data:
        with open(filepath, 'w+') as f:
            f.write(data)


def call_exe(cmd, *args):
    """Execute an external command with arguments."""
    cmd_path = Path(cmd)
    if cmd_path.is_file():
        print([str(cmd_path), *args])
        subprocess.run([str(cmd_path), *args], check=False)
    else:
        print_error(f"{cmd} does not exist!")


def zip_directory(dir_path: str, zip_file_path: str) -> None:
    """Compress a directory into a zip file.

    Args:
        dir_path: Path to the directory to compress.
        zip_file_path: Path where the zip file will be created.
    """
    dir_path = Path(dir_path)
    zip_file_path = Path(zip_file_path)
    zip_file_destination = zip_file_path.stem

    print(f"Compressing directory: {dir_path} into: {zip_file_path}")

    parent = dir_path.parent
    name = dir_path.name
    shutil.make_archive(str(zip_file_destination), 'zip', root_dir=str(parent), base_dir=name)


# ----- Main help functions -----

class BuildToolPaths:
    """Container for all tool paths that the build process needs."""
    inkscape_dir: Path | None = None
    gs_dir: Path | None = None
    doxygen_dir: Path | None = None
    inno_dir: Path | None = None
    mingw_dir: Path | None = None
    msvc2022_path: Path | None = None
    qmake_dir: Path | None = None
    qt_dir: Path | None = None
    jom_dir: Path | None = None

    def verify_paths(self) -> bool:
        print("Verifying and selecting build tool path variables...")
        isOk = True

        # Check if mingw path exists
        self.mingw_dir = select_path_from_list([g_mingw_dir], "Mingw could not be found in the expected location.", "Found MinGW!")
        if self.mingw_dir == "":
            isOk = False

        # Check if Qt path exists
        self.qt_dir = select_path_from_list([g_qmake_dir], "Qt libs could not be found in one of the expected locations.", "Found Qt libs!")
        if self.qt_dir == "":
            isOk = False
        self.qmake_dir = Path(g_qmake_dir)

        # Check if qtcreator path exist
        self.creator_dir = select_path_from_list(qtcreatorDirList, "Qt Creator could not be found in one of the expected locations.", "Found Qt Creator!")
        if self.creator_dir == "":
            isOk = False
        self.jom_dir = self.creator_dir / "bin"

        # Make sure Visual Studio 2022 is installed in correct location
        self.msvc2022_path = select_path_from_list(msvc2022DirList, "Microsoft Windows SDK 7.1 (MSVC2010) is not installed in expected place.", "Found location of Microsoft Windows SDK 7.1 (MSVC2010)!")
        #if self.msvc2022_path == "":
        #    isOk = False

        # Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
        self.inno_dir = select_path_from_list(innoDirList, "Inno Setup is not installed in expected place.", "Found Inno Setup!")
        if self.inno_dir == "":
            isOk = False

        # Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
        self.inkscape_dir = select_path_from_list(inkscapeDirList, "Inkscape is not installed in expected place.", "Found Inkscape!")
        if self.inkscape_dir == "":
            isOk = False

        # Make sure that doxygen is present for documentation build, but we dont care about result just print error if missing
        self.doxygen_dir = select_path_from_list(doxygenDirList, "Doxygen is not installed in expected place.", "Found Doxygen!")

        # Make sure that Ghostscript is present for documentation build, Doxygen seems to require 32-bit version
        self.gs_dir = select_path_from_list(gsDirList, "Ghostscript 32-bit is not installed in expected place.", "Found Ghostscript!")
        if not find_file_in_dir_tree(self.gs_dir, 'gswin32.exe'):
            print_error('You must install the 32-bit version of Ghostscipt, Doxygen is apparently hard-coded for that version')

        if isOk:
            print_success("Verification of tool path variables.")

        return isOk


def extract_hopsan_build_path(arch: str, path_name: str) -> str:
    script_path = Path('dependencies') / 'setHopsanBuildPaths.bat'

    try:
        result = subprocess.run(
            [str(script_path), arch],
            capture_output=True,
            text=True,
            check=False
        )
    except FileNotFoundError:
        return f"Script not found: {script_path}"

    if result.returncode != 0:
        return "Failed to run setHopsanBuildPaths.bat script"

    for line in result.stdout.splitlines():
        if line.startswith(path_name):
            parts = line.split(':', 1)
            if len(parts) == 2:
                return parts[1].strip()

    return f"{path_name} path not found!"


def create_clean_output_directory(package_output_destination: Path) -> bool:
    """Try to remove and recreate the output directory.
    Args:
        package_output_destination: Path object pointing to the desired output directory.
    Returns:
        True if successful, False otherwise.
    """
    # Clear old output folder
    remove_dir(package_output_destination)
    if package_output_destination.exists():
        print_warning("Unable to clear old output folder.")
        if not ask_yes_no_question("Continue? (y/n): "):
            return False

    # Create new output folder
    package_output_destination.mkdir(parents=True, exist_ok=True)
    if not package_output_destination.exists():
        print_error("Failed to create package output folder.")
        return False

    return True


def prepare_source_code(version_number: str, revision_number: str, dev_release: bool) -> None:
    """
    Regenerate the default library, update version macros, and rebuild the splash screen.

    Parameters
    ----------
    version_number: str
        Human readable version (e.g. ``"2.5"``).
    revision_number: str
        Numeric revision (e.g. ``"20240406"``).
    dev_release: bool
        ``True`` for a development build, ``False`` for a production release.
    """

    # ------------------------------------------------------------------
    # Regenerate the default library
    # ------------------------------------------------------------------
    default_lib_dir = g_hopsan_src_dir / "componentLibraries" / "defaultLibrary"
    os.chdir(default_lib_dir)
    call_exe(default_lib_dir / "generateLibraryFiles.bat", "-nopause")
    os.chdir(g_hopsan_src_dir)

    # ------------------------------------------------------------------
    # Temporary splash‑screen copy
    # ------------------------------------------------------------------
    splash_src = g_hopsan_src_dir / "HopsanGUI" / "graphics" / "splash.svg"
    splash_tmp = g_hopsan_src_dir / "HopsanGUI" / "graphics" / "tempdummysplash.svg"
    shutil.copyfile(splash_src, splash_tmp)

    # ------------------------------------------------------------------
    # Version strings
    # ------------------------------------------------------------------
    full_version = f"{version_number}.{revision_number}"

    if not dev_release:
        # Update core / GUI / CLI version macros
        replace_pattern(
            g_hopsan_src_dir / "HopsanCore" / "include" / "HopsanCoreVersion.h",
            r"#define HOPSANCOREVERSION .*",
            f'#define HOPSANCOREVERSION "{full_version}"',
        )
        replace_pattern(
            g_hopsan_src_dir / "HopsanGUI" / "version_gui.h",
            r"#define HOPSANGUIVERSION .*",
            f'#define HOPSANGUIVERSION "{full_version}"',
        )
        replace_pattern(
            g_hopsan_src_dir / "HopsanCLI" / "version_cli.h",
            r"#define HOPSANCLIVERSION .*",
            f'#define HOPSANCLIVERSION "{full_version}"',
        )

        # Hide development warning in the temporary splash file
        replace_pattern(splash_tmp, r"Development version", "")

        # Remove the DEVELOPMENT define in the CMakeList file
        # TODO: Use as CMake option instead maybe
        replace_pattern(
            g_hopsan_src_dir / "HopsanGUI" / "CMakeLists.txt",
            r"target_compile_definitions(${target_name} PRIVATE DEVELOPMENT)",
            r"",
        )

    # Release‑specific macro (always set)
    replace_pattern(
        g_hopsan_src_dir / "HopsanGUI" / "version_gui.h",
        r"#define HOPSANRELEASEVERSION .*",
        f'#define HOPSANRELEASEVERSION "{full_version}"',
    )

    # ------------------------------------------------------------------
    # Update splash‑screen placeholders
    # ------------------------------------------------------------------
    replace_pattern(splash_tmp, r"0\.00\.0", version_number)
    replace_pattern(splash_tmp, r"20170000\.0000", revision_number)

    # ------------------------------------------------------------------
    # Export splash as PNG via Inkscape
    # ------------------------------------------------------------------
    inkscape_exe = g_toolpaths.inkscape_dir / r"inkscape.com"
    svg_path = splash_tmp.as_posix()
    png_path = (g_hopsan_src_dir / "HopsanGUI" / "graphics" / "splash.png").as_posix()

    call_exe(
        inkscape_exe,
        svg_path,
        "--export-background=#ffffff",
        "--export-dpi=90",
        "--export-type=png",
        f"--export-filename={png_path}",
    )

    # ------------------------------------------------------------------
    # Clean up temporary file
    # ------------------------------------------------------------------
    splash_tmp.unlink()


# def msvcCompile(msvcVersion, architecture, msvcpath):
    # print("Compiling HopsanCore with Microsoft Visual Studio "+msvcVersion+" "+architecture+"...")

    # if msvcpath == "":
        # print(r'Error: msvcpath not set!')
        # return False

    # # Remove previous files
    # remove_file(g_hopsan_src_dir+r'\bin\hopsancore*.*')

    # # Create clean build directory
    # hopsanBuildDir = g_hopsan_src_dir+r'\HopsanCore_bd'
    # remove_dir(hopsanBuildDir)
    # mkdirs(hopsanBuildDir)

    # # Create compilation script and compile
    # os.chdir(g_hopsan_src_dir)
    # # Generate compile script, setup compiler and compile
    # mkspec = "win32-msvc"+msvcVersion
    # jom = quotePath(jomDir+r'\jom.exe')
    # qmake = quotePath(qmake_dir+r'\qmake.exe')
    # hopcorepro = quotePath(g_hopsan_src_dir+r'\HopsanCore\HopsanCore.pro')
    # f = open('compileWithMSVC.bat', 'w')
    # f.write(r'echo off'+"\n")
    # f.write(r'REM This file has been automatically generated by the python build script. Do NOT commit it to svn!'+"\n")
    # f.write(r'setlocal enabledelayedexpansion'+"\n")
    # f.write(r'call '+quotePath(msvcpath+r'\SetEnv.cmd')+r' /Release /'+architecture+"\n")
    # f.write(r'COLOR 07'+"\n")
    # f.write(r'cd '+quotePath(hopsanBuildDir)+"\n")
    # f.write(r'call '+jom+r' clean'+"\n")
    # f.write(r'call '+qmake+r' '+hopcorepro+r' -r -spec '+mkspec+r' "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"'+"\n")
    # f.write(r'call '+jom+"\n")
    # f.write(r'cd ..'+"\n")
    # #f.write("pause\n")
    # f.close()

    # # Compile
    # os.system("compileWithMSVC.bat")
    # #print_debug(os.environ["PATH"])

    # #Remove build directory
    # remove_dir(hopsanBuildDir)

    # hopsan_src_dirBin = g_hopsan_src_dir+r'\bin'
    # if not file_exists(hopsan_src_dirBin+r'\hopsancore.dll'):
        # print_error("Failed to build HopsanCore with Visual Studio "+msvcVersion+" "+architecture)
        # return False

    # def makeMSVCOutDirName(version, arch):
        # return "MSVC"+version+"_"+arch

    # #Move files to correct MSVC directory
    # targetDir = hopsan_src_dirBin+"\\"+makeMSVCOutDirName(msvcVersion, architecture)
    # remove_dir(targetDir)
    # mkdirs(targetDir)
    # move(hopsan_src_dirBin+r'\hopsancore.dll', targetDir)
    # move(hopsan_src_dirBin+r'\hopsancore.lib', targetDir)
    # move(hopsan_src_dirBin+r'\hopsancore.exp', targetDir)

    # return True


def build_mingw_release(install_destination: Path) -> bool:
    # ========================================================
    #  Build HOPSANCORE with MSVC
    # ========================================================
    # if buildVCpp:
        # if msvc2022Path != "":
            # if not msvcCompile("2022", "x64", msvc2022Path):
                # return False

    # ========================================================
    #  BUILD WITH MINGW
    # ========================================================
    print("Building with MinGW")

    build_type='Release'
    code_dir=g_hopsan_src_dir
    build_dir = g_temporary_build_root / f'hopsan-release-build-{build_type}'

    # Ensure clean build and install directories
    remove_dir(build_dir)
    remove_dir(install_destination)

    # Generate build script for MinGW
    script_content = f"""echo off
    REM This file was automatically generated by the makeWindowsrelease.py script!
    set PATH={g_mingw_dir};{g_qmake_dir};%PATH%
    set "mingw_path={g_mingw_dir}"
    set "qmake_path={g_qmake_dir}"

    cmake -G"MinGW Makefiles" -DCMAKE_COLOR_MAKEFILE=OFF -DCMAKE_BUILD_TYPE={build_type} -DCMAKE_INSTALL_PREFIX={install_destination} -B{build_dir} -S{code_dir} --fresh
    cmake --build {build_dir} --config {build_type} --parallel 16
    cmake --build {build_dir} --config {build_type} --parallel 16 --target install
    REM ctest -C {build_dir} --output-on-failure --parallel 8
    """

    build_script = Path('compile-with-MinGW.bat')
    build_script.write_text(script_content)
    try:
        subprocess.run(
            str(build_script),
            shell=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print_error(f"Build failed with exit code {e.returncode}")

    required_files = [
        install_destination / 'bin' / 'hopsancore.dll',
        install_destination / 'bin' / 'hopsangui.exe',
        install_destination / 'bin' / 'hopsancli.exe',
    ]
    if not all(file.exists() for file in required_files):
        print_error("Failed to build Hopsan with MinGW.")
        return False

    print_success("Building with MinGW")
    return True


def finalize_install_dir(hopsan_installation_destination):
    # Write the do not save files here file
    write_do_not_save_file_to_all_directories(hopsan_installation_destination)
    # Set all files to read-only
    set_read_only_for_all_files_in_dir(hopsan_installation_destination)
    return True


def create_zip_package(hopsan_installation_destination: str, zip_file: str, output_dir: str) -> bool:
    """Create a zip package and move it to the output directory.

    Args:
        zip_file: Name of the zip file to create.
        output_dir: Directory where the zip file will be moved.

    Returns:
        True if successful, False otherwise.
    """
    print(f"Creating zip package: {zip_file}...")
    zip_directory(hopsan_installation_destination, zip_file)
    move(zip_file, output_dir)

    output_path = Path(output_dir) / zip_file
    if not output_path.exists():
        print_error(f"Failed to create zip package: {zip_file}")
        return False

    print_success(f"Created zip package: {zip_file} successfully!")
    return True


def create_inno_installer(hopsan_installation_destination: str, exe_file_name: str, inno_arch: str, output_dir: str) -> bool:
    """Generate an Inno Setup installer executable.

    Args:
        exe_file_name: Name of the executable file (without .exe extension).
        inno_arch: Target architecture (e.g., "x64", "x86").
        output_dir: Directory where the installer will be created.

    Returns:
        True if successful, False otherwise.
    """
    exe_file = f"{exe_file_name}.exe"
    print(f"Generating install executable: {exe_file}...")

    inno_args = [
        f'/o{output_dir}',
        f'/f{exe_file_name}',
        f'/dMyAppVersion={gFullVersion}',
        f'/dMyArchitecture={inno_arch}',
        f'/dMyFilesSource={hopsan_installation_destination}',
        str(g_hopsan_src_dir / "packaging/windows/HopsanReleaseInnoSetupScript.iss")
    ]

    call_exe(g_toolpaths.inno_dir / "iscc.exe", *inno_args)

    output_path = Path(output_dir) / exe_file
    if not output_path.exists():
        print_error(f"Failed to create installer executable: {exe_file}")
        return False

    print_success(f"Generated install executable: {exe_file}")
    return True


def create_installation_packages(hopsan_installation_destination: Path, package_output_destination: Path):

    # Make sure we are in the hopsan root
    os.chdir(g_hopsan_src_dir)

    zip_file = f"Hopsan-{gReleaseFileVersionName}-win64-zip.zip"
    zip_with_compiler_file = f"Hopsan-{gReleaseFileVersionName}-win64-with_compiler-zip.zip"
    exe_file_name = f"Hopsan-{gReleaseFileVersionName}-win64-installer"
    exe_with_compiler_file_name = f"Hopsan-{gReleaseFileVersionName}-win64-with_compiler-installer"
    inno_arch = "x64"

    # Create zip package
    if not create_zip_package(hopsan_installation_destination, zip_file, package_output_destination):
        return False

    # Execute Inno compile script
    if not create_inno_installer(hopsan_installation_destination, exe_file_name, inno_arch, package_output_destination):
        return False

    # Copy the compiler
    if gIncludeCompiler:
        print("Copying compiler...")
        mingw_parent = Path(g_mingw_dir).parent
        copy_dir_to(str(mingw_parent), hopsan_installation_destination)

        mingw_dir_name = mingw_parent.name
        move(
            str(Path(hopsan_installation_destination) / mingw_dir_name),
            str(Path(hopsan_installation_destination) / "mingw64")
        )

        #print('Removing /opt')
        #remove_dir(gTemporaryBuildDir+r'\mingw64\opt')
        # Now build zip and installer with compiler included
        if not create_zip_package(hopsan_installation_destination, zip_with_compiler_file, package_output_destination):
            return False
        if not create_inno_installer(hopsan_installation_destination, exe_with_compiler_file_name, inno_arch, package_output_destination):
            return False

    # Copy release notes to output directory
    copy_file('Hopsan-release-notes.txt', package_output_destination)

    return True


def run_validation(hopsan_installation_destination: Path) -> bool:
    """Run validation tests and return success status."""
    print("Running validation tests")
    test_script = g_hopsan_src_dir / "runValidationTests.bat"
    result = subprocess.run(
        f"{test_script} nopause",
        shell=True,
        cwd=hopsan_installation_destination,
        check=False
    )
    return result.returncode == 0


def cleanup():
    """Remove temporary build and install directories"""
    print(f"Cleaning up, removing: {g_temporary_build_root}")
    remove_dir(g_temporary_build_root)


###############################################################################
# Execution of main begins here
###############################################################################

print("""
/------------------------------------------------------------\\
| HOPSAN RELEASE BUILD AND PACKAGING SCRIPT                  |
|                                                            |
\\------------------------------------------------------------/
""")

set_color(BColors.WHITE)

g_hopsan_src_dir = Path(os.getcwd())
g_mingw_dir = extract_hopsan_build_path('x64', 'mingw')
g_qmake_dir = extract_hopsan_build_path('x64', 'qmake')
g_toolpaths = BuildToolPaths()

gDoDevRelease = False
gIncludeCompiler = False

pauseOnFailValidation = False
doBuild = True
success = True

if not g_toolpaths.verify_paths():
    success = False
    print_error("Make release script failed while verifying paths.")

qt_bins_ok = check_files_exist_in_dir(g_toolpaths.qmake_dir, qtRuntimeBins)
qt_plugins_ok = check_files_exist_in_dir(g_toolpaths.qmake_dir.parent / "plugins", qtPluginBins)
mingw_bins_ok = check_files_exist_in_dir(g_toolpaths.mingw_dir, mingwBins)
mingw_optbins_ok = check_files_exist_in_dir(g_toolpaths.mingw_dir.parent / "opt" / "bin", mingwOptBins)
deps_ok = check_files_exist_in_dir(Path(g_hopsan_src_dir) / "dependencies", dependencyFiles)

success = success and all([
    qt_bins_ok,
    qt_plugins_ok,
    mingw_bins_ok,
    mingw_optbins_ok,
    deps_ok
])

if not success:
    print_error("Make release script could not find all needed files.")

if success:
    print("\n")
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

    print(f"""
    ---------------------------------------
    This is a DEV release: {gDoDevRelease}
    Release file version name: {gReleaseFileVersionName}
    Release revision number: {gReleaseRevision}
    Release full version number: {gFullVersion}
    Build MSVC based release: {buildVCpp}
    Include compiler: {gIncludeCompiler}
    Pause on failed validation: {pauseOnFailValidation}
    ---------------------------------------
    """)

    if askYesNoQuestion("Is this OK? (y/n): "):
        success = True
    else:
        print_error("Aborted by user.")
        success = False

hopsan_installation_destination = g_temporary_build_root / Path(f"Hopsan-{gReleaseFileVersionName}-win64")
package_output_dir = g_hopsan_src_dir / "output"

print(f"Using TempDir: {g_temporary_build_root}")
print(f"Installing to: {hopsan_installation_destination}")
print(f"Saving packages to: {package_output_dir}")

if success:
    if not create_clean_output_directory(package_output_dir):
        success = False
        cleanup()

if success:
    prepare_source_code(gBaseVersion, gReleaseRevision, gDoDevRelease)
    if doBuild:
        if not build_mingw_release(hopsan_installation_destination):
            success = False
            cleanup()
            print_error("Make release script failed with build error.")

if success:
    if not finalize_install_dir(hopsan_installation_destination):
        success = False
        cleanup()
        print_error("Make release script failed when finalizing files.")

if success:
    if not create_installation_packages(hopsan_installation_destination, package_output_dir):
        success = False
        cleanup()
        print_error("Make release script failed while generating installation packages.")

if success:
    if not run_validation(hopsan_installation_destination) and pauseOnFailValidation:
        print_warning("Make release script failed in model validation.")
        askYesNoQuestion("Press enter to continue!")

if success:
    print_success(f"Make release script finished successfully. The release packages can be found in: {package_output_dir}")
    cleanup()

input("Press any key to continue...")
