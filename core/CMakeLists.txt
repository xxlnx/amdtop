include_directories(../utils)
set(libsrc
    gpudevice.c
    drmhelper.c
    gpuinfo.c
    gpumode.c
    atombios.c)
add_library(gpu SHARED ${libsrc})
target_link_libraries(gpu
    utils)

add_subdirectory(test)
INSTALL(FILES amdgpu.ids DESTINATION ${CMAKE_BINARY_DIR}/build/data)
