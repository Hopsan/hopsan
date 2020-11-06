# Add debug if debugmode (only default on for windows)
dbg_ext =
win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath and libname
discount_home = $${PWD}/discount
discount_lib = $${discount_home}/lib
libname = markdown

defineTest(have_local_libmarkdown) {
  exists($${discount_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_libmarkdown) {
  unix:system(ldconfig -p | grep -q lib$${libname}) {
    return(true)
  }
  return(false)
}

defineTest(have_libmarkdown) {
  have_local_libmarkdown() {
    return(true)
  }
  have_system_libmarkdown() {
    return(true)
  }
  return(false)
}

have_local_libmarkdown() {
  INCLUDEPATH *= $${discount_home}/include
  LIBS *= -L$${discount_lib} -l$${libname}$${dbg_ext}
  macx {
    # TODO: I am unable to get RPATH to work on osx, so ugly copying the dylib. file for now
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($${discount_lib}/lib$${libname}$${dbg_ext}.dylib) $$quote($${PWD}/../bin) $$escape_expand(\\n\\t)
  } win32 {
    # On Windows, since RPATH is ignored by LoadLibrary(), copy the library file to the bin directory after build instead
    src_file = $$quote($${discount_home}/bin/lib$${libname}.dll)
    dst_dir = $$quote($${PWD}/../bin)
    # Replace slashes in paths with backslashes for Windows
    src_file ~= s,/,\\,g
    dst_dir ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY $${src_file} $${dst_dir} $$escape_expand(\\n\\t)
  }  else {
    # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
    QMAKE_RPATHDIR *= $${discount_lib}
  }
  message(Found local libmarkdown)
} else:have_system_libmarkdown() {
  LIBS += -l$${libname}
  message(Found system ldconfig libmarkdown)
}
