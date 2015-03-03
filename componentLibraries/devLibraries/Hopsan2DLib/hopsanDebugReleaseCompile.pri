# Special options for deug and release mode
# In debug mode HopsanCore has the debug extension _d
CONFIG(debug, debug|release) {
    !macx:LIBS *= -lHopsanCore_d
    macx:LIBS *= -lHopsanCore
    DEFINES *= DEBUGCOMPILING
}
CONFIG(release, debug|release) {
    LIBS *= -lHopsanCore
    DEFINES *= RELEASECOMPILING
}
