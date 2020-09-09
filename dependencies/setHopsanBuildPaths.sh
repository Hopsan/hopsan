#!/bin/bash

# General shell script for Mac/Linux to set topmost tool paths, primarily for Mac
# By using Qt from the macports qt5-mac package there is no difference between macOS and Linux

echo Using Bash version $BASH_VERSION

if [[ ! $OSTYPE == darwin* ]] && [[ -z "${QT_SELECT}" ]]; then
    echo Set \(and export\) QT_SELECT if you want to choose Qt version
    echo Defaulting to Qt 5
    export QT_SELECT=5
fi

# Figure out what qmake variant to use, prefer qmake, if available.
if [[ ! -z "${HOPSAN_BUILD_QT_QMAKE}" ]]; then
    export HOPSAN_BUILD_QT_QMAKE=${HOPSAN_BUILD_QT_QMAKE}
    echo Using qmake: ${HOPSAN_BUILD_QT_QMAKE}
elif [[ ! -z "${HOPSAN_BUILD_QT_HOME}" ]]; then
    export HOPSAN_BUILD_QT_QMAKE=${HOPSAN_BUILD_QT_HOME}/bin/qmake
    echo Using qmake: ${HOPSAN_BUILD_QT_QMAKE}
elif [[ $(command -v qmake) ]]; then
    # On debian based systems this script assumes that qmake is in fact, qtchooser
    export HOPSAN_BUILD_QT_QMAKE=qmake
    echo Found qmake in PATH
    qmake --version
elif [[ $OSTYPE == darwin* ]]; then
    # On macOS we break here with failure
    echo ERROR: Did not find qmake, aborting!
    exit 1
elif [[ ${QT_SELECT} -eq 5 ]] && [[ $(command -v qmake-qt5) ]]; then
    export HOPSAN_BUILD_QT_QMAKE=qmake-qt5
    echo Found qmake-qt5 in PATH
elif [[ ${QT_SELECT} -eq 4 ]] && [[ $(command -v qmake-qt4) ]]; then
    export HOPSAN_BUILD_QT_QMAKE=qmake-qt4
    echo Found qmake-qt4 in PATH
else
    echo Could not find qmake, attempting to use qmake-qt5
    export HOPSAN_BUILD_QT_QMAKE=qmake-qt5
fi

export CTEST_PARALLEL_LEVEL=$(getconf _NPROCESSORS_ONLN)
export CMAKE_BUILD_PARALLEL_LEVEL=$(getconf _NPROCESSORS_ONLN)
