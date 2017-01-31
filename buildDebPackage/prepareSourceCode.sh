#!/bin/bash
# $Id$

# Shell script for exporting and preparing the Hopsan source code before RELEASE build
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-01
# For use in Hopsan, requires "subversion command line" installed (apt-get install subversion)

git_export_all()
{
  local src=$1
  local dst=$2
  local olddir=$(pwd)
  local tarfile=hopsan_git_export.tar
  echo "Exporting from git: ${src} to ${dst}"
  mkdir -p ${dst}
  cd ${src}
  ${hopsanroot}/git_archive_all.py ${tarfile}
  cd $olddir
  mv ${src}/${tarfile} ${dst}
  cd ${dst}
  tar -xf ${tarfile} --strip-components=1
  rm ${tarfile}
  cd $olddir
}


E_BADARGS=65
if [ $# -lt 5 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir baseversion releaserevision fullversionname doDevRelease doBuildInComponents}"
  exit $E_BADARGS
fi

srcDir="$1"
hopsanroot="$1"
dstDir="$2"
baseversion="$3"
releaserevision="$4"
fullversionname="$5"
doDevRelease="$6"
doBuildInComponents="$7"

# -----------------------------------------------------------------------------
# Determine the Core Gui and CLI svn rev numbers for this release
#

cd $srcDir/HopsanCore; corecommitdt=`../getGitInfo.sh date.time .`; cd $OLDPWD
cd $srcDir/HopsanCore; corecommithash=`../getGitInfo.sh shorthash .`; cd $OLDPWD
cd $srcDir/HopsanGUI; guicommitdt=`../getGitInfo.sh date.time .`; cd $OLDPWD
cd $srcDir/HopsanCLI; clicommitdt=`../getGitInfo.sh date.time .`; cd $OLDPWD
echo "Core_CDT: ${corecommitdt}, GUI_CDT: ${guicommitdt}, CLI_CDT: ${clicommitdt}"

# -----------------------------------------------------------------------------
# Export source dirs and files
#
echo "Exporting $srcDir to $dstDir for preparation"
rm -rf $dstDir
git_export_all $srcDir $dstDir 


# -----------------------------------------------------------------------------
# Prepare files
#
cd $dstDir

# Clean bin folder
#rm -rf ./bin/*

# Generate default library files
cd componentLibraries/defaultLibrary; ./generateLibraryFiles.py .; cd $OLDPWD

# Set the Core Gui and CLI svn rev numbers for this release
sed "s|#define HOPSANCORE_COMMIT_HASH.*|#define HOPSANCORE_COMMIT_HASH ${corecommithash}|g" -i HopsanCore/include/HopsanCoreGitVersion.h
sed "s|#define HOPSANCORE_COMMIT_TIMESTAMP.*|#define HOPSANCORE_COMMIT_TIMESTAMP ${corecommitdt}|g" -i HopsanCore/include/HopsanCoreGitVersion.h
sed "s|#define HOPSANGUI_COMMIT_TIMESTAMP.*|#define HOPSANGUI_COMMIT_TIMESTAMP ${guicommitdt}|g" -i HopsanGUI/version_gui.h
sed "s|#define HOPSANCLI_COMMIT_TIMESTAMP.*|#define HOPSANCLI_COMMIT_TIMESTAMP ${clicommitdt}|g" -i HopsanCLI/version_cli.h

if [ $doDevRelease = "false" ]; then
  # Set version numbers (by changing .h files) BEFORE build
  sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"$fullversionname\"|g" -i HopsanCore/include/HopsanCoreVersion.h
  sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"$fullversionname\"|g" -i HopsanGUI/version_gui.h

  # Hide splash screen development warning
  sed "s|Development version||g" -i HopsanGUI/graphics/splash2.svg

  # Make sure development flag is not defined
  sed "s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|g" -i HopsanGUI/HopsanGUI.pro
fi

# Set splash screen version number
sed "s|X\.X\.X|$baseversion|g" -i HopsanGUI/graphics/splash2.svg
sed "s|R\.R\.R|$releaserevision|g" -i HopsanGUI/graphics/splash2.svg
inkscape ./HopsanGUI/graphics/splash2.svg --export-background=rgb\(255,255,255\) --export-png ./HopsanGUI/graphics/splash.png

# Make sure we compile defaultLibrary into core
if [ "$doBuildInComponents" = "true" ]; then
  sed 's|.*DEFINES \*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g' -i Common.prf
  sed 's|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\|g' -i HopsanCore/HopsanCore.pro
  sed '/.*<lib>.*/d' -i componentLibraries\defaultLibrary\defaultComponentLibrary.xml
  sed 's|componentLibraries||g' -i HopsanNG.pro
fi

# Build user documentation
./buildDocumentation.sh
