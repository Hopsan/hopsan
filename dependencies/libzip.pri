libzip_dir = $${PWD}/libzip
zlib_dir = $${PWD}/zlib
message(libzip_dir)
exists($${libzip_dir}) {
  INCLUDEPATH *= $${libzip_dir}/include
  LIBS *= -L$${libzip_dir}/bin -L$${libzip_dir}/lib -lzip

  macx {
  # Not supported
  } win32 {
    src_file = $$quote($${libzip_dir}/bin/libzip.dll)
    src_file_zlib = $$quote($${zlib_dir}/bin/libzlib.dll)
    dst_dir = $$quote($${PWD}/../bin)
    # Replace slashes in paths with backslashes for Windows
    src_file ~= s,/,\\,g
    src_file_zlib ~= s,/,\\,g
    dst_dir ~= s,/,\\,g

    QMAKE_POST_LINK += $$QMAKE_COPY $${src_file} $${dst_dir} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $${src_file_zlib} $${dst_dir} $$escape_expand(\\n\\t)
  } else {
    # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
    unix:QMAKE_RPATHDIR *= $${libzip_dir/bin}
  }
}

