#!/bin/bash

function addit {
giturl=$1
dir=$2
co=$3

echo $giturl $dir $co
rm -rf $dir
git submodule add $giturl $dir
cd $dir
git checkout $co
cd $OLDPWD

}

function additc {
    addit $1 ${2}_code $3
}

#git reset --hard HEAD
rm .gitmodules

addit https://github.com/msgpack/msgpack-c.git Dependencies/msgpack-c cpp-1.3.0
addit https://github.com/zeromq/cppzmq.git Dependencies/cppzmq HEAD
additc https://github.com/zeromq/zeromq4-1.git Dependencies/zeromq v4.1.6
additc https://github.com/Orc/discount.git Dependencies/discount v2.1.8
additc https://github.com/modelon/FMILibrary.git Dependencies/FMILibrary 86929c1a32ec4a5953107e49f0d60d1a48cfeef7
additc https://github.com/tbeu/matio.git Dependencies/matio v1.5.2
additc https://github.com/osakared/qwt.git Dependencies/qwt tags/qwt-6.1.3
additc https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git Dependencies/hdf5 hdf5-1_8_18
additc https://github.com/Khan/KaTeX.git Dependencies/katex v0.7.1
addit http://git.code.sf.net/p/tclap/code Dependencies/tclap tclap-1-2-1-release-final


addit https://github.com/peterNordin/IndexingCSVParser.git HopsanCore/dependencies/IndexingCSVParser HEAD
addit https://github.com/peterNordin/libNumHop.git HopsanCore/dependencies/libNumHop HEAD

addit https://github.com/peterNordin/hopsan_deps_tools.git Dependencies/tools HEAD
addit https://github.com/Kentzo/git-archive-all.git Dependencies/git-archive-all 1.16.4

echo Done
