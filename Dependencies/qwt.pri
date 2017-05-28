# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
#unix:CONFIG(debug, debug|release):dbg_ext = d

# Set hompath and libname
qwt_home = $${PWD}/qwt
qwt_libdir = $${qwt_home}/lib
libname = qwt

defineTest(have_qwt) {
  exists($${qwt_libdir}) {
    return(true)
  }
  return(false)
}

have_qwt() {
  INCLUDEPATH *= $${qwt_home}/include
  LIBS *= -L$${qwt_libdir} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${qwt_libdir}
}
