include($${PWD}/../Dependencies/zeromq-check.pri)
!have_zeromq() {
  !build_pass:warning("Failed to locate ZeroMQ libs, building WITHOUT HopsanRemote parts")
} else {

  TEMPLATE = subdirs

  CONFIG += ordered
  SUBDIRS = \
      libhopsanremotecommon \
      libhopsanremoteclient \
      hopsanserver \
      hopsanserverworker \
      hopsanremoteclient \
      hopsanaddressserver \
      hopsanservermonitor
}
