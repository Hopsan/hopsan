#!/bin/bash
# $Id: unpackPatchAndBuildQwt.sh 7638 2015-02-09 10:15:01Z petno25 $

# General shell script for Mac/Linux to set topmost tool paths, primarilly for Mac
# Author: Magnus Sethson magnus.sethson@liu.se
# Date:   2015-02-20

if [ "$OSTYPE" == "darwin14" ]; then
    export hopsan_qt_path=$HOME/Qt/5.4
    export hopsan_qt_bin=$hopsan_qt_path/bin/
    export hopsan_qt_qmake=$hopsan_qt_bin/qmake
else
    export hopsan_qt_path=
    export hopsan_qt_bin=
    export hopsan_qt_qmake=qmake
fi

