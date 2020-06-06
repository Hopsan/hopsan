# Special options for deug and release mode
# In debug mode HopsanCore has the debug extension _d
CONFIG(debug, debug|release) {
    !macx:LIBS *= -lhopsancore_d
    macx:LIBS *= -lhopsancore
    DEFINES *= HOPSAN_BUILD_TYPE_DEBUG
}
CONFIG(release, debug|release) {
    LIBS *= -lhopsancore
    DEFINES *= HOPSAN_BUILD_TYPE_RELEASE
}
DEFINES *= HOPSANCORE_DLLIMPORT
