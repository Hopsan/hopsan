#!/bin/bash
# $Id$

name=hopsan
version=0.5.3

# TODO: should ask user for version input maybe

# TODO: shoudl run sed on version files, splash screen and such


# Determine deb dir name
packagedir=$name-$version
# First clear dir if it already exist
rm -r $packagedir
# Export template
svn export hopsan-0.0.0 $packagedir
cd $packagedir
# Generate NEW changelog file for this release version with no content in particular
rm debian/changelog
dch -p -M --create --package $name --newversion=$version See Hopsan-release-notes.txt for changes
dch -p -m --release ""
# Now lets create and test the package
debuild -us -uc --lintian-opts --color always -X files
cd ..

# TODO: Should try to put output in some special folder

# TODO: MAYBE cleanup build dir, or ask user, or not