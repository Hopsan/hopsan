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
devversion=0.6.

# Pbuilder dists and archs
distArchArray=( raring:amd64 raring:i386 quantal:amd64 quantal:i386 precise:amd64 precise:i386 )

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

# Ask user for version input
echo
echo -n "Enter release version number on the form a.b.c or leave blank for DEV build release:"
read version
doDevRelease="false"
if [ -z "$version" ]; then
  doDevRelease="true"
  svnrev=`./getSvnRevision.sh`
  version=$devversion"r"$svnrev
fi

echo
boolAskYNQuestion "Do you want the defaultComponentLibrary to be build in?" "n"
doBuildInComponents="$boolYNQuestionAnswer"

echo
distArchArrayDo=()
boolAskYNQuestion "Do you want to build for multiple supported dists, using pbuilder?" "y"
doPbuild="$boolYNQuestionAnswer"
if [ "$doPbuild" = "true" ]; then
  for i in "${distArchArray[@]}"; do
    boolAskYNQuestion "Do you want to build, "$i"?" "y"
    distArchArrayDo+=("$i"":""$boolYNQuestionAnswer")
  done
fi

echo
echo ---------------------------------------
echo "This is a DEV release: $doDevRelease"
echo "Release version number: $version"
echo "Built in components: $doBuildInComponents"
echo "Using pbuilder: $doPbuild"
if [ "$doPbuild" = "true" ]; then
  echo ${distArchArrayDo[@]}
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
packagedir=$name-$version
outputbasename=$name\_$version
packageorigsrcfile=$outputbasename.orig.tar.gz
packagesrcfile=$name-$version.tar.gz

# -----------------------------------------------------------------------------
# Prepare source code
#
srcExportDir=$outputDir/hopsanSrcExport\_$version
./prepareSourceCode.sh ../  $srcExportDir $version $doDevRelease $doBuildInComponents

cd $srcExportDir
tar -czf $packageorigsrcfile *
cd $OLDPWD
mv $srcExportDir/$packageorigsrcfile .

# -----------------------------------------------------------------------------
# Now build DEB package
#
buildStatusArray=()

# First clear dir if it already exist
rm -rf $packagedir

# Export template
svn export hopsan-template $packagedir
# Copy "unpack" prepared source  files to this dir
tar -xzf $packageorigsrcfile -C $packagedir

# Generate NEW changelog file for this release version with no content in particular
cd $packagedir
rm debian/changelog
dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
dch -p -m --release ""
cd ..

if [ "$doPbuild" = "true" ]; then
  # Generate dsc source file
  dpkg-source --before-build $packagedir
  dpkg-source -b $packagedir
  dscFile=`ls $outputbasename*.dsc`

  for i in "${distArchArrayDo[@]}"; do
    # Split input into array to extract data
    IFS=':' read -ra arr <<< "$i"
    dist="${arr[0]}"
    arch="${arr[1]}"
    doBuild="${arr[2]}"

    if [ "$doBuild" = "true" ]; then
      echo
      echo "==========================="
      echo "Building for $dist $arch"
      echo "==========================="
      sleep 1
      doCreateUpdatePbuilderBaseTGZ="true"
      basetgzFile="$pbuilderWorkDir$dist$arch.tgz"
      pbuilderBuildDir="$pbuilderWorkDir""build/"
      mkdir -p $pbuilderBuildDir
      resultPath="$pbuilderWorkDir""result/$dist"
      mkdir -p "$resultPath"
      logFile="$resultPath/$outputbasename.log"

      # Update or create pbuild environments
      extraPackages="debhelper unzip subversion lsb-release libtbb-dev libqt4-dev libqtwebkit-dev libqt4-opengl-dev python-dev fakeroot"
      debootstrapOk="true"
      if [ "$doCreateUpdatePbuilderBaseTGZ" = "true" ]; then
	    if [ -f $basetgzFile ]; then
	      echo
	      echo "Updating existing TGZ: $basetgzFile"
	      echo "------------------------"
	      sudo pbuilder --update --extrapackages "$extraPackages"  --aptcache "" --buildplace "$pbuilderBuildDir" --basetgz $basetgzFile
	    else
	      echo
	      echo "Creating new TGZ: $basetgzFile"
	      echo "------------------------"
	      sudo pbuilder --create --components "main universe" --extrapackages "$extraPackages" --aptcache "" --buildplace "$pbuilderBuildDir" --distribution $dist --architecture $arch --basetgz $basetgzFile
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
	  sudo pbuilder --build --basetgz $basetgzFile --logfile "$logFile" --aptcache "" --buildplace "$pbuilderBuildDir" --buildresult $resultPath $dscFile
	  
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
  
  echo ${buildStatusArray[@]}

else
  # Now lets create and test the package
  cd $packagedir
  debuild -us -uc --lintian-opts --color always -X files
  cd ..

  # Move new files to output dir
  mkdir -p $outputDir/thismachine
  mvrDir $packagedir $outputDir/thismachine
  mv $outputbasename* $outputDir/thismachine
fi

echo
echo Done!
