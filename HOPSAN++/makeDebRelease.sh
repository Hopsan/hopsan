#!/bin/bash
# $Id$

name=hopsan
devversion=0.6.x
devrelease="false"

# Ask user for version input 
echo -n "Enter release version number on the form a.b.c or leave blank for DEV build release:"
read version

if [ -z "$version" ]; then
  devrelease="true"
  version=$devversion
  echo
  echo Building DEV release
else
  echo
  echo Version will be: $version is this OK, y/n
  read ans
  if [ "$ans" != "y" ]; then
    echo Aborting!
    exit 1
  fi
fi

# -----------------------------------------------------------------------------
# First prepare source folders
#

# Clean bin folder
rm -f ./bin/*

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
dch -p -M -d --create See Hopsan-release-notes.txt for changes
dch -p -m --release ""

# Now lets create and test the package
debuild -us -uc --lintian-opts --color always -X files
cd ..

# Move new files to output dir
rm -rf output
mkdir -p output
mv $packagedir* output
mv $outputbasename* output

# TODO: MAYBE cleanup build dir, or ask user, or not
#------------------------------------------------------------------------------

