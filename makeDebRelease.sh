#!/bin/bash
# $Id$

# Shell script for building DEB packages of hopsan for multiple distributions using pbuilder
# Author: Peter Nordin peter.nordin@liu.se

set -u
#set -e

#--------------------------------------------------------------------------------------------------
# Config starts here
#--------------------------------------------------------------------------------------------------
buildRoot=buildDebPackage
name=hopsan
devversion=2.8.1

# Pbuilder dists and archs
debianDistArchArray=( stretch:amd64:qt5py27 stretch:i386:qt5py27 jessie:amd64:qt5py27 jessie:i386:qt5py27 )
debianDistArchSDArray=( stretch:amd64:qt5py3sd0 )
ubuntuDistArchArray=( bionic:amd64:qt5py3 artful:amd64:qt5py3 artful:i386:qt5py3 zesty:amd64:qt5py27 zesty:i386:qt5py27 xenial:amd64:qt5py27 xenial:i386:qt5py27 trusty:amd64:trusty trusty:i386:trusty )
ubuntuDistArchSDArray=( zesty:amd64:qt5py3sd0 )

# Pbuilder mirrors
ubuntuMirror="http://se.archive.ubuntu.com/ubuntu/"
debianMirror="http://ftp.se.debian.org/debian/"

#--------------------------------------------------------------------------------------------------
# Code starts here
#--------------------------------------------------------------------------------------------------
hopsanroot=$(pwd)
echo "Using Hopsan root: $hopsanroot"
echo "Enter sudo password for pbuilder:"
sudo -v

# Move directory src into directory dst. If a dir with the same name as src already exists it will be removed
mvrDir ()
{
  local -r src=${1%/}
  local -r dst=${2%/}
  local -r dstname="$dst/$src"
  if [ -d "$dstname" ]; then
    rm -r "$dstname"
  fi
  # Now move to unique directory
  echo "mv $1 to $dstname"
  mv "$1" "$dstname"
}

# Ask yes/no question, returning true or false in the global variable boolYNQuestionAnswer
boolAskYNQuestion()
{
  declare -g boolYNQuestionAnswer=false
  echo -n "$1" "Answer y/n [""$2""]: "
  read ans
  if [ -z "$ans" ]; then
    if [ "$2" = "y" ]; then
      boolYNQuestionAnswer="true"
    fi
  elif [ "$ans" = "y" ]; then
    boolYNQuestionAnswer=true
  fi
}

# Export a specific git directory, submodules are not included
git_export()
{
  set -e
  local -r src="$1"
  local -r dst="$2"
  local -r tarfile=hopsan_git_export.tar
  echo "Exporting from git: ${src} to ${dst}"
  mkdir -p "${dst}"
  pushd ${src}
  git archive --output ${tarfile} HEAD
  popd
  mv ${src}/${tarfile} ${dst}
  pushd ${dst}
  tar -xf ${tarfile}
  rm ${tarfile}
  popd
  set +e
}


dscFile="NoFile"
builDSCFile()
{
  local -r packagedir="$1"
  local -r packageorigsrcfile="$2"
  local -r config="$3"
  local -r usepythonqt="$4"
  local -r outputbasename="$5"

  echo $packagedir
  echo $packageorigsrcfile
  echo $config
  echo $doUsePythonQt
  echo $outputbasename

  # First clear dir if it already exist
  rm -rf ${packagedir}

  # Export package configuration
  git_export debconfig_base $packagedir
  git_export debconfig_${config} $packagedir

  # Check if we should remove PythonQt if it should not be used
  if [ "$usepythonqt" = false ]; then
    sed "/setupPythonQt.sh/d" -i "$packagedir/debian/rules"
  fi

  # Copy "unpack" prepared source files to this dir
  tar -xzf $packageorigsrcfile -C $packagedir

  # Generate NEW change log file for this release version with no content in particular
  pushd $packagedir
  rm debian/changelog
  dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
  dch -p -m --release ""
  popd

  # Generate dsc source file
  dpkg-source --before-build ${packagedir}
  dpkg-source -b ${packagedir}
  dscFile=`ls $outputbasename*.dsc`
}

# Ask user for version input
echo
echo -n "Enter release version number on the form a.b.c or leave blank for DEV build release: "
read baseversion
doDevRelease="false"
if [ -z "$baseversion" ]; then
  doDevRelease="true"
  baseversion=$devversion
fi
readonly releaserevision=$(./getGitInfo.sh date.time .)
readonly fullversionname=${baseversion}.${releaserevision}

echo
boolAskYNQuestion "Do you want the defaultComponentLibrary to be build in?" "n"
readonly doBuildInComponents="$boolYNQuestionAnswer"

echo
boolAskYNQuestion "Do you want to build with PythonQt and python support?" "y"
readonly doUsePythonQt="$boolYNQuestionAnswer"

echo
distArchArrayDo=()
for i in "${debianDistArchArray[@]}"; do
  boolAskYNQuestion "Do you want to build, "$i"?" "n"
  distArchArrayDo+=("$i"":""D"":""$boolYNQuestionAnswer")
