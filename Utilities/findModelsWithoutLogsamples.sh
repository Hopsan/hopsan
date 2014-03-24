#!/bin/bash
# $Id$

# Shell script that is searching for models where num logsamples are missing
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2014-03-20

# Check for correct number of arguments
test $# -ne 1 && echo "Usage: `basename $0` <dirPath>" && exit -1

echo "Searching for *.hmf recursively in $1"

while IFS= read -d $'\0' -r file ; do
   # First check old format
   grep "logsamples" "$file" > /dev/null
   if [ $? -eq 1 ]; then
       # Now check new format
       grep "numsamples" "$file" > /dev/null
       if [ $? -eq 1 ]; then
            echo "$file is missing number of logsamples"
       fi
   fi
done < <(find "$1" -name "*.hmf" -print0)
echo "Done!"
