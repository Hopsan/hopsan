#!/bin/bash
# $Id$

# Shell script for building DEB packages of hopsan for multiple distributions using pbuilder
# Author: Peter Nordin peter.nordin@liu.se

set -u
#set -e

#--------------------------------------------------------------------------------------------------
# Configuration starts here
#--------------------------------------------------------------------------------------------------
hopsancode_root=$(pwd)
pbuilderWorkDir=/var/tmp/deb_hopsan/pbuilder
name=hopsan
devversion=2.12.0

# Pbuilder dists and archs
debianDistArchArray=( buster:amd64:buster buster:i386:buster stretch:amd64:qt5py27 stretch:i386:qt5py27 jessie:amd64:qt5py27 jessie:i386:qt5py27 )
ubuntuDistArchArray=( eoan:amd64:qt5py3sysdeps eoan:i386:qt5py3sysdeps disco:amd64:qt5py3sysdeps disco:i386:qt5py3sysdeps bionic:amd64:qt5py3 bionic:i386:qt5py3 xenial:amd64:qt5py27 xenial:i386:qt5py27 trusty:amd64:trusty trusty:i386:trusty )

# Pbuilder mirrors
ubuntuMirror=http://se.archive.ubuntu.com/ubuntu/
debianMirror=http://ftp.se.debian.org/debian/

#--------------------------------------------------------------------------------------------------
# Code starts here
#--------------------------------------------------------------------------------------------------

echo Using Hopsan root: ${hopsancode_root}
echo Enter sudo password for pbuilder:
sudo -v

# Ask yes/no question, returning true or false in the global variable boolYNQuestionAnswer
function boolAskYNQuestion {
  declare -g boolYNQuestionAnswer=false
  echo -n "$1" Answer y/n ["$2"]:
  read -r ans
  if [[ -z "$ans" ]]; then
    if [[ "$2" == "y" ]]; then
      boolYNQuestionAnswer=true
    fi
  elif [[ "$ans" == "y" ]]; then
    boolYNQuestionAnswer=true
  fi
}

function ask_question {
  local -r question="$1"
  local -r default="$2"
  declare -g questionAnswer=$default
  echo -n $question [$default]:
  read -r ans
  if [[ ! -z "$ans" ]]; then
    questionAnswer=$ans
  fi
}

# Export git directory including submodules
function git_export_all {
  set -e
  local -r src=$1
  local -r dst=$2
  local -r tarfile=hopsan_git_export.tar
  echo "Exporting from git: ${src} to ${dst}"
  mkdir -p ${dst}
  pushd ${src}
  ${hopsancode_root}/Dependencies/git-archive-all/git_archive_all.py ${tarfile}
  popd
  mv ${src}/${tarfile} ${dst}
  pushd ${dst}
  tar -xf ${tarfile} --strip-components=1
  rm ${tarfile}
  popd
  set +e
}


dscFile="NoFile"
function builDSCFile {
  local -r confdir="$1"
  local -r packagedir="$2"
  local -r packageorigsrcfile="$3"
  local -r config="$4"
  local -r usepythonqt="$5"
  local -r outputbasename="$6"

  echo ---------------------------------
  echo Build dsc package:
  echo Source file: $packageorigsrcfile
  echo Packagedir : $packagedir
  echo Output name: $outputbasename
  echo Confdir    : $confdir
  echo Config     : $config
  echo With Python: $doUsePythonQt
  echo ---------------------------------


  # First clear packaging dir if it already exist
  rm -rf ${packagedir}

  # Copy package configuration
  # TODO Use install
  mkdir -p ${packagedir}
  cp -a ${confdir}/debconfig_base/debian $packagedir
  cp -a ${confdir}/debconfig_${config}/debian $packagedir

  # Check if we should remove PythonQt if it should not be used
  if [[ $usepythonqt == false ]]; then
    sed "/setupPythonQt.sh/d" -i "${packagedir}"/debian/rules
  fi

  # Copy "unpack" prepared source files to this dir
  tar -xzf ${packageorigsrcfile} -C ${packagedir}

  # Generate NEW change log file for this release version with no content in particular
  pushd ${packagedir}
  rm debian/changelog
  dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
  dch -p -m --release ""
  popd

  # Generate dsc source file
  dpkg-source --before-build ${packagedir}
  dpkg-source -b ${packagedir}
  dscFile=$(ls ${outputbasename}*.dsc)
}

# -----------------------------------------------------------------------------
# Main
#

# Ask user for version input
echo
echo -n "Enter release version number or git tag on the form a.b.c, vA.B.C or leave blank for DEV build release: "
read -r baseversion_or_tag
doDevRelease=false
if [[ -z "$baseversion_or_tag" ]]; then
  doDevRelease=true
  baseversion_or_tag=${devversion}
