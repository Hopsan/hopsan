# Add debug extension if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
#unix:CONFIG(debug, debug|release):dbg_ext = d

# Set expected local install dir
zeromq_home = $${PWD}/zeromq
cppzmq_home = $${PWD}/cppzmq
zeromq_lib = $${zeromq_home}/lib
zeromq_bin = $${zeromq_home}/bin

defineTest(have_local_zeromq) {
  exists($${zeromq_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_zeromq) {
  packagesExist(libzmq) {
    return(true)
  }
  return(false)
}

defineTest(have_zeromq) {
  have_local_zeromq() {
    return(true)
  }
  have_system_zeromq() {
    return(true)
  }
  return(false)
}

have_local_zeromq() {
  INCLUDEPATH *= $${zeromq_home}/include
  INCLUDEPATH *= $${cppzmq_home}
  win32:LIBS *= -L$${zeromq_bin} -lzmq$${dbg_ext}
  unix:LIBS *= -L$${zeromq_lib} -lzmq$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  unix:QMAKE_RPATHDIR *= $${zeromq_lib}
  win32:QMAKE_RPATHDIR *= $${zeromq_bin}
  message(Found local ZeroMQ)
} else:have_system_zeromq() {
  CONFIG *= link_pkgconfig
  PKGCONFIG *= libzmq
  message(Found system pkg-config ZeroMQ)
}
