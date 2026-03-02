ssp4c_dir = $${PWD}/ssp4c

exists($${ssp4c_dir}) {
  DEFINES *= USESSP4C
  DEFINES *= SSP4C_STATIC
  INCLUDEPATH *= $${ssp4c_dir}/include
  message(SSP includes: $${ssp4c_dir}/include)

  macx {
    # Not supported
  } win32 {
    CONFIG(debug, debug|release) {
      ssp4c_dbg_ext =
    }
    LIBS *= -L$${ssp4c_dir}/lib -lssp4c$${ssp4c_dbg_ext}
  } else {
    LIBS *= -L$${ssp4c_dir}/lib -lssp4c -lz
  }
  message(Found local ssp4c)
} else {
  message(Compiling without ssp4c)
}
