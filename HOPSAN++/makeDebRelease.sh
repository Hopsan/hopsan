#!/bin/bash
# $Id$

name=hopsan
devversion=0.6.

# Pbuildnames
pbuildpath_oneiric386="/var/cache/pbuilder/oneiric386.tgz"
pbuildpath_oneiric64="/var/cache/pbuilder/oneiric64.tgz"

# Ask user for version input 
echo -n "Enter release version number on the form a.b.c or leave blank for DEV build release:"
read version

devrelease="false"
if [ -z "$version" ]; then
  devrelease="true"
  svnrev=`./getSvnRevision.sh`
  version=$devversion"r"$svnrev
fi

echo
echo "Do you want to build for all supported dists, using pbuilder (This WILL taka a LOONG time) NOT YET WORKING"
echo "Answer: y/n"
read ans

dopbuild="false"
if [ "$ans" = "y" ]; then
    dopbuild="true"
fi

echo
echo ---------------------------------------
echo This is a DEV release: $devrelease
echo Release version number: $version
echo Using pbuilder: $dopbuild
echo ---------------------------------------
echo Is this OK, y/n
read ans
if [ "$ans" != "y" ]; then
  echo Aborting!
  exit 1
fi

echo


# -----------------------------------------------------------------------------
# First prepare source folders
#

# Clean bin folder
rm -rf ./bin/*

# Build user documentation
./buildDocumentation.sh user

if [ $devrelease = "false" ]; then
  # Set version numbers (by changing .h files) BEFORE build
  sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"$version\"|g" -i HopsanCore/include/version.h
  sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"$version\"|g" -i HopsanGUI/version_gui.h

  # Set splash screen version number
  sed "s|X\.X\.X|$version|g" -i HopsanGUI/graphics/splash2.svg
  inkscape ./HopsanGUI/graphics/splash2.svg --export-background=rgb\(255,255,255\) --export-png ./HopsanGUI/graphics/splash.png
  # Revert changes in svg
  svn revert ./HopsanGUI/graphics/splash2.svg
  
  # Make sure development flag is not defined
  sed "s|.*DEFINES *= DEVELOPMENT|#DEFINES *= DEVELOPMENT|" -i HopsanGUI/HopsanGUI.pro
fi
#------------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Now build DEB package
#
cd buildDebPackage

# Determine deb dir name
packagedir=$name-$version
outputbasename=$name\_$version
packageorigsrcfile=$outputbasename.orig.tar.gz

# First clear dir if it already exist
rm -rf $packagedir
rm -rf $packageorigsrcfile

# Create the source code file
# Lets make a fake one for now
tar -czf $packageorigsrcfile HOPSANSOURCE

# Export template
svn export hopsan-template $packagedir
cd $packagedir

# Generate NEW changelog file for this release version with no content in particular
rm debian/changelog
#dch -p -M --create --package $name --newversion=$version See Hopsan-release-notes.txt for changes
dch -p -M -d --create "See Hopsan-release-notes.txt for changes"
dch -p -m --release ""

if [ "$dopbuild" = "true" ]; then
    
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
  # Now lets create and test the package
  debuild -us -uc --lintian-opts --color always -X files
  cd ..

  # Move new files to output dir
  rm -rf output
  mkdir -p output
  mv $packagedir* output
  mv $outputbasename* output
fi

# TODO: MAYBE cleanup build dir, or ask user, or not
#------------------------------------------------------------------------------

