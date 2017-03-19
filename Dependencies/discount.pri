# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
discount_home = $${PWD}/discount_install
discount_libdir = $${discount_home}/lib
libname = markdown

defineTest(have_discount) {
  exists($${discount_libdir}) {
    return(true)
  }
  return(false)
}

have_discount() {
  INCLUDEPATH *= $${discount_home}/include
  LIBS *= -L$${discount_libdir} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${discount_libdir}
}
