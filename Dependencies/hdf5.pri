# Add debug if debugmode (only default on for windows)
win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
homedir = $${PWD}/hdf5
libdir = $${homedir}/lib
bindir = $${homedir}/bin
libname1 = hdf5-shared
libname2 = hdf5_cpp-shared

defineTest(have_hdf5) {
  exists($${libdir}) {
    return(true)
  }
  return(false)
}

have_hdf5() {
  INCLUDEPATH *= $${homedir}/include
  LIBS *= -L$${libdir} -L$${bindir} -l$${libname1}$${dbg_ext} -l$${libname2}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${libdir}
  QMAKE_RPATHDIR *= $${bindir}
}
