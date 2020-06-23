#!/bin/bash

#set -u

cd bin
# Run test programs
numfailed=0
tests=$(ls tst_*)
for test in ${tests}; do
    ./${test}
    if [[ $? -ne 0 ]]; then
        ((numfailed++))
    fi
done

# Run HopsanGUI built-in tests
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    # TODO make this work
    echo Warning: HopsanGUI tests disabled on macOS Travis CI
else
   ./hopsangui --test --platform offscreen
fi

if [[ $? -ne 0 ]]; then
    ((numfailed++))
fi

if [[ ${numfailed} -eq 0 ]]; then
    echo
    echo All tests passed
    exit 0
else
    echo
    echo ${numfailed} test applications failed
    exit 1
fi
