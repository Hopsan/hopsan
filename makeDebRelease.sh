#!/bin/bash
# $Id$

# Shell script for building DEB packages of hopsan for multiple distributions using pbuilder
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-04


#--------------------------------------------------------------------------------------------------
# Config starts here
#--------------------------------------------------------------------------------------------------
buildRoot="buildDebPackage/"
name=hopsan
devversion=0.7.x

# Pbuilder dists and archs
debianDistArchArray=( stretch:amd64:qt5 stretch:i386:qt5 jessie:amd64:qt5 jessie:i386:qt5 wheezy:amd64:qt4 wheezy:i386:qt4 )
ubuntuDistArchArray=( wily:amd64:qt5 wily:i386:qt5 vivid:amd64:qt5 vivid:i386:qt5 trusty:amd64:qt4 trusty:i386:qt4 )

# Pbuilder mirrors
ubuntuMirror="http://se.archive.ubuntu.com/ubuntu/"
debianMirror="http://ftp.se.debian.org/debian/"

#--------------------------------------------------------------------------------------------------
# Code starts here
#--------------------------------------------------------------------------------------------------

# Move directory overwriting dst function, dst dir will be removed if name is same
mvrDir ()
{
  src=${1%/}
  dst=${2%/}
  dstname="$dst/$src"
  if [ -d "$dstname" ]; then
    rm -r $dstname
  fi
  #Now move to unique directory
  echo "mv $1 to $dstname"
  mv $1 $dstname
}

# Ask yes/no question, returning true or false in the global variable boolYNQuestionAnswer
boolYNQuestionAnswer="false"
boolAskYNQuestion()
{
  boolYNQuestionAnswer="false"
  echo -n "$1" "Answer y/n [""$2""]: "
  read ans
  if [ -z "$ans" ]; then
    if [ "$2" = "y" ]; then
      boolYNQuestionAnswer="true"
    fi
  elif [ "$ans" = "y" ]; then
    boolYNQuestionAnswer="true"
  fi
}

