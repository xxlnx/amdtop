include_directories(../utils)
set(libsrc
    gpudevice.c
    drmhelper.c
    gpuinfo.c
    gpumode.c
    atombios.c)
add_library(gpu STATIC ${libsrc})
target_link_libraries(gpu
    utils)

add_subdirectory(test)
