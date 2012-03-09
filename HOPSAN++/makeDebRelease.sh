#!/bin/bash
# $Id$

name=hopsan
version=0.5.3d

# TODO: should ask user for version input maybe

# -----------------------------------------------------------------------------
# First prepare source folders
#

# Clean bin folder
rm -f ./bin/*

# Build user documentation
./buildDocumentation.sh user

# Set version numbers (by changing .h files) BEFORE build
sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"$version\"|g" -i HopsanCore/include/version.h
sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"$version\"|g" -i HopsanGUI/version_gui.h

# Set splash screen version number
sed "s|X\.X\.X|$version|g" -i HopsanGUI/graphics/splash2.svg
inkscape ./HopsanGUI/graphics/splash2.svg --export-background=rgb\(255,255,255\) --export-png ./HopsanGUI/graphics/splash.png
# Revert changes in svg
svn revert ./HopsanGUI/graphics/splash2.svg

# Make sure development flag is not defined
sed "s|.*#define DEVELOPMENT|//#define DEVELOPMENT|" -i HopsanGUI/common.h
#------------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Now build DEB package
#
cd buildDebPackage

# Determine deb dir name
packagedir=$name-$version
packageorigsrcfile=$name\_$version.orig.tar.gz

# First clear dir if it already exist
rm -rf $packagedir
rm -rf $packageorigsrcfile

# Create the source code file
# Lets make a fake one for now
tar -czf $packageorigsrcfile SOURCE

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

# TODO: Should try to put output in some special folder

# TODO: MAYBE cleanup build dir, or ask user, or not
#------------------------------------------------------------------------------

