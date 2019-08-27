#!/bin/bash

if [[ $# -lt 2 ]]; then
  dir="."
fi

if [[ -d "$2" ]]; then
  dir="$2"
elif [[ -f "$2" ]]; then
  dir="$2"
else
  echo Error $2 is not a file or directory
  exit 1
fi

if [[ "$1" == "shorthash" ]]; then
  git log -n1 --pretty=format:%h -- $dir
  exit $?
elif [[ "$1" == "date" ]]; then
  if [[ "$OSTYPE" == "darwin"* ]]; then
    # date is one of those system commands that differes on BSD-clones, /magse
    date -j -f %s `git log -n1 --pretty=format:%ct -- $dir` +%Y%m%d
    exit $?
  else
    date -d @`git log -n1 --pretty=format:%ct -- $dir` +%Y%m%d
    exit $?
  fi
elif [[ "$1" == "date.time" ]]; then
  if [[ "$OSTYPE" == "darwin"* ]]; then
    commitdate=$(date -j -f %s `git log -n1 --pretty=format:%ct -- $dir` +%Y%m%d)
    committime=$(date -j -f %s `git log -n1 --pretty=format:%ct -- $dir` +%H%M)
    ret_code=$?
    echo ${commitdate}.${committime}
    exit ${ret_code}
  else
    commitdate=$(date -d @`git log -n1 --pretty=format:%ct -- $dir` +%Y%m%d)
    committime=$(date -d @`git log -n1 --pretty=format:%ct -- $dir` +%H%M)
    ret_code=$?
    echo ${commitdate}.${committime}
    exit ${ret_code}
  fi
else
  echo Incorrect request: $1
  exit 1
fi


#git log --pretty=format:%cd --date=format:'%Y%m%d' -- $dir
#commitdate=`git log -n1 --pretty=format:%cd --date=short -- $dir`
#echo ${commitdate//-/}
#git log -n1 --pretty=format:%cd -- $dir
#date -d @`git log -n1 --pretty=format:%ct -- $dir` +%Y%m%d%H%M
