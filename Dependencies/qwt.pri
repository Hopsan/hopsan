# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext = d

# Set hompath and libname for local install
qwt_home = $${PWD}/qwt
qwt_lib = $${qwt_home}/lib
libname = qwt

defineTest(have_local_qwt) {
  exists($${qwt_lib}) {
    return(true)
  } 
  return(false)
}


defineTest(have_system_qwt) {
  unix:system(pkg-config --exists $${libname}) {
    return(true)
  } else:unix:system(ldconfig -p | grep -q lib$${libname}) {
    return(true)
  }
  return(false)
}

defineTest(have_qwt) {
  have_local_qwt() {
    return(true)
  }
  have_system_qwt() {
    return(true)
  }
  return(false)
}

have_local_qwt() {
  #QMAKEFEATURES *= $${qwt_home}/features
  #CONFIG += qwt
  INCLUDEPATH *= $${qwt_home}/include
  LIBS *= -L$${qwt_lib} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on releaspe runtime paths should be stripped
  QMAKE_RPATHDIR *= $${qwt_lib}
  message(Found local qwt)
} else:have_system_qwt() {
  unix:CONFIG += qwt
  message(Found system qwt)
}
