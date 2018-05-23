#!/bin/bash

# General shell script for Mac/Linux to set topmost tool paths, primarilly for Mac
# By using Qt from the macports qt5-mac package there is no difference between mac and linux

echo This bashversion $BASH_VERSION

if [[ ! $OSTYPE == darwin* ]] && [[ -z "${QT_SELECT}" ]]; then
    echo Set \(and export\) QT_SELECT if you want to choose Qt version
    echo Defaulting to Qt 5
    export QT_SELECT=5
fi

# Figure out what qmake variant to use, prefere qmake, if available.
if [[ ! -z "${HOPSAN_BUILD_QT_QMAKE}" ]]; then
    export hopsan_qt_qmake=${HOPSAN_BUILD_QT_QMAKE}
    echo Using qmake: ${hopsan_qt_qmake}
elif [[ ! -z "${HOPSAN_BUILD_QT_HOME}" ]]; then
    export hopsan_qt_qmake=${HOPSAN_BUILD_QT_HOME}/bin/qmake
    echo Using qmake: ${hopsan_qt_qmake}
elif [[ $(command -v qmake) ]]; then
    # On debian based systems this script assumes that qmake is in fact, qtchooser
    export hopsan_qt_qmake=qmake
    echo Found qmake in path
elif [[ $OSTYPE == darwin* ]]; then
    # On macOS we break bere with failure
    echo ERROR: Did not find qmake
    break
elif [[ ${QT_SELECT} -eq 5 ]] && [[ $(command -v qmake-qt5) ]]; then
    export hopsan_qt_qmake=qmake-qt5
    echo Found qmake-qt5 in path
elif [[ ${QT_SELECT} -eq 4 ]] && [[ $(command -v qmake-qt4) ]]; then
    export hopsan_qt_qmake=qmake-qt4
    echo Found qmake-qt4 in path
else
    echo Could not find qmake, defaulting to qmake-qt5
    export hopsan_qt_qmake=qmake-qt5
fi
