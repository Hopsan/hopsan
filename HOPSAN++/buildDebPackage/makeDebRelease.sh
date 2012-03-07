#!/bin/bash
# $Id$

version=0.5.3

svn export hopsan-0.0.0 hopsan-$version
cd hopsan-$version
rm debian/changelog
dch -p -M --create --package hopsan --newversion=$version See Hopsan-release-notes.txt for changes
dch -p -m --release ""
debuild -us -uc --lintian-opts --color always -X files
cd ..
