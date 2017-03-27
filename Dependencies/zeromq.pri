
# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
#unix:CONFIG(debug, debug|release):dbg_ext = d

# Set libpath and libname
zeromq_home = $${PWD}/zeromq_install
zeromq_lib = $${zeromq_home}/lib
zeromq_bin = $${zeromq_home}/bin
libname = zmq

defineTest(have_zeromq) {
  exists($${zeromq_lib}) {
    return(true)
  }
  return(false)
}

have_zeromq() {
  INCLUDEPATH *= $${zeromq_home}/include
  win32:LIBS *= -L$${zeromq_bin} -l$${libname}$${dbg_ext}
  unix:LIBS *= -L$${zeromq_lib} -l$${libname}$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  unix:QMAKE_RPATHDIR *= $${zeromq_lib}
  win32:QMAKE_RPATHDIR *= $${zeromq_bin}
}
