# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = ops
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT -= core
QT -= gui

TARGET = $${TARGET}$${DEBUG_EXT}

# Allow non-strict ansi code
QMAKE_CXXFLAGS *= -U__STRICT_ANSI__ -Wno-c++0x-compat

#--------------------------------------------------
# Add the include path to our self, (Ops)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {

}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    DEFINES += OPS_DLLEXPORT
    DEFINES -= UNICODE
}

# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += \
    src/OpsWorker.cpp \
    src/OpsWorkerSimplex.cpp \
    src/OpsWorkerComplexRF.cpp \
    src/OpsWorkerNelderMead.cpp \
    src/OpsEvaluator.cpp \
    src/OpsWorkerParticleSwarm.cpp \
    src/OpsWorkerComplexRFP.cpp \
    src/OpsWorkerParamterSweep.cpp \
    src/OpsWorkerDifferentialEvolution.cpp \
    src/OpsWorkerControlledRandomSearch.cpp \
    src/OpsWorkerComplexBurmen.cpp \
    src/OpsWorkerGenetic.cpp \
    src/ludcmp.cpp \
    src/matrix.cpp

HEADERS += \
    include/OpsWorker.h \
    include/OpsWorkerSimplex.h \
    include/OpsWorkerComplexRF.h \
    include/OpsWorkerNelderMead.h \
    include/OpsEvaluator.h \
    include/OpsWorkerParticleSwarm.h \
    include/OpsWorkerComplexRFP.h \
    include/OpsWorkerParameterSweep.h \
    include/OpsWorkerDifferentialEvolution.h \
    include/OpsWorkerControlledRandomSearch.h \
    include/OpsWorkerComplexBurmen.h \
    include/OpsWorkerGenetic.h \
    include/OpsMessageHandler.h \
    include/OpsWin32DLL.h \
    include/ludcmp.h \
    include/matrix.h








