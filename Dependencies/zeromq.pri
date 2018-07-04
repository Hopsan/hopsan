# Add debug extension if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
#unix:CONFIG(debug, debug|release):dbg_ext = d

include($${PWD}/zeromq-check.pri)

have_local_zeromq() {
  INCLUDEPATH *= $${zeromq_home}/include
  INCLUDEPATH *= $${cppzmq_home}
  win32:LIBS *= -L$${zeromq_bin} -lzmq$${dbg_ext}
  unix:LIBS *= -L$${zeromq_lib} -lzmq$${dbg_ext}
  macx {
    # TODO: I am unable to get RPATH to work on osx, so ugly copying the dylib. file for now
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${zeromq_lib}/libzmq$${dbg_ext}.dylib) $$quote($${PWD}/../bin) $$escape_expand(\\n\\t)
  } else {
    # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
    unix:QMAKE_RPATHDIR *= $${zeromq_lib}
    win32:QMAKE_RPATHDIR *= $${zeromq_bin}
  }
  message(Found local ZeroMQ)
} else:have_system_zeromq() {
  CONFIG *= link_pkgconfig
  PKGCONFIG *= libzmq
  message(Found system pkg-config ZeroMQ)
}