fi
readonly releaserevision=$(./getGitInfo.sh date.time .)
readonly baseversion=$(echo ${baseversion_or_tag} | sed 's/[^0-9\.]//g')
readonly fullversionname=${baseversion}.${releaserevision}

readonly tag_regex="^v.*"
if [[ ${baseversion_or_tag} =~ ${tag_regex} ]]; then
  ask_question "Enter git URL: " https://github.com/Hopsan/hopsan.git
  readonly hopsancode_url=$questionAnswer
fi

echo
boolAskYNQuestion "Do you want the defaultComponentLibrary to be build in?" n
readonly doBuildInComponents=${boolYNQuestionAnswer}

echo
boolAskYNQuestion "Do you want to build with PythonQt and python support?" y
readonly doUsePythonQt=${boolYNQuestionAnswer}

echo
distArchArrayDo=()
for i in "${debianDistArchArray[@]}"; do
  boolAskYNQuestion "Do you want to build, $i?" n
  distArchArrayDo+=("$i"":""D"":""$boolYNQuestionAnswer")
done
for i in "${ubuntuDistArchArray[@]}"; do
  boolAskYNQuestion "Do you want to build, $i?" n
  distArchArrayDo+=("$i"":""U"":""$boolYNQuestionAnswer")
done

echo
echo ---------------------------------------
echo This is a DEV release: $doDevRelease
echo Release baseversion number: $baseversion
echo Release revision number: $releaserevision
echo Release version name: $fullversionname
echo Built in components: $doBuildInComponents
echo Using PythonQt: $doUsePythonQt
printf "%s\n" "${distArchArrayDo[@]}"
echo ---------------------------------------
boolAskYNQuestion "Is this OK?" n
if [ "$boolYNQuestionAnswer" = false ]; then
  echo Aborting!
  exit 1
fi

echo

# -----------------------------------------------------------------------------
# Determine output directory and file names
#
outputDir=${hopsancode_root}/output_deb
outputDebDir=${outputDir}/debs
outputfile_basename=${name}_${fullversionname}
packageorigsrcfile=${outputfile_basename}.orig.tar.gz
package_dirname=${name}-${fullversionname}
stage_directory=${outputDir}/hopsanStage_${fullversionname}

mkdir -p $outputDebDir
mkdir -p $pbuilderWorkDir
pushd ${outputDir}


# -----------------------------------------------------------------------------
# Clone or export source code for a clean build, unless src and dst directories are the same
#
if [[ ${baseversion_or_tag} =~ ${tag_regex} ]]; then
  echo Cloning from ${hopsancode_url} into ${stage_directory}
  rm -rf ${stage_directory}
  git clone -b ${baseversion_or_tag} --depth 1 ${hopsancode_url} ${stage_directory}
  if [[ $? -ne 0 ]]; then
    echo Error: Failed to clone from git
    exit 1
  fi
  pushd ${stage_directory}
  git submodule update --init --recommend-shallow
  if [[ $? -ne 0 ]]; then
    echo Error: Failed to clone submodules from git
    exit 1
  fi
  popd
  readonly hopsancode_gitdir=${stage_directory}
elif [[ "${hopsancode_root}" == "${stage_directory}" ]]; then
  echo Source code and stage directory are the same. Not exporting code! Building in source code directory!
  readonly hopsancode_gitdir=${hopsancode_root}
else
  echo Exporting ${hopsancode_root} to ${stage_directory} for preparation
  rm -rf ${stage_directory}
  git_export_all ${hopsancode_root} ${stage_directory}
  readonly hopsancode_gitdir=${hopsancode_root}
fi


# -----------------------------------------------------------------------------
# Prepare source code inside stage directory and package it into original source package
#
set -e
${stage_directory}/packaging/prepareSourceCode.sh ${hopsancode_gitdir} ${stage_directory} \
                                                        ${baseversion} ${releaserevision} ${fullversionname} \
                                                        ${doDevRelease} ${doBuildInComponents}
set +e
# Remove .git directory and submodule files (if present) before packaging source code
find ${stage_directory} -name .git -exec rm -rf {} \;
# Package source code
tar -czf ${packageorigsrcfile} -C ${stage_directory} .


