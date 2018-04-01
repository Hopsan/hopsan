#!/bin/bash

set -e
set -u

cd bin
tests=$(ls tst_*)
for test in ${tests}; do
    ./${test}
done
echo All tests passed
