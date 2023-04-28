fmi4c_dir = $${PWD}/fmi4c

exists($${fmi4c_dir}) {
  DEFINES *= USEFMI4C
  DEFINES *= FMI4C_STATIC
  INCLUDEPATH *= $${fmi4c_dir}/include

  macx {
    # Not supported
  } win32 {
    LIBS *= -L$${fmi4c_dir}/lib -lfmi4c -lzlibstatic
  } else {
    LIBS *= -L$${fmi4c_dir}/lib -lfmi4c -lz
  }
  message(Found local FMI4C)
} else {
  message(Compiling without FMI4C)
}
