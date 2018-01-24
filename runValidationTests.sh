#!/bin/bash
# $Id$

# Shell script for running the validation tests on models
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-05-29

failed=0
searchdir="$1"
startDir=$(pwd)
cd bin
if [ -x hopsancli_d ]; then
  cmd="./hopsancli_d"
elif [ -x hopsancli ]; then
  cmd="./hopsancli"
else
  echo "Error: hopsancli not found"
  exit 1
fi

echo "Using $cmd for evaluation"
#sleep 1

# Now run hopsancli model unit test on all  hopsanvalidationconfig files found
echo    "**********************************************************" > valtest_failed
echo -n "Validation tests that failed: " >> valtest_failed
echo `date` >> valtest_failed
echo    "**********************************************************" >> valtest_failed
while read line; do
  #echo "Evaluating $line"
  $cmd -t "$line" > valtest_output
  if [ $? -ne 0 ]; then
    failed=1
    echo $line >> valtest_failed
  fi
  cat valtest_output | grep "Test successful: \|failed\|Failed\|Error: \|Warning: " 
done < <(find "$startDir/$searchdir" -name "*.hvc")
if [ -f valtest_output ]; then
  rm valtest_output
fi

if [ $failed -eq 1 ]; then
  setterm -foreground red
  echo
  cat valtest_failed
fi

setterm -default
cd $startDir
exit $failed

