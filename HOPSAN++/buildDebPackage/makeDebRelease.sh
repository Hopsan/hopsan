#!/bin/bash
# $Id$

version=0.5.3

svn export hopsan-0.0.0 hopsan-$version
cd hopsan-$version
debuild -us -uc --lintian-opts --color always -X files
cd ..
