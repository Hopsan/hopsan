#!/bin/bash

# Example execution:
# ./sendComplibAll.sh ~/svn/hopsan/trunk/bin/hopsandserverclient serverlist petno25 ~/svn/hopsan_private/dynafmu/hopsanComponents/dynaFmuLib.zip buildcomplib.sh

remoteclient=$1
nodelist=$2
userid=$3
libfile=$4
buildscript=$5

libname=$libfile
libname=${libname##*/}
libname=${libname%.*}

if [ -f "$nodelist" ]; then
  echo Reading servers from: $nodelist
  if [ -n "$libname" ]; then
    while read address; do
        if [[ $address != \#* ]]; then
            echo $remoteclient -s $address -u $userid -a $libfile -a $buildscript --shellexec "/bin/bash $buildscript $libname"
            $remoteclient -s $address -u $userid -a $libfile -a $buildscript --shellexec "/bin/bash $buildscript $libname"
        fi
    done < $nodelist
  else
    echo You must specify a library name \(without .zip extension\)
  fi
else
  echo Error no server list specified
fi
