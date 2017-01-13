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

#git reset --hard HEAD


addit https://github.com/msgpack/msgpack-c.git Dependencies/msgpack-c cpp-1.3.0
addit https://github.com/zeromq/cppzmq.git Dependencies/cppzmq HEAD
addit https://github.com/zeromq/zeromq4-1.git Dependencies/zeromq v4.1.6
addit https://github.com/Orc/discount.git Dependencies/discount v2.1.8
addit https://github.com/peterNordin/IndexingCSVParser.git Dependencies/IndexingCSVParser HEAD
addit https://github.com/peterNordin/libNumHop.git Dependencies/libNumHop HEAD
addit https://github.com/modelon/FMILibrary.git Dependencies/FMILibrary 86929c1a32ec4a5953107e49f0d60d1a48cfeef7
addit https://github.com/tbeu/matio.git Dependencies/matio v1.5.2
addit https://github.com/osakared/qwt.git Dependencies/qwt tags/qwt-6.1.3
addit https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git Dependencies/hdf5 hdf5-1_8_18
addit https://github.com/Khan/KaTeX.git Dependencies/katex v0.7.0
addit http://git.code.sf.net/p/tclap/code Dependencies/tclap tclap-1-2-1-release-final
#addit Dependencies/
echo Done
