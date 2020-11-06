fmi_home = $${PWD}/fmilibrary

INCLUDEPATH *= $${fmi_home}/include
LIBS *= -L$${fmi_home}/lib -lfmilib_shared

macx {
  QMAKE_RPATHDIR *= $${fmi_home}/lib
  # TODO: I am unable to get RPATH to work on osx, so ugly copying the dylib file for now
  QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${fmi_home}/lib/libfmilib_shared.dylib) $$quote($${PWD}/../bin) $$escape_expand(\\n\\t)
} win32 {
  # On Windows, since RPATH is ignored by LoadLibrary(), copy the library file to the bin directory after build instead
  src_file = $$quote($${fmi_home}/lib/libfmilib_shared.dll)
  dst_dir = $$quote($${PWD}/../bin)
  # Replace slashes in paths with backslashes for Windows
  src_file ~= s,/,\\,g
  dst_dir ~= s,/,\\,g
  QMAKE_POST_LINK += $$QMAKE_COPY $${src_file} $${dst_dir} $$escape_expand(\\n\\t)
} else {
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${fmi_home}/lib
}
