#!/bin/bash

# Shell script for building DEB packages of hopsan for multiple distributions using pbuilder

set -u
#set -e

#--------------------------------------------------------------------------------------------------
# Configuration starts here
#--------------------------------------------------------------------------------------------------
hopsancode_root=$(pwd)
pbuilderWorkDir=/var/tmp/deb_hopsan/pbuilder
name=hopsan
devversion=2.22.1

# Pbuilder dists and archs
debianDistArchArray=( bookworm:amd64:bookworm
                      bullseye:amd64:bullseye
                      buster:amd64:qt5py3_buster )
ubuntuDistArchArray=( lunar:amd64:bookworm
                      jammy:amd64:bullseye
                      focal:amd64:focal )

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

dscFile="NoFile"
function builDSCFile {
  local -r confdir="$1"
  local -r packagedir="$2"
  local -r packagepreparedsrcfile="$3"
  local -r config="$4"
  local -r outputbasename="$5"

  echo ---------------------------------
  echo Build dsc package:
  echo ---------------------------------
  echo Source file: $packagepreparedsrcfile
  echo Packagedir : $packagedir
  echo Output name: $outputbasename
  echo Confdir    : $confdir
  echo Config     : $config
  echo ---------------------------------


  # First clear packaging dir if it already exist
  rm -rf ${packagedir}

  # Copy package configuration
  mkdir -p ${packagedir}
  cp -a ${confdir}/${config}/debian ${packagedir}

  # Copy "unpack" prepared source files to this dir
  tar -xzf ${packagepreparedsrcfile} -C ${packagedir}

  # Generate NEW change log file for this release version with no content in particular
  pushd ${packagedir} > /dev/null
  rm debian/changelog
  dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
  dch -p -m --release ""
  popd > /dev/null

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

readonly baseversion=$(echo ${baseversion_or_tag} | sed 's/[^0-9\.]//g')
branch_or_tag_to_clone="master"
if [[ ${baseversion_or_tag} =~ ^v ]]; then
  branch_or_tag_to_clone=${baseversion_or_tag}
fi

echo "Choose remote to clone from:"
select remote in file://${hopsancode_root} https://github.com/Hopsan/hopsan.git https://github.com/peterNordin/hopsan.git
do
  if [[ -z "$remote" ]]; then
    echo Invalid choice
    exit 1
  fi
  readonly hopsancode_url=${remote}
  break
done
if [[ ${hopsancode_url} == file://${hopsancode_root} ]]; then
  pushd ${hopsancode_root} > /dev/null
  branch_or_tag_to_clone=$(git branch --show-current)
  popd > /dev/null
fi

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
echo -------------------------------------------------
echo This is a DEVELOPMENT release: ${doDevRelease}
echo Release baseversion number: ${baseversion}
echo URL to clone: ${hopsancode_url}
echo Branch to clone: ${branch_or_tag_to_clone}
echo
printf "%s\n" "${distArchArrayDo[@]}"
echo -------------------------------------------------
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
# If a persistent cache dir for hopsan dependencies dowloads exists in the "workspace" then prefer it over loca cache that will be removed by git clean
if [[ -d ${hopsancode_root}/../hopsan-dependencies-cache ]]; then
    readonly dependencies_cache=${hopsancode_root}/../hopsan-dependencies-cache
else
    readonly dependencies_cache=${outputDir}/dependencies-cache
fi
readonly tmp_stage_directory=${outputDir}/hopsan-stage

mkdir -p ${outputDebDir}
mkdir -p ${pbuilderWorkDir}
mkdir -p ${dependencies_cache}
pushd ${outputDir} > /dev/null

# -----------------------------------------------------------------------------
# Clone source code to ensure a clean build
#
echo "Cloning from ${hopsancode_url} into ${tmp_stage_directory}"
if [[ -d ${tmp_stage_directory} ]]; then
    echo "Reusing: ${tmp_stage_directory} as in exists, resetting --hard and clean -ffdx"
    pushd ${tmp_stage_directory} > /dev/null
    git remote set-url origin ${hopsancode_url}
    git fetch --all --prune
    git reset --hard origin/${branch_or_tag_to_clone}
    git clean -ffdx
    popd > /dev/null
else
    #rm -rf ${tmp_stage_directory}
    git clone -b ${branch_or_tag_to_clone} --depth 1 ${hopsancode_url} ${tmp_stage_directory}
    if [[ $? -ne 0 ]]; then
        echo Error: Failed to clone from ${hopsancode_url}
        exit 1
    fi
fi
pushd ${tmp_stage_directory} > /dev/null
echo "Updaing git submodules"
git submodule sync
git submodule update --init --recommend-shallow
if [[ $? -ne 0 ]]; then
    echo Error: Failed to clone submodules from git
    exit 1
fi
popd > /dev/null

# -----------------------------------------------------------------------------
# Determine version number for output directory and file names
#
set -e
pushd ${tmp_stage_directory} > /dev/null
readonly releaserevision=$(./getGitInfo.sh date.time .)
readonly fullversionname=${baseversion}.${releaserevision}
echo Release revision number: $releaserevision
echo Release version name: $fullversionname
sleep 2
popd > /dev/null

readonly outputfile_basename=${name}_${fullversionname}
readonly packageorigsrcfile=${outputfile_basename}.orig.tar.gz
readonly package_dirname=${name}-${fullversionname}

stage_directory=${tmp_stage_directory}-${fullversionname}
rm -rf ${stage_directory} # Cleanup unlikely leftovers
cp -r ${tmp_stage_directory} ${stage_directory}

# -----------------------------------------------------------------------------
# Prepare source code inside stage directory and package it into original source package
#
readonly hopsancode_gitwc=${stage_directory}
${stage_directory}/packaging/prepareSourceCode.sh ${hopsancode_gitwc} ${stage_directory} \
                                                  ${baseversion} ${releaserevision} ${fullversionname} \
                                                  ${doDevRelease}
# Download dependencies, since that can not be done inside a pbuilder environment
# Unfortunately all dependencies must be downloaded since we can not know at this point which of them will be used
pushd ${stage_directory}/dependencies > /dev/null
./download-dependencies.py --all --cache ${dependencies_cache}
popd > /dev/null
set +e
# Remove .git directory and submodule files (if present) before packaging source code
find ${stage_directory} -name .git | xargs rm -rf
# Package source code
tar -czf ${packageorigsrcfile} -C ${stage_directory} .


# -----------------------------------------------------------------------------
# Now build DEB package
#
readonly debian_conf_root=${stage_directory}/packaging/deb
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

    builDSCFile ${debian_conf_root} $package_dirname $packageorigsrcfile $conf $outputfile_basename

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
    debconf_path=${debian_conf_root}/${conf}
    extraPackages=$(echo $(cat ${debconf_path}/debian/control | sed -e '/Build-Depends/,/Standards-Version/!d' -e 's/Build-Depends://' -e '/Standards-Version.*/d' -e 's/[(][^)]*[)]//g' -e 's/\,//g'))
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
