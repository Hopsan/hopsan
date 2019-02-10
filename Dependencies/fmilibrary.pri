fmi_home = $${PWD}/fmilibrary

INCLUDEPATH *= $${fmi_home}/include
LIBS *= -L$${fmi_home}/lib -lfmilib_shared

macx {
  QMAKE_RPATHDIR *= $${fmi_home}/lib
  # TODO: I am unable to get RPATH to work on osx, so ugly copying the dylib file for now
  QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${fmi_home}/lib/libfmilib_shared.dylib) $$quote($${PWD}/../bin) $$escape_expand(\\n\\t)
} else {
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${fmi_home}/lib
}
