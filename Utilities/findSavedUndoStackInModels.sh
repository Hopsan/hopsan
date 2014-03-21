#!/bin/bash
# $Id$

# Shell script that is searching for saved undo stacks in model files (we usually want to remove that data as it make the files HUGE)
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2014-03-20

echo "Searching for *.hmf recursively in $1"

while IFS= read -d $'\0' -r file ; do
   grep "hopsanundo" "$file"
   if [ $? -eq 0 ]; then
       echo "UndoStack saved in: $file"
   fi
done < <(find "$1" -name "*.hmf" -print0)
echo "Done!"