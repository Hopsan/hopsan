# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
discount_home = $${PWD}/discount
discount_lib = $${discount_home}/lib
libname = markdown

defineTest(have_local_libmarkdown) {
  exists($${discount_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_libmarkdown) {
  unix:system(ldconfig -p | grep -q lib$${libname}) {
    return(true)
  }
  return(false)
}

defineTest(have_libmarkdown) {
  have_local_libmarkdown() {
    return(true)
  }
  have_system_libmarkdown() {
    return(true)
  }
  return(false)
}

have_local_libmarkdown() {
  INCLUDEPATH *= $${discount_home}/include
  LIBS *= -L$${discount_lib} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${discount_lib}
  message(Found local libmarkdown)
} else:have_system_libmarkdown() {
  LIBS += -l$${libname}
  message(Found system ldconfig libmarkdown)
}
