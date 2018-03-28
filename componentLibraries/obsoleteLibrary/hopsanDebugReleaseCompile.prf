# Special options for deug and release mode
# In debug mode HopsanCore has the debug extension _d
CONFIG(debug, debug|release) {
    LIBS *= -lhopsancore_d
    DEFINES *= DEBUGCOMPILING
}
CONFIG(release, debug|release) {
    LIBS *= -lhopsancore
    DEFINES *= RELEASECOMPILING
}
