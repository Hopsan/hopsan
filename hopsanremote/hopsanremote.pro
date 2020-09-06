include($${PWD}/../dependencies/zeromq-check.pri)
!have_zeromq() {
  !build_pass:warning("Failed to locate ZeroMQ libs, building WITHOUT HopsanRemote parts")
} else {

  TEMPLATE = subdirs

  SUBDIRS = \
      libhopsanremotecommon \
      libhopsanremoteclient \
      hopsanserver \
      hopsanserverworker \
      hopsanremoteclient \
      hopsanaddressserver \
      hopsanservermonitor

  libhopsanremoteclient.dependes = libhopsanremotecommon
  hopsanserver.depends = libhopsanremotecommon
  hopsanserverworker.depends = libhopsanremotecommon
  hopsanremoteclient.depends = libhopsanremoteclient
  hopsanaddressserver.depends = libhopsanremotecommon libhopsanremoteclient
  hopsanservermonitor.depends = libhopsanremoteclient

}
