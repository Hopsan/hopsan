INCLUDEPATH *= $${PWD}/fmi4c/include
LIBS *= -L$${PWD}/fmi4c -lfmi4c

macx {
  QMAKE_RPATHDIR *= $${PWD}/fmi4c
  # TODO: I am unable to get RPATH to work on osx, so ugly copying the dylib file for now
  QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${PWD}/fmi4c/libfmi4c.dylib) $$quote($${PWD}/../bin) $$escape_expand(\\n\\t)
} win32 {
  # On Windows, since RPATH is ignored by LoadLibrary(), copy the library file to the bin directory after build instead
  src_file = $$quote($${PWD}/fmi4c/libfmi4c.dll)
  dst_dir = $$quote($${PWD}/../bin)
  # Replace slashes in paths with backslashes for Windows
  src_file ~= s,/,\\,g
  dst_dir ~= s,/,\\,g
  QMAKE_POST_LINK += $$QMAKE_COPY $${src_file} $${dst_dir} $$escape_expand(\\n\\t)
  message("Copying $${src_file} to $${dst_dir}")
} else {
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${PWD}/fmi4c
}
