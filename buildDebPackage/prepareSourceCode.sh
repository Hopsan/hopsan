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
  echo "Usage: `basename $0` {hopsancode_root stage_directory base_version release_revision full_version_string doDevRelease doBuildInComponents}"
  exit $E_BADARGS
fi

readonly hopsancode_root="$1"
readonly stage_directory="$2"
readonly base_version_or_tag="$3"
readonly release_revision="$4"
readonly full_version_string="$5"
readonly doDevRelease="$6"
readonly doBuildInComponents="$7"
if [[ "$OSTYPE" == "darwin"* ]]; then
  inkscape_cmd="/Applications/Inkscape.app/Contents/MacOS/Inkscape"
else
  inkscape_cmd=inkscape
fi

giturl_regex='(https)://.*(.git)'
if [[ ${hopsancode_root} =~ ${giturl_regex} ]]; then
  readonly base_version=$(echo ${base_version_or_tag} | sed 's/[^0-9\.]//g')
  readonly code_git_dir=${stage_directory}
else
  readonly base_version=${base_version_or_tag}
  readonly code_git_dir=${hopsancode_root}
fi

echo base_Version $base_version
echo stage_directory $stage_directory

# -----------------------------------------------------------------------------
# Clone or export source code for a clean build, unless src and dst directories are the same
#
if [[ ${hopsancode_root} =~ ${giturl_regex} ]]; then
  echo Cloning from ${hopsancode_root} into ${stage_directory}
  rm -rf ${stage_directory}
  git clone -b ${base_version_or_tag} --depth 1 ${hopsancode_root} ${stage_directory}
  pushd ${stage_directory}
  git submodule update --init --recommend-shallow
  popd
  if [[ $? -ne 0 ]]; then
    echo Error: Failed to clone from git
    exit 1
  fi
elif [[ ${hopsancode_root} == ${stage_directory} ]]; then
  echo Source code and stage directory are the same. Not exporting code! Building in source code directory!
else
  echo Exporting $hopsancode_root to $stage_directory for preparation
  rm -rf ${stage_directory}
  git_export_all ${hopsancode_root} ${stage_directory}
fi

# -----------------------------------------------------------------------------
# Determine the Core Gui and CLI revision numbers and hashes
#
pushd ${code_git_dir}/HopsanCore; corecommitdt=$(../getGitInfo.sh date.time .); popd
pushd ${code_git_dir}/HopsanCore; corecommithash=$(../getGitInfo.sh shorthash .); popd
pushd ${code_git_dir}/HopsanGUI; guicommitdt=$(../getGitInfo.sh date.time .); popd
pushd ${code_git_dir}/HopsanCLI; clicommitdt=$(../getGitInfo.sh date.time .); popd
echo "Core_CDT: ${corecommitdt}, GUI_CDT: ${guicommitdt}, CLI_CDT: ${clicommitdt}"

# -----------------------------------------------------------------------------
# Prepare files in destination directory
#
pushd ${stage_directory}

# Generate default library files
pushd componentLibraries/defaultLibrary; ./generateLibraryFiles.py .; popd

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
if [[ $(command -v ${inkscape_cmd} > /dev/null) -eq 0 ]]; then
  ${inkscape_cmd} ./HopsanGUI/graphics/splash.svg --export-background=rgb\(255,255,255\) --export-dpi=90 --export-png ./HopsanGUI/graphics/splash.png
elif [[ $(command -v convert > /dev/null) -eq 0 ]]; then
  echo Warning: Inkscape is not available, falling back to convert
  convert -background white ./HopsanGUI/graphics/splash.svg ./HopsanGUI/graphics/splash.png
else
  echo Error: Neither Inkscape or convert can be used to generate splash screen
fi

# If selected, make changes to compile defaultLibrary into Hopsan Core
# Deprecated, we should not do this anymore
if [[ "$doBuildInComponents" = "true" ]]; then
  sed 's|.*DEFINES \*= HOPSAN_INTERNALDEFAULTCOMPONENTS|DEFINES *= HOPSAN_INTERNALDEFAULTCOMPONENTS|g' -i Common.prf
  sed 's|#INTERNALCOMPLIB.CPP#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cpp \\|g' -i HopsanCore/HopsanCore.pro
  sed '/.*<lib>.*/d' -i componentLibraries/defaultLibrary/defaultComponentLibrary.xml
  sed 's|componentLibraries||g' -i HopsanNG.pro
fi

# Build user documentation
./buildDocumentation.sh
