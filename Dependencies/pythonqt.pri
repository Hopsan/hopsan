# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext = _d
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
pythonqt_home = $${PWD}/pythonqt
pythonqt_lib = $${pythonqt_home}/lib
libname = PythonQt

defineTest(have_local_pythonqt) {
  exists($${pythonqt_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_pythonqt) {
  packagesExist(PythonQt-Qt5-Python3) {
    return(true)
  }
  return(false)
}

defineTest(have_pythonqt) {
  have_local_pythonqt() {
    return(true)
  }
  have_system_pythonqt() {
    return(true)
  }
  return(false)
}

have_local_pythonqt() {
  INCLUDEPATH *= $${pythonqt_home}/include
  #TODO Use prf instead
  exists($${pythonqt_lib}/libPythonQt-Qt5-Python3$${dbg_ext}.so) {
    LIBS *= -L$${pythonqt_lib} -lPythonQt-Qt5-Python3$${dbg_ext}
    PYTHONQT_PYVERSION=3
    export(PYTHONQT_PYVERSION)
  } else {
    PYTHONQT_PYVERSION=2
    export(PYTHONQT_PYVERSION)
    LIBS *= -L$${pythonqt_lib} -l$${libname}$${dbg_ext}
  }
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${pythonqt_lib}
  message(Found local PythonQt)
} else:have_system_pythonqt() {
  CONFIG *= link_pkgconfig
  PKGCONFIG *= PythonQt-Qt5-Python3
  PYTHONQT_PYVERSION=3
  export(PYTHONQT_PYVERSION)
  message(Found system pkg-config PythonQt)
}
