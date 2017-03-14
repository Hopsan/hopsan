
# Set home path
msgpack_home = $${PWD}/msgpack-c

defineTest(have_msgpack) {
  exists($${msgpack_home}/include) {
    return(true)
  }
  return(false)
}

have_msgpack() {
  INCLUDEPATH *= $${msgpack_home}/include
}
