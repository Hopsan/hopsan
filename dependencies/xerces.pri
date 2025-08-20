xerces_dir = $${PWD}/xerces

exists($${xerces_dir}) {
  INCLUDEPATH *= $${xerces_dir}/include
  LIBS *= -L$${xerces_dir}/bin -L$${xerces_dir}/lib -lxerces-c

  macx {
  # Not supported
  } win32 {
    src_file = $$quote($${xerces_dir}/bin/libxerces-c.dll)
    dst_dir = $$quote($${PWD}/../bin)
    # Replace slashes in paths with backslashes for Windows
    src_file ~= s,/,\\,g
    dst_dir ~= s,/,\\,g

    message($${src_file})
    message($${dst_dir})

    QMAKE_POST_LINK += $$QMAKE_COPY $${src_file} $${dst_dir} $$escape_expand(\\n\\t)
  } else {
    # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
    unix:QMAKE_RPATHDIR *= $${xerces_dir/bin}
    unix:QMAKE_RPATHDIR *= $${xerces_dir/lib}
  }
}

