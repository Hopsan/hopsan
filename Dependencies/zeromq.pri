
# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
#unix:CONFIG(debug, debug|release):dbg_ext = d

# Set libpath and libname
zeromq_home = $${PWD}/zeromq_install
libname = zmq

defineTest(have_zeromq) {
  exists($${zeromq_home}/lib) {
    return(true)
  }
  return(false)
}

have_zeromq() {
  INCLUDEPATH *= $${zeromq_home}/include
  LIBS *= -L$${zeromq_home}/lib -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${zeromq_home}/lib
}
