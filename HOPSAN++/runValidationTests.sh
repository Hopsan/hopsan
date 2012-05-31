#!/bin/bash
# $Id$

# Shell script for running the validation tests on models
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-05-29

failed=0
searchdir="$1"
startDir=$(pwd)
cd bin
if [ -x HopsanCLI_d ]; then
  cmd="./HopsanCLI_d"
elif [ -x HopsanCLI ]; then
  cmd="./HopsanCLI"
else
  echo "Error: HopsanCLI not found"
  exit 1
fi

echo "Using $cmd for evaluation"
#sleep 1

# Now run HopsanCLI model unit test on all  hopsanvalidationconfig files found
while read line; do
  echo "Evaluating $line"
  $cmd -c "$line"
  if [ $? -ne 0 ]; then
    failed=1
  fi
done < <(find "$startDir/$searchdir" -name "*.hvc")
cd $startDir

exit $failed

