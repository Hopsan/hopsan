#!/bin/bash
# $Id$

# Shell script for building DEB packages of hopsan for multiple distributions using pbuilder
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-04

outputDir=output
name=hopsan
devversion=0.6.

# Pbuildpath
pbuilderBaseTGZpath="/var/cache/pbuilder/"

# Pbuilder dists and archs
distArchArray=( quantal:amd64 quantal:i386 precise:amd64 precise:i386 oneiric:amd64 oneiric:i386 )
distArchArrayDo=()

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
echo -n "Do you want the defaultComponentLibrary to be build in? Answer: y/n : "
read ans

doBuildInComponents="false"
if [ "$ans" = "y" ]; then
  doBuildInComponents="true"
fi

echo
echo -n "Do you want to build for multiple supported dists, using pbuilder? Answer: y/n : "
read ans

doPbuild="false"
if [ "$ans" = "y" ]; then
  doPbuild="true"
  for i in "${distArchArray[@]}"; do
    echo -n "Do you want to build, "$i", y/n: "
    read ans

    if [ "$ans" = "y" ]; then
      distArchArrayDo=( "${distArchArrayDo[@]}" "$i":true )
    else
      distArchArrayDo=( "${distArchArrayDo[@]}" "$i":false )
    fi
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
echo -n "Is this OK? y/n: "
read ans
if [ "$ans" != "y" ]; then
  echo Aborting!
  exit 1
fi

echo

# -----------------------------------------------------------------------------
# Go to dir and clear old files
#
cd buildDebPackage
#rm -rf $outputDir
mkdir -p $outputDir

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
      echo "Building for $dist $arch"
      sleep 1
      doCreateUpdatePbuilderBaseTGZ="true"
      basetgzFile=$pbuilderBaseTGZpath$dist$arch.tgz
      resultPath="$pbuilderBaseTGZpath/result/$dist"

      # Update or create pbuild environments
      extraPackages="debhelper unzip subversion lsb-release libtbb-dev libqt4-dev libqtwebkit-dev libqt4-opengl-dev"
      debootstrapOk="true"
      if [ "$doCreateUpdatePbuilderBaseTGZ" = "true" ]; then
	    if [ -f $basetgzFile ]; then
	      sudo pbuilder --update --extrapackages "$extraPackages" --basetgz $basetgzFile
	    else
	      sudo pbuilder --create --components "main universe" --extrapackages "$extraPackages" --distribution $dist --architecture $arch --basetgz $basetgzFile
	    fi
	    # Check for sucess
	    if [ $? -ne 0 ]; then
		debootstrapOk="false"
		echo "pubulider create or update FAILED! for $dist $arch, aborting!"
		read -p "press any key to continue"
	    fi
      fi
      
      if [ "$debootstrapOk" = "true" ]; then
          # Now build source package
	  sudo pbuilder --build --removepackages "ccache" --basetgz $basetgzFile --buildresult $resultPath $dscFile
	  outputDebName=`ls $resultPath/$outputbasename*_$arch.deb`
	  
          # Now copy and rename output deb file to dist output dir
	  mkdir -p $outputDir/$dist
	  cp $outputDebName $outputDir/$dist/$outputbasename\_$dist\_$arch.deb
	  
          # Check package with lintian
	  lintian --color always -X files $outputDir/$dist/$outputbasename\_$dist\_$arch.deb
      fi
    fi
  done
  
  mv $packagedir* $outputDir/
  mv $outputbasename* $outputDir/

else
  # Now lets create and test the package
  cd $packagedir
  debuild -us -uc --lintian-opts --color always -X files
  cd ..

  # Move new files to output dir
  mkdir -p $outputDir/thismachine
  mv $packagedir* $outputDir/thismachine
  mv $outputbasename* $outputDir/thismachine
fi

echo Done!
