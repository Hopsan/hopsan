#!/bin/bash

remoteclient=$1
nodelist=$2
libfile=$3
buildscript=$4

libname=$libfile
libname=${libname##*/}
libname=${libname%.*}

if [ -f "$nodelist" ]; then
  echo Reading servers from: $nodelist
  if [ -n "$libname" ]; then
    while read address; do
        if [[ $address != \#* ]]; then
            echo $remoteclient -s $address -a $libfile -a $buildscript --shellexec "/bin/sh $buildscript $libname"
            $remoteclient -s $address -a $libfile -a $buildscript --shellexec "/bin/sh $buildscript $libname"
        fi
    done < $nodelist
  else
    echo You must specify a library name \(without .zip extension\)
  fi
else
  echo Error no server list specified
fi