#!/bin/bash
# $Id$

# Shell script for preparing the Hopsan source code before RELEASE build
# Author: Peter Nordin peter.nordin@liu.se

git_export_all()
{
  set -e
  local src=$1
  local dst=$2
  local tarfile=hopsan_git_export.tar
  echo "Exporting from git: ${src} to ${dst}"
  mkdir -p ${dst}
  pushd ${src}
  ${src_hopsan_root}/Dependencies/git-archive-all/git_archive_all.py ${tarfile}
  popd
  mv ${src}/${tarfile} ${dst}
  pushd ${dst}
  tar -xf ${tarfile} --strip-components=1
  rm ${tarfile}
  popd
  set +e
}


E_BADARGS=65
if [ $# -lt 7 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {src_hopsan_root dst_directory base_version release_revision full_version_string doDevRelease doBuildInComponents}"
  exit $E_BADARGS
fi

readonly src_hopsan_root="$1"
readonly dst_directory="$2"
readonly base_version="$3"
readonly release_revision="$4"
readonly full_version_string="$5"
readonly doDevRelease="$6"
readonly doBuildInComponents="$7"

# -----------------------------------------------------------------------------
# Determine the Core Gui and CLI revision numbers and hashes
#
cd ${src_hopsan_root}/HopsanCore; corecommitdt=$(../getGitInfo.sh date.time .); cd $OLDPWD
cd ${src_hopsan_root}/HopsanCore; corecommithash=$(../getGitInfo.sh shorthash .); cd $OLDPWD
cd ${src_hopsan_root}/HopsanGUI; guicommitdt=$(../getGitInfo.sh date.time .); cd $OLDPWD
cd ${src_hopsan_root}/HopsanCLI; clicommitdt=$(../getGitInfo.sh date.time .); cd $OLDPWD
echo "Core_CDT: ${corecommitdt}, GUI_CDT: ${guicommitdt}, CLI_CDT: ${clicommitdt}"

# -----------------------------------------------------------------------------
# Export source code for a clean build, unless src and dst directories are the same 
#
if [[ ${src_hopsan_root} = ${dst_directory} ]]; then
  echo "Source and destination are the same. Not exporting!"
else
  echo "Exporting $src_hopsan_root to $dst_directory for preparation"
  rm -rf ${dst_directory}
  git_export_all ${src_hopsan_root} ${dst_directory}
fi

# -----------------------------------------------------------------------------
# Prepare files in destination directory
#
cd ${dst_directory}

# Generate default library files
cd componentLibraries/defaultLibrary; ./generateLibraryFiles.py .; cd $OLDPWD

# Set the Core Gui and CLI rev numbers, hashes and release version for this release
sed "s|#define HOPSANCORE_COMMIT_HASH .*|#define HOPSANCORE_COMMIT_HASH ${corecommithash}|g" -i HopsanCore/include/HopsanCoreGitVersion.h
sed "s|#define HOPSANCORE_COMMIT_TIMESTAMP .*|#define HOPSANCORE_COMMIT_TIMESTAMP ${corecommitdt}|g" -i HopsanCore/include/HopsanCoreGitVersion.h
sed "s|#define HOPSANGUI_COMMIT_TIMESTAMP .*|#define HOPSANGUI_COMMIT_TIMESTAMP ${guicommitdt}|g" -i HopsanGUI/version_gui.h
sed "s|#define HOPSANCLI_COMMIT_TIMESTAMP .*|#define HOPSANCLI_COMMIT_TIMESTAMP ${clicommitdt}|g" -i HopsanCLI/version_cli.h

sed "s|#define HOPSANRELEASEVERSION .*|#define HOPSANRELEASEVERSION \"${full_version_string}\"|g" -i HopsanGUI/version_gui.h

# Handle official release preparation
if [[ $doDevRelease = "false" ]]; then
  # Overwrite version numbers for official release so that allcomponents show the same version
  sed "s|#define HOPSANCOREVERSION .*|#define HOPSANCOREVERSION \"${full_version_string}\"|g" -i HopsanCore/include/HopsanCoreVersion.h
  sed "s|#define HOPSANGUIVERSION .*|#define HOPSANGUIVERSION \"${full_version_string}\"|g" -i HopsanGUI/version_gui.h
  sed "s|#define HOPSANCLIVERSION .*|#define HOPSANCLIVERSION \"${full_version_string}\"|g" -i HopsanCLI/version_cli.h

  # Hide splash screen development warning
  sed "s|Development version||g" -i HopsanGUI/graphics/splash.svg

  # Make sure development flag is not defined
  sed "s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|g" -i HopsanGUI/HopsanGUI.pro
fi

# Set splash screen version number
sed "s|0\.0\.0|$base_version|g" -i HopsanGUI/graphics/splash.svg
sed "s|20170000\.0000|$release_revision|g" -i HopsanGUI/graphics/splash.svg
inkscape ./HopsanGUI/graphics/splash.svg --export-background=rgb\(255,255,255\) --export-dpi=90 --export-png ./HopsanGUI/graphics/splash.png

# If selected, make changes to compile defaultLibrary into Hopsan Core
# Deprecated, we should not do this anymore
if [[ "$doBuildInComponents" = "true" ]]; then
  sed 's|.*DEFINES \*= HOPSAN_INTERNALDEFAULTCOMPONENTS|DEFINES *= HOPSAN_INTERNALDEFAULTCOMPONENTS|g' -i Common.prf
  sed 's|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\|g' -i HopsanCore/HopsanCore.pro
  sed '/.*<lib>.*/d' -i componentLibraries/defaultLibrary/defaultComponentLibrary.xml
  sed 's|componentLibraries||g' -i HopsanNG.pro
fi

# Build user documentation
./buildDocumentation.sh
