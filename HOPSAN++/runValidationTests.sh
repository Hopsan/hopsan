#!/bin/bash
# $Id$

# Shell script for running the validation tests on models
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-05-29

searchdir="$1"
startDir=$(pwd)
cd bin
if [ -x HopsanCLI ]; then
  cmd="./HopsanCLI_d"
elif [ -x HopsanCLI_d ]; then
  cmd="./HopsanCLI_d"
else
  echo "Error: HopsanCLI not found"
  exit 1
fi

echo "Using $cmd for evaluation"
sleep 1

# Now run HopsanCLI model unit test on all  hopsanvalidationconfig files found
while read line; do
  echo "Evaluating $line"
  $cmd -c "$line"
done < <(find "$startDir/$searchdir" -name "*.hvc")
cd $startDir


