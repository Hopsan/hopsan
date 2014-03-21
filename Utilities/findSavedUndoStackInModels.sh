#!/bin/bash
# $Id$

# Shell script that is searching for saved undo stacks in model files (we usually want to remove that data as it make the files HUGE)
# Can also clear undo stack from file (may not work with nested undo stacks)
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2014-03-20

# Check for correct number of arguments
test $# -ne 1 -a $# -ne 2 && echo "Usage: `basename $0` <dirPath> clear (The second argument is optional)" && exit -1

echo "Searching for *.hmf recursively in $1"

while IFS= read -d $'\0' -r file ; do
   grep "hopsanundo" "$file"
   if [ $? -eq 0 ]; then
       echo "UndoStack saved in: $file"
       if [ "$2" == "clear" ]; then
         echo "Clearing undo from file: $file"
         awk -v flag=1 '/<hopsanundo>/ {flag=0;next} /<\/hopsanundo>/{flag=1;next} flag {print}' "$file" > tempdighw035y2pb
         mv  tempdighw035y2pb "$file"
       fi
   fi
done < <(find "$1" -name "*.hmf" -print0)
echo "Done!"