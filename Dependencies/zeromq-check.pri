# Set expected local install dir
zeromq_home = $${PWD}/zeromq
cppzmq_home = $${PWD}/cppzmq
zeromq_lib = $${zeromq_home}/lib
zeromq_bin = $${zeromq_home}/bin

defineTest(have_local_zeromq) {
  exists($${zeromq_lib}) {
    return(true)
  }
  return(false)
}

defineTest(have_system_zeromq) {
  packagesExist(libzmq) {
    return(true)
  }
  return(false)
}

defineTest(have_zeromq) {
  have_local_zeromq() {
    return(true)
  }
  have_system_zeromq() {
    return(true)
  }
  return(false)
}
