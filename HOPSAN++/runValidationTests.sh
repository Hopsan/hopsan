#!/bin/bash
# $Id$

# Shell script for running the validation tests on models
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-05-29

searchdir="$1"

startDir=`pwd`
cliPath="$startDir"/bin/
while read line; do
  echo $line 
  txtline=${line%.hmf}.txt
  echo  $txtline
  if [ -f "$txtline" ]; then
    echo "Text exist"
    basepath=${line%/*}
    file=${line##*/}
    name=${file%.hmf}

    echo $basepath
    echo $file
    echo $name
  
    cd "$basepath"
    pwd

    echo "$cliPath"/HopsanCLI_d -t "$name"
    "$cliPath"/HopsanCLI_d -t "$name"

    cd "$startDir"
    
  fi

  pwd

done < <(find "$searchdir" -name "*.hmf")

