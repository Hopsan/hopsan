#!/bin/bash

set -u
set -e

if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` srcDir python_version"
  exit 1
fi

src_dir="${1}"
python_version=${2}

#echo $src_dir
#echo $python_version

if [ "${python_version}" == "2" ]; then
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/env python3$|#!/usr/bin/python|' -i {} \;
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/env python$|#!/usr/bin/python|' -i {} \;
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/python3$|#!/usr/bin/python|' -i {} \;
elif [ "${python_version}" == "3" ]; then
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/env python3$|#!/usr/bin/python3|' -i {} \;
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/env python$|#!/usr/bin/python3|' -i {} \;
  find ${src_dir} -name "*.py" -exec sed 's|#!/usr/bin/python$|#!/usr/bin/python3|' -i {} \;
else
  echo "Error: Incorrect Python version: ${python_version} given. Must be 2 or 3"
  exit 1
fi
