include( $${PWD}/../Common.prf )

# Set thisDir to the directory of the pri file, else lookup will use PWD from the calling project file
thisDir = $${PWD}

#Helpfunction to fetch correct QWT INCLUDEPATH and LIBS path
defineReplace(setZMQPathInfo){
    #Assign input arguments
    externalSrc = $$1
    dstDir = $$2

    #Set QWT paths, Paths that are higher up in the list will have priority if found
    win32:contains(QMAKE_HOST.arch, x86_64){
        # 64 bit Windows release
        ZMQ_PATHS *= $${thisDir}/../Dependencies/zeromq4-1-4.1.3_x64
    } else {
        # 32 bit Windows version or Linux / Mac
        ZMQ_PATHS *= $${thisDir}/../Dependencies/zeromq4-1-4.1.3
    }
    ZMQ_PATH = $$selectPath($$externalSrc, $$ZMQ_PATHS, "zmq")

    #warning(ZMQ_PATH $${ZMQ_PATH})

    #Empty variables to fill in
    magic_hopsan_libpath =
    magic_hopsan_includepath =
    magic_hopsan_qmake_post_link =
    libDir =

    exists($${ZMQ_PATH}){
        libDir = $${ZMQ_PATH}/.libs
        #message(libDir: $${libDir})

        #Check libfolder path
        exists($${libDir}){
            files =
            libName = zmq

            #Add debug if debugmode (only default on for windows)
            dbg_ext =
            win32:CONFIG(debug, debug|release):dbg_ext =
            #unix:CONFIG(debug, debug|release):dbg_ext = d

            # OS X uses the Frameworks concept instead of lib&include directories. A security meassure.
            #macx {
            #    magic_hopsan_libpath = $${libDir}
            #    magic_hopsan_includepath = $${QWT_PATH}
            #}

            # Standard way of finding qwt lib&include
            #!macx {
                magic_hopsan_libpath = -L$${libDir} -l$${libName}$${dbg_ext}
                magic_hopsan_includepath = $${ZMQ_PATH}/include
            #}

            #Set files to copy,  only zmq[d].dll/libzmq[d].so
            win32:files = lib$${libName}$${dbg_ext}$${DLL_EXT}
            unix:files = lib$${libName}$${dbg_ext}$${DLL_EXT}*
            # On mac we need all of the framework
            #macx:files = $${libName}.framework

            #Generate the copy command
            magic_hopsan_qmake_post_link = $$generateCopyDllCommand($$files, $$libDir, $$dstDir)
            #message(qmake_post_link: $${magic_hopsan_qmake_post_link})
            #message(files:$$files)
        }
    }

    !exists($${libDir}){
        !build_pass:warning("Could not find your compiled zmq dll")
        magic_hopsan_libpath =
        magic_hopsan_includepath =
        magic_hopsan_qmake_post_link =
    }

    #Export lists to INCLUDEPATH, LIBS and QMAKE_POST_LINK variables
    export(magic_hopsan_libpath)
    export(magic_hopsan_includepath)
    export(magic_hopsan_qmake_post_link)

    #message(----------------Includepath is $${magic_hopsan_includepath})
    #message(----------------Libs is $${magic_hopsan_libpath})

    return($$magic_hopsan_libpath)
}

INCLUDEPATH *= $${PWD}/../Dependencies/msgpack-c-cpp-1.3.0/include/
INCLUDEPATH *= $${PWD}/../Dependencies/cppzmq-master/
