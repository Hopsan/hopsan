#!/bin/bash

set -e
set -u

# Ask yes/no question, returning true or false in the global variable boolYNQuestionAnswer
boolAskYNQuestion()
{
  declare -g boolYNQuestionAnswer=false
  echo -n "$1" "Answer y/n [""$2""]: "
  read ans
  if [ -z "$ans" ]; then
    if [ "$2" = "y" ]; then
      boolYNQuestionAnswer="true"
    fi
  elif [ "$ans" = "y" ]; then
    boolYNQuestionAnswer=true
  else
    boolYNQuestionAnswer=false
  fi
}

# Ask user for version input
echo
echo -n "Enter release base version number on the form a.b.c: "
read baseversion
if [[ -z $baseversion ]]; then
    echo Error: Must give a base version
    exit 1
fi
readonly releaserevision=$(./getGitInfo.sh date.time .)
readonly fullversionname=${baseversion}.${releaserevision}

echo
boolAskYNQuestion "Do you want to build a development release?" "y"
doDevRelease="${boolYNQuestionAnswer}"

echo
echo ---------------------------------------
echo "Build DEVELOPMENT release: $doDevRelease"
echo "Release base version number: $baseversion"
echo "Release revision number: $releaserevision"
echo "Release full version string: $fullversionname"
echo ---------------------------------------
boolAskYNQuestion "Is this OK?" "n"
if [ "${boolYNQuestionAnswer}" = false ]; then
  echo Aborting!
  exit 1
fi


# Prepare snapcraft file
cp snap/local/snapcraft.yaml.in snap/snapcraft.yaml
sed "s|HOPSAN_FULL_RELEASE_VERSION|${fullversionname}|" -i snap/snapcraft.yaml
sed "s|HOPSAN_BASE_VERSION|${baseversion}|" -i snap/snapcraft.yaml
sed "s|HOPSAN_RELEASE_REVISION|${releaserevision}|" -i snap/snapcraft.yaml

sed "s|HOPSAN_DEVELOPMENT_RELEASE|${doDevRelease}|" -i snap/snapcraft.yaml
if [[ ${doDevRelease} = "true" ]]; then
  sed "s|HOPSAN_SNAP_GRADE|devel|" -i snap/snapcraft.yaml
else
  sed "s|HOPSAN_SNAP_GRADE|stable|" -i snap/snapcraft.yaml
fi

echo Done preparing snapcraft.yaml
echo
echo 'Make sure that "git status --ignored" was clean before you ran this script!'
echo 'You may want to "multipass delete snapcraft-hopsan" followed by "multipass purge"'
echo 'Now build the snap, "snapcraft snap" or "snapcraft snap --use-lxd" and then upload the snap'
echo
