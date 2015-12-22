#!/bin/bash
# $Id$

# Shell script that counts the number of code lines recursively
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2015-12-16

let ctr=0
rootDir=$1

function doIt {
    echo -n "$1    "
    (find $rootDir -name "$1" -print0 | xargs -0 cat) | wc -l 
    let ctr+=`(find $rootDir -name "$1" -print0 | xargs -0 cat) | wc -l`
}  

echo "RootDir: $rootDir"
doIt "*.c"
doIt "*.cc"
doIt "*.h"
doIt "*.cpp"
doIt "*.hpp"
doIt "*.py"
doIt "*.sh"
doIt "*.bat"
doIt "*.hcom"
#doIt "*.dox"

echo Total = $ctr