done
for i in "${ubuntuDistArchArray[@]}"; do
  boolAskYNQuestion "Do you want to build, "$i"?" "n"
  distArchArrayDo+=("$i"":""U"":""$boolYNQuestionAnswer")
done
for i in "${debianDistArchSDArray[@]}"; do
  boolAskYNQuestion "Do you want to build (system deps), "$i"?" "n"
  distArchArrayDo+=("$i"":""D"":""$boolYNQuestionAnswer")
done
for i in "${ubuntuDistArchSDArray[@]}"; do
  boolAskYNQuestion "Do you want to build (system deps), "$i"?" "n"
  distArchArrayDo+=("$i"":""U"":""$boolYNQuestionAnswer")
done


echo
echo ---------------------------------------
echo "This is a DEV release: $doDevRelease"
echo "Release baseversion number: $baseversion"
echo "Release revision number: $releaserevision"
echo "Release version name: $fullversionname"
echo "Built in components: $doBuildInComponents"
echo "Using PythonQt: $doUsePythonQt"
printf "%s\n" "${distArchArrayDo[@]}"
echo ---------------------------------------
boolAskYNQuestion "Is this OK?" "n"
if [ "$boolYNQuestionAnswer" = false ]; then
  echo Aborting!
  exit 1
fi

echo

# -----------------------------------------------------------------------------
# Go to buildRoot before build
# Set output directories and absolute Pbuildpath
#
cd ${buildRoot}
outputDir=output
pbuilderWorkDir='/var/tmp/deb_hopsan/pbuilder'
outputDebDir="${outputDir}/debs"
mkdir -p $outputDebDir
mkdir -p $pbuilderWorkDir

# -----------------------------------------------------------------------------
# Determine deb dir name
#
outputbasename=${name}_${fullversionname}
packageorigsrcfile=${outputbasename}.orig.tar.gz
packagedir=${name}-${fullversionname}
packagesrcfile=${packagedir}.tar.gz

# -----------------------------------------------------------------------------
# Prepare source code
#
set -e
srcExportDir=${outputDir}/hopsanSrcExport_${fullversionname}
./prepareSourceCode.sh $(pwd)/../  ${srcExportDir} ${baseversion} ${releaserevision} ${fullversionname} ${doDevRelease} ${doBuildInComponents}

pushd $srcExportDir
tar -czf $packageorigsrcfile *
popd
mv ${srcExportDir}/${packageorigsrcfile} .
set +e

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

    builDSCFile $packagedir $packageorigsrcfile $conf $doUsePythonQt $outputbasename

    doCreateUpdatePbuilderBaseTGZ=true
    basetgzFile=${pbuilderWorkDir}/${dist}${arch}.tgz
    pbuilderBuildDir=${pbuilderWorkDir}/build
    mkdir -p ${pbuilderBuildDir}
    resultPath=${pbuilderWorkDir}/result/${dist}
    mkdir -p ${resultPath}
    logFile=${resultPath}/${outputbasename}.log

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
    extraPackages=$(grep Build-Depends debconfig_${conf}/debian/control | sed -e 's/Build-Depends://' -e 's/[(][^)]*[)]//g' -e 's/\,//g')
    #echo $extraPackages

    # Remove file if it is corrupt
    if [ -f ${basetgzFile} ]; then
      if ! gunzip -t $basetgzFile ; then
        rm -f "${basetgzFile}"
      fi
    fi

    # Check if its time to update
    if [ -f ${basetgzFile} ]; then
      age=$(( $(date +%s) - $(stat -c %Y ${basetgzFile}) ))
      #echo "age: $age"
      if [ ${age} -le $((3600*6)) ]; then
        doCreateUpdatePbuilderBaseTGZ="false"
      fi
    fi

    # Update or create pbuild environments
    debootstrapOk=true
    if [ "$doCreateUpdatePbuilderBaseTGZ" = true ]; then    
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
      outputDebName=`ls $resultPath/$outputbasename*_$arch.deb`

      # Check if it exist (success)
      if [ -f "$outputDebName" ]; then
        buildStatusArray+=("$dist""_""$arch"":BuildOk")

        # Now move and rename output deb file to dist output dir
        resultingDebFile=${outputDebDir}/${outputbasename}_${dist}_${arch}.deb
        mv $outputDebName ${resultingDebFile}

        # Check package with lintian ('|| true' avoids script abort on errors)
        lintian --no-tag-display-limit --color always -X files ${resultingDebFile} || true
      else
        buildStatusArray+=("$dist""_""$arch"":BuildFailed")
      fi
    fi
  fi
done
  
# Move the package directory
mvrDir ${packagedir} ${outputDir}
# Move the package related files
mv ${outputbasename}* ${outputDir}

echo
echo "Build Status:"
echo "-------------"

printf "%s\n" "${buildStatusArray[@]}"

echo
echo Done!