dscFile="NoFile"
builDSCFile()
{
  packagedir="$1"
  packageorigsrcfile="$2"
  qtver="$3"
  doUsePythonQt="$4"
  outputbasename="$5"

  echo $packagedir
  echo $packageorigsrcfile
  echo $qtver
  echo $doUsePythonQt
  echo $outputbasename

  # First clear dir if it already exist
  rm -rf $packagedir

  # Export template
  svn export hopsan-template $packagedir
  if [ "$qtver" = "qt5" ]; then
      mv $packagedir/debian/control_qt5 $packagedir/debian/control
      mv $packagedir/debian/rules_qt5 $packagedir/debian/rules
  else
      rm $packagedir/debian/control_qt5
      rm $packagedir/debian/rules_qt5
  fi

  # Check if we should remove PythonQt if it should not be used
  if [ "$doUsePythonQt" = "false" ]; then
    sed "/setupPythonQt.sh/d" -i "$packagedir/debian/rules"
  fi
  # Copy "unpack" prepared source files to this dir
  tar -xzf $packageorigsrcfile -C $packagedir

  # Generate NEW change log file for this release version with no content in particular
  cd $packagedir
  rm debian/changelog
  dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
  dch -p -m --release ""
  cd ..

  # Generate dsc source file
  dpkg-source --before-build $packagedir
  dpkg-source -b $packagedir
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
releaserevision=`./getSvnRevision.sh`
fullversionname=$baseversion"_r"$releaserevision

echo
boolAskYNQuestion "Do you want the defaultComponentLibrary to be build in?" "n"
doBuildInComponents="$boolYNQuestionAnswer"

echo
boolAskYNQuestion "Do you want to build with PythonQt and python support?" "n"
doUsePythonQt="$boolYNQuestionAnswer"

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


echo
echo ---------------------------------------
echo "This is a DEV release: $doDevRelease"
echo "Release baseversion number: $baseversion"
echo "Release revision number: $releaserevision"
echo "Release version name: $fullversionname"
echo "Built in components: $doBuildInComponents"
echo "Using PythonQt: $doUsePythonQt"
echo "Using pbuilder: $doPbuild"
if [ "$doPbuild" = "true" ]; then
  printf "%s\n" "${distArchArrayDo[@]}"
fi
echo ---------------------------------------
boolAskYNQuestion "Is this OK?" "n"
if [ "$boolYNQuestionAnswer" = "false" ]; then
  echo Aborting!
  exit 1
fi

echo

# -----------------------------------------------------------------------------
# Go to buildRoot before build
# Set output directories and absolute Pbuildpath
#
cd $buildRoot
outputDir=output
pbuilderWorkDir="$PWD/$outputDir/pbuilder/"
outputDebDir="$outputDir/debs"
mkdir -p $outputDebDir
mkdir -p $pbuilderWorkDir

# -----------------------------------------------------------------------------
# Determine deb dir name
#

outputbasename=$name\_$baseversion.r$releaserevision
packageorigsrcfile=$outputbasename.orig.tar.gz
packagedir=$name-$baseversion.r$releaserevision
packagesrcfile=$name-$baseversion.r$releaserevision.tar.gz

# -----------------------------------------------------------------------------
# Prepare source code
#
srcExportDir=$outputDir/hopsanSrcExport\_$fullversionname
./prepareSourceCode.sh `pwd`/../  $srcExportDir $baseversion $releaserevision $fullversionname $doDevRelease $doBuildInComponents

cd $srcExportDir
tar -czf $packageorigsrcfile *
cd $OLDPWD
mv $srcExportDir/$packageorigsrcfile .

# -----------------------------------------------------------------------------
# Now build DEB package
#
buildStatusArray=()

# # First clear dir if it already exist
# rm -rf $packagedir
#
# # Export template
# svn export hopsan-template $packagedir
# # Check if we should remove PythonQt if it should not be used
# if [ "$doUsePythonQt" = "false" ]; then
#   sed "/unpackPatchAndBuildPythonQt.sh/d" -i "$packagedir/debian/rules"
# fi
# # Copy "unpack" prepared source files to this dir
# tar -xzf $packageorigsrcfile -C $packagedir
#
# # Generate NEW changelog file for this release version with no content in particular
# cd $packagedir
# rm debian/changelog
# dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
# dch -p -m --release ""
# cd ..


#   # Generate dsc source file
#   dpkg-source --before-build $packagedir
#   dpkg-source -b $packagedir
#   dscFile=`ls $outputbasename*.dsc`

for i in "${distArchArrayDo[@]}"; do
  # Split input into array to extract data
  IFS=':' read -ra arr <<< "$i"
  dist="${arr[0]}"
  arch="${arr[1]}"
  qtver="${arr[2]}"
  distBase="${arr[3]}"
  doBuild="${arr[4]}"

  if [ "$doBuild" = "true" ]; then
    echo
    echo "==========================="
    echo "Building for $dist $arch"
    echo "==========================="
    sleep 1

    builDSCFile $packagedir $packageorigsrcfile $qtver $doUsePythonQt $outputbasename

    doCreateUpdatePbuilderBaseTGZ="true"
    basetgzFile="$pbuilderWorkDir$dist$arch.tgz"
    pbuilderBuildDir="$pbuilderWorkDir""build/"
    mkdir -p $pbuilderBuildDir
    resultPath="$pbuilderWorkDir""result/$dist"
    mkdir -p "$resultPath"
    logFile="$resultPath/$outputbasename.log"

    # Set pbuild options specific to ubuntu or debian
    if [ "$distBase" = "U" ]; then
      #todo add mirror and keyring
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

    # Set packages that need to be installed, this installs them once and for all, and avoids wasting time on every build
    if [ "$qtver" = "qt5" ]; then
      extraPackages="debhelper unzip subversion lsb-release libtbb-dev qtbase5-dev libqt5webkit5-dev libqt5svg5-dev qtmultimedia5-dev libqt5opengl5-dev python-dev libhdf5-dev  fakeroot cmake libtool-bin qt5-default automake pkg-config"
    else
      extraPackages="debhelper unzip subversion lsb-release libtbb-dev libqt4-dev libqtwebkit-dev libqt4-opengl-dev python-dev libhdf5-dev fakeroot cmake automake libtool pkg-config"
    fi

    # Update or create pbuild environments
    debootstrapOk="true"
    if [ "$doCreateUpdatePbuilderBaseTGZ" = "true" ]; then
      if [ -f $basetgzFile ]; then
        echo
        echo "Updating existing TGZ: $basetgzFile"
        echo "------------------------"
        sudo pbuilder --update $distBaseOpts  --basetgz "$basetgzFile" --extrapackages "$extraPackages" --aptcache "" --buildplace "$pbuilderBuildDir"
      else
        echo
        echo "Creating new TGZ: $basetgzFile"
        echo "------------------------"
        sudo pbuilder --create --distribution $dist --architecture $arch --mirror $optsMirror $optsDebootstrap --components "$optsComponents" --basetgz "$basetgzFile" --extrapackages "$extraPackages" --aptcache "" --buildplace "$pbuilderBuildDir"
      fi
      # Check for success
      if [ $? -ne 0 ]; then
      debootstrapOk="false"
      buildStatusArray+=("$dist""_""$arch"":DebootstrapFailed")
      echo "pubulider create or update FAILED! for $dist $arch, aborting!"
      fi
    fi

    if [ "$debootstrapOk" = "true" ]; then
      # Now build source package
      echo
      echo "Building with pbuilder"
      echo "------------------------"
      sudo pbuilder --build --basetgz "$basetgzFile" --aptcache "" --buildplace "$pbuilderBuildDir" --logfile "$logFile" --buildresult $resultPath $dscFile

      # Figure out the actual output file name
      outputDebName=`ls $resultPath/$outputbasename*_$arch.deb`

      # Check if it exist (success)
      if [ -f "$outputDebName" ]; then
        buildStatusArray+=("$dist""_""$arch"":BuildOk")

        # Now move and rename output deb file to dist output dir
        mv $outputDebName $outputDebDir/$outputbasename\_$dist\_$arch.deb

        # Check package with lintian
        lintian --color always -X files $outputDebDir/$outputbasename\_$dist\_$arch.deb
      else
        buildStatusArray+=("$dist""_""$arch"":BuildFailed")
      fi
    fi
  fi
done
  
# Move the package directory
mvrDir $packagedir $outputDir
# Move the package related files
mv $outputbasename* $outputDir

echo
echo "Build Status:"
echo "-------------"

printf "%s\n" "${buildStatusArray[@]}"

echo
echo Done!
