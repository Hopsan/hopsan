# Add debug if debugmode (only default on for windows)
win32:CONFIG(debug, debug|release):dbg_ext =
unix:CONFIG(debug, debug|release):dbg_ext =

# Set hompath
homedir = $${PWD}/hdf5
libdir = $${homedir}/lib
bindir = $${homedir}/bin

defineTest(have_local_hdf5) {
  exists($${libdir}) {
    return(true)
  } 
  return(false)
}

defineTest(have_system_hdf5) {
  unix:system(pkg-config --exists hdf5) {
    return(true)
  } else:unix:system(ldconfig -p | grep -q libhdf5_cpp) {
    return(true)
  }
  return(false)
}

defineTest(have_hdf5) {
  have_local_hdf5() {
    return(true)
  }
  have_system_hdf5() {
    return(true)
  }
  return(false)
}

have_local_hdf5() {
  INCLUDEPATH *= $${homedir}/include
  LIBS *= -L$${libdir} -L$${bindir} -lhdf5-shared$${dbg_ext} -lhdf5_cpp-shared$${dbg_ext}
  # Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
  QMAKE_RPATHDIR *= $${libdir}
  QMAKE_RPATHDIR *= $${bindir}
  message(Found local HDF5)
} else:have_system_hdf5() {
  unix:system(pkg-config --exists hdf5) {
    CONFIG *= link_pkgconfig
    PKGCONFIG += hdf5
    #LIBS *= $$system(pkg-config --libs hdf5)
    LIBS *= -lhdf5_cpp
    #INCLUDEPATH *= $$system(pkg-config --cflags-only-I hdf5)
    #QMAKE_CXXFLAGS *= $$system(pkg-config --cflags-only-other hdf5)
    message(Found system pkg-config HDF5)
  } else {
    LIBS += -lhdf5 -lhdf5_cpp
    message(Found system ldconfig HDF5)
  }
}
