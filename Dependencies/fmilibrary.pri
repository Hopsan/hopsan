
fmi_home = $${PWD}/FMILibrary_install

INCLUDEPATH *= $${fmi_home}/include
LIBS *= -L$${fmi_home}/lib -lfmilib_shared
# Note! The RPATH is absolute and only meant for dev builds in the IDE, on release runtime paths should be stripped
QMAKE_RPATHDIR *= $${fmi_home}/lib
