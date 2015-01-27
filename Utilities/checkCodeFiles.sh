#!/bin/bash
# $Id$

# Shell script checks that the code file for some important stuff
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2014-04-02

# global variables
rootDir=`pwd`
isOK=1

function checkSvnKeywordIdPresent()
{
    #file="$1"
    grep "\$Id\:" "$1" > /dev/null
    if [ $? -eq 1 ]; then
        echo "$file is missing svn Id property line"
    fi
}

function checkCodeFiles()
{
    for fpat in "$@"; do
        while IFS= read -r -d $'\0' file; do
            #echo "Checking Code file: $file"
            checkSvnKeywordIdPresent $file
        done < <(find "$rootDir" -type f -name "$fpat" -print0)
    done
}


# Check for correct number of arguments
test $# -ne 1 && echo "Usage: `basename $0` <dirPath>" && exit -1

rootDir="$1"

checkCodeFiles "*.c" "*.cc" "*.cpp" "*.cci" "*.h" "*.hpp" "*.hcom" "*.py"

echo "Done!"
