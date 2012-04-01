#!/bin/bash
# $Id$

outputDir=output
name=hopsan
devversion=0.6.

# Pbuildnames
pbuildpath_oneiric386="/var/cache/pbuilder/oneiric386.tgz"
pbuildpath_oneiric64="/var/cache/pbuilder/oneiric64.tgz"

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
    
  # Update or create pbuild environments
  # TODO should not have manual if for each dist
  if [ -f $pbuildpath_oneiric64 ]; then
      sudo pbuilder --update --basetgz $pbuildpath_oneiric64
  else
      sudo pbuilder --create --distribution oneiric --architecture amd64 --basetgz $pbuildpath_oneiric64
  fi
  
  if [ -f $pbuildpath_oneiric386 ]; then
      sudo pbuilder --update --basetgz $pbuildpath_oneiric386
  else
      sudo pbuilder --create --distribution oneiric --architecture i386 --basetgz $pbuildpath_oneiric386
  fi

else
  # Remove the dependency build from rules, we use our pre build ones

  # Now lets create and test the package
  debuild -us -uc --lintian-opts --color always -X files
  cd ..

  # Move new files to output dir
  mv $packagedir* $outputDir
  mv $outputbasename* $outputDir
fi
