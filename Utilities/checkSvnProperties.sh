#!/bin/bash
# $Id$

# Shell script checks that the correct svn properties are set for the usual Hopsan file types
# The script can also set the missing properties if needed
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2014-03-21

# global variables
rootDir=`pwd`
doSetProps=0
isOK=1

function checkExactSvnProperty()
{
    #file="$1"
    #prop="$2"
    #desval="$3"
    svn info "$1" 1> /dev/null 2>&1
    if [ $? == 0 ]; then
        curval=`svn propget $2 "$1"`
        if [ "$curval" != "$3" ]; then
            echo "Wrong $2 in file: $1, ( $curval )"
            isOK=0
            if [ $doSetProps -eq 1 ]; then
                svn propset "$2" "$3" "$1"
            fi
        fi
    fi
}

function checkEOLStyleFiles()
{
    for fpat in "$@"; do
        while IFS= read -r -d $'\0' file; do
            #echo "Checking file: $file"
            checkExactSvnProperty "$file" "svn:eol-style" "native"
        done < <(find "$rootDir" -type f -name "$fpat" -print0)
    done
}

function checkCodeFiles()
{
    for fpat in "$@"; do
        while IFS= read -r -d $'\0' file; do
            #echo "Checking file: $file"
            checkExactSvnProperty "$file" "svn:eol-style" "native"
            checkExactSvnProperty "$file" "svn:keywords" "Id"
        done < <(find "$rootDir" -type f -name "$fpat" -print0)
    done
}

function checkScriptFiles()
{
    for fext in "$@"; do
        while IFS= read -r -d $'\0' file; do
            #echo "Checking file: $file"
            checkExactSvnProperty "$file" "svn:eol-style" "native"
            checkExactSvnProperty "$file" "svn:keywords" "Id"
            checkExactSvnProperty "$file" "svn:executable" "*"
        done < <(find "$rootDir" -type f -name "$fpat" -print0)
    done
}

# Check for correct number of arguments
test $# -ne 1 -a $# -ne 2 && echo "Usage: `basename $0` <dirPath> set (The second argument is optional)" && exit -1

rootDir="$1"
if [ "$2" == "set" ]; then
  doSetProps=1
fi

checkEOLStyleFiles "*.hmf" "*.xml" "*.txt" "*.pro" "*.pri" "*.prf" "*.dox"
checkCodeFiles "*.cc" "*.cpp" "*.cci" "*.h" "*.hpp" "*.hcom" "*.py"
checkScriptFiles "*.bat" "*.sh"

echo "Done!"
