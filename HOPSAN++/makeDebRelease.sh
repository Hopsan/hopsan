#!/bin/bash
# $Id$

outputDir=output
name=hopsan
devversion=0.6.

# Pbuildpath
pbuilderBaseTGZpath="/var/cache/pbuilder/"

# Ask user for version input 
echo -n "Enter release version number on the form a.b.c or leave blank for DEV build release:"
read version

doDevRelease="false"
if [ -z "$version" ]; then
  doDevRelease="true"
  svnrev=`./getSvnRevision.sh`
  version=$devversion"r"$svnrev
fi

echo
echo "Do you want to build for all supported dists, using pbuilder (This WILL taka a LOONG time) NOT YET WORKING"
echo "Answer: y/n"
read ans

doPbuild="false"
if [ "$ans" = "y" ]; then
    doPbuild="true"
fi

echo
echo ---------------------------------------
echo This is a DEV release: $doDevRelease
echo Release version number: $version
echo Using pbuilder: $doPbuild
echo ---------------------------------------
echo Is this OK, y/n
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
rm -rf $outputDir
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
./prepareSourceCode.sh ../  $srcExportDir $version $doDevRelease

cd $srcExportDir
tar -czf $packageorigsrcfile *
cd $OLDPWD
mv $srcExportDir/$packageorigsrcfile .
#cp -a $packageorigsrcfile $packagesrcfile

# -----------------------------------------------------------------------------
# Now build DEB package
#

# First clear dir if it already exist
rm -rf $packagedir
####rm -rf $packageorigsrcfile

# Create the source code file
# Lets make a fake one for now
###tar -czf $packageorigsrcfile HOPSANSOURCE

# Export template
svn export hopsan-template $packagedir
# Copy "unpack" prepared source  files to this dir
tar -xzf $packageorigsrcfile -C $packagedir

cd $packagedir


# Generate NEW changelog file for this release version with no content in particular
rm debian/changelog
#dch -p -M --create --package $name --newversion=$version See Hopsan-release-notes.txt for changes
dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
dch -p -m --release ""

if [ "$doPbuild" = "true" ]; then
  
  # Generate dsc source file
  cd ..
  dpkg-source --before-build $packagedir
  dpkg-source -b $packagedir
  dscFile=`ls $outputbasename*.dsc`
  
  doCreateUpdatePbuilderBaseTGZ="true"
  dist=oneiric
  arch=amd64
  basetgzFile=$pbuilderBaseTGZpath$dist$arch.tgz
  resultPath="$pbuilderBaseTGZpath/result/$dist"
  
  # Update or create pbuild environments
  if [ "$doCreateUpdatePbuilderBaseTGZ" = "true" ]; then
    if [ -f $basetgzFile ]; then
      sudo pbuilder --update --basetgz $basetgzFile
    else
	  sudo pbuilder --create --components "main universe" --extrapackages "debhelper unzip libtbb-dev libqt4-dev" --distribution $dist --architecture $arch --basetgz $basetgzFile
    fi
  fi
  
  # Now build source package
  sudo pbuilder --build --basetgz $basetgzFile --buildresult $resultPath $dscFile
  
  # Now copy/move files to correct output dir
  mkdir -p $outputDir/$dist
  cp $resultPath/* $outputDir/$dist
  mv $packagedir* $outputDir/$dist
  mv $outputbasename* $outputDir
  
  # Add distname to filename
  cd $outputDir/$dist
  debName=`ls $outputbasename*_$arch.deb`
  mv $debName $outputbasename\_$dist\_$arch.deb
  cd $OLDPWD

else
  # Remove the dependency build from rules, we use our pre build ones

  # Now lets create and test the package
  debuild -us -uc --lintian-opts --color always -X files
  cd ..

  # Move new files to output dir
  mkdir -p $outputDir/thismachine
  mv $packagedir* $outputDir/thismachine
  mv $outputbasename* $outputDir/thismachine
fi