# -----------------------------------------------------------------------------
# Now build DEB package
#
buildStatusArray=()
for i in "${distArchArrayDo[@]}"; do
  # Split input into array to extract data
  IFS=':' read -ra arr <<< "$i"
  dist="${arr[0]}"
  arch="${arr[1]}"
  conf="${arr[2]}"
  distBase="${arr[3]}"
  doBuild="${arr[4]}"

  if [ "$doBuild" = "true" ]; then
    echo
    echo "==========================="
    echo "Building for $dist $arch"
    echo "==========================="
    sleep 1

    readonly debian_conf_root=${stage_directory}/packaging/deb
    builDSCFile ${debian_conf_root} $package_dirname $packageorigsrcfile $conf $doUsePythonQt $outputfile_basename

    doCreateUpdatePbuilderBaseTGZ=true
    basetgzFile=${pbuilderWorkDir}/${dist}${arch}.tgz
    pbuilderBuildDir=${pbuilderWorkDir}/build
    mkdir -p ${pbuilderBuildDir}
    resultPath=${pbuilderWorkDir}/result/${dist}
    mkdir -p ${resultPath}
    logFile=${resultPath}/${outputfile_basename}.log

    # Set pbuild options specific to ubuntu or debian
    if [ "$distBase" = "U" ]; then
      optsComponents="main universe"
      optsMirror="$ubuntuMirror"
      optsDebootstrap="--debootstrapopts --keyring=/usr/share/keyrings/ubuntu-archive-keyring.gpg"
    elif [ "$distBase" = "D" ]; then
      optsComponents="main contrib"
      optsMirror="$debianMirror"
      optsDebootstrap="--debootstrapopts --keyring=/usr/share/keyrings/debian-archive-keyring.gpg"
    fi

    # Debug
    #echo $optsComponents
    #echo $optsMirror
    #echo $optsDebootstrap
    #sleep 2

    # Extract build dependencies from control file
    # Installs them once and for all to avoid wasting time on every build
    extraPackages=$(grep Build-Depends ${debian_conf_root}/debconfig_${conf}/debian/control | sed -e 's/Build-Depends://' -e 's/[(][^)]*[)]//g' -e 's/\,//g')
    #echo $extraPackages

    # Remove file if it is corrupt
    if [ -f ${basetgzFile} ]; then
      if ! gunzip -t $basetgzFile ; then
        rm -f "${basetgzFile}"
      fi
    fi

    # Check if its time to update
    if [[ -f ${basetgzFile} ]]; then
      age=$(( $(date +%s) - $(stat -c %Y ${basetgzFile}) ))
      #echo "age: $age"
      if [[ ${age} -le $((3600*6)) ]]; then
        doCreateUpdatePbuilderBaseTGZ=false
      fi
    fi

    # Update or create pbuild environments
    debootstrapOk=true
    if [[ $doCreateUpdatePbuilderBaseTGZ == true ]]; then
      if [ -f $basetgzFile ]; then
        echo
        echo "Updating existing TGZ: $basetgzFile"
        echo "------------------------"
        sudo pbuilder --update  --basetgz "${basetgzFile}" --extrapackages "${extraPackages}" --aptcache "" --buildplace "${pbuilderBuildDir}"
      else
        echo
        echo "Creating new TGZ: $basetgzFile"
        echo "------------------------"
        sudo pbuilder --create --distribution ${dist} --architecture ${arch} --mirror ${optsMirror} ${optsDebootstrap} --components "${optsComponents}" --basetgz "${basetgzFile}" --extrapackages "${extraPackages}" --aptcache "" --buildplace "${pbuilderBuildDir}"
      fi
      # Check for success
      if [ $? -ne 0 ]; then
        debootstrapOk=false
        buildStatusArray+=("$dist""_""$arch"":DebootstrapFailed")
        echo "pubulider create or update FAILED! for $dist $arch, aborting!"
      fi
    fi

    if [ "$debootstrapOk" = true ]; then
      # Now build source package
      echo
      echo "Building with pbuilder"
      echo "------------------------"
      sudo pbuilder --build --basetgz "${basetgzFile}" --aptcache "" --buildplace "$pbuilderBuildDir" --logfile "$logFile" --buildresult $resultPath $dscFile

      # Figure out the actual output file name
      outputDebName=$(ls ${resultPath}/${outputfile_basename}*_${arch}.deb)

      # Check if it exist (success)
      if [ -f "$outputDebName" ]; then
        buildStatusArray+=("$dist""_""$arch"":BuildOk")

        # Now move and rename output deb file to deb output dir
        resultingDebFile=${outputDebDir}/${outputfile_basename}_${dist}_${arch}.deb
        mv ${outputDebName} ${resultingDebFile}

        # Check package with lintian ('|| true' avoids script abort on errors)
        lintian --no-tag-display-limit --color always -X files ${resultingDebFile} || true
      else
        buildStatusArray+=("$dist""_""$arch"":BuildFailed")
      fi
    fi
  fi
done

echo
echo "Build Status:"
echo "-------------"

printf "%s\n" "${buildStatusArray[@]}"

# Cleanup
rm -rf ${stage_directory}

echo
echo Done!
