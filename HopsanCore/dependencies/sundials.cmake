set(sundials_dir ${CMAKE_CURRENT_LIST_DIR}/sundials)
set(sundials_extra_dir ${CMAKE_CURRENT_LIST_DIR}/sundials-extra)

set(sundials_c
    ${sundials_dir}/src/kinsol/kinsol_spils.c
    ${sundials_dir}/src/kinsol/kinsol_ls.c
    ${sundials_dir}/src/kinsol/kinsol_io.c
    ${sundials_dir}/src/kinsol/kinsol_direct.c
    ${sundials_dir}/src/kinsol/kinsol_bbdpre.c
    ${sundials_dir}/src/kinsol/kinsol.c
    ${sundials_dir}/src/sunmatrix/dense/fsunmatrix_dense.c
    ${sundials_dir}/src/sunmatrix/dense/sunmatrix_dense.c
    ${sundials_dir}/src/nvector/serial/fnvector_serial.c
    ${sundials_dir}/src/nvector/serial/nvector_serial.c
    ${sundials_dir}/src/sundials/sundials_futils.c
    ${sundials_dir}/src/sundials/sundials_nvector_senswrapper.c
    ${sundials_dir}/src/sundials/sundials_nonlinearsolver.c
    ${sundials_dir}/src/sundials/sundials_version.c
    ${sundials_dir}/src/sundials/sundials_iterative.c
    ${sundials_dir}/src/sundials/sundials_band.c
    ${sundials_dir}/src/sundials/sundials_direct.c
    ${sundials_dir}/src/sundials/sundials_dense.c
    ${sundials_dir}/src/sundials/sundials_nvector.c
    ${sundials_dir}/src/sundials/sundials_linearsolver.c
    ${sundials_dir}/src/sundials/sundials_matrix.c
    ${sundials_dir}/src/sundials/sundials_math.c
    ${sundials_dir}/src/sunmatrix/band/sunmatrix_band.c
    ${sundials_dir}/src/sunlinsol/band/sunlinsol_band.c
    ${sundials_dir}/src/sunlinsol/dense/sunlinsol_dense.c
)


function(add_sundials_src target)
  file(GLOB_RECURSE sundials_headers ${sundials_dir}/include/*.h ${sundials_extra_dir}/include/*.h)
  target_sources(${target} PRIVATE ${sundials_c} ${sundials_headers})

  target_include_directories(${target} PRIVATE
    $<BUILD_INTERFACE:${sundials_dir}/include>
    $<BUILD_INTERFACE:${sundials_extra_dir}/include>)
endfunction()

function(install_sundials_src destination)
    install(DIRECTORY ${sundials_dir}/src DESTINATION ${destination} )
    install(DIRECTORY ${sundials_dir}/include DESTINATION ${destination} )
    install(DIRECTORY ${sundials_extra_dir}/include DESTINATION ${destination} )
endfunction()
