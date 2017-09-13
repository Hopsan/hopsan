# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = Ops
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT -= core
QT -= gui

TARGET = $${TARGET}$${DEBUG_EXT}

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}
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
    DEFINES += OPSDLLEXPORT
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
    src/OpsWorkerComplexBurmen.cpp

HEADERS += \
    include/win32dll.h \
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
    include/OpsMessageHandler.h









