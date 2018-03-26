#!/bin/bash

# General shell script for Mac/Linux to set topmost tool paths, primarilly for Mac
# By using Qt from the macports qt5-mac package there is no difference between mac and linux

if [[ ! -v QT_SELECT ]]; then
    echo Set \(and export\) QT_SELECT if you want to choose Qt version
    export QT_SELECT=0
fi

if [[ "$OSTYPE" == "darwin14" ]]; then
    export hopsan_qt_path=$HOME/Qt/5.4
    export hopsan_qt_bin=$hopsan_qt_path/bin/
    export hopsan_qt_qmake=$hopsan_qt_bin/qmake
else
    # GNU/Linux build does not use these two (relies on system installed Qt)
    export hopsan_qt_path=
    export hopsan_qt_bin=

    # Figure out what qmake variant to use, prefere qmake, if available
    # this script assumes that qmake is in fact, qtchooser
    if [[ $(command -v qmake) ]]; then
        export hopsan_qt_qmake=qmake
        echo Found qmake
    elif [[ ${QT_SELECT} -eq 5 ]] && [[ $(command -v qmake-qt5) ]]; then
        export hopsan_qt_qmake=qmake-qt5
        echo Found qmake-qt5
    elif [[ ${QT_SELECT} -eq 4 ]] && [[ $(command -v qmake-qt4) ]]; then
        export hopsan_qt_qmake=qmake-qt4
        echo Found qmake-qt4
    else
        echo Could not find qmake, defaulting to qmake-qt5
        export hopsan_qt_qmake=qmake-qt5
    fi
fi
