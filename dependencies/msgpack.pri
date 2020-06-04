
# Set home path
msgpack_home = $${PWD}/msgpack-c

defineTest(have_local_msgpack) {
  exists($${msgpack_home}/include) {
    return(true)
  }
  return(false)
}

defineTest(have_system_msgpack) {
  packagesExist(msgpack) {
    return(true)
  }
  return(false)
}

defineTest(have_msgpack) {
  have_local_msgpack() {
    return(true)
  }
  have_system_msgpack() {
    return(true)
  }
  return(false)
}

have_local_msgpack() {
  INCLUDEPATH *= $${msgpack_home}/include
  message(Found local msgpack-c)
} else:have_system_msgpack() {
  CONFIG *= link_pkgconfig
  PKGCONFIG *= msgpack
  message(Found system pkg-config msgpack)
}
