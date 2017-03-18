# Add debug if debugmode (only default on for windows)
dbg_ext = _d
#win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
pythonqt_home = $${PWD}/pythonqt_install
pythonqt_libdir = $${pythonqt_home}/lib
libname = PythonQt

defineTest(have_pythonqt) {
  exists($${pythonqt_libdir}) {
    return(true)
  }
  return(false)
}

have_qwt() {
  INCLUDEPATH *= $${pythonqt_home}/include
  LIBS *= -L$${pythonqt_libdir} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${pythonqt_libdir}
}
