find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(srcs
    main.c
    window.c
    device.c
    context.c
    devicewindow.c
    tabwindow.c
    mainwindow.c
    statuswindow.c
    tabinfo.c
    deviceinfo.c
    driverinfo.c
    biosinfo.c
    stateinfo.c
    chipinfo.c
    firmware.c
    systeminfo.c
    aboutinfo.c)


add_executable(amdtop ${srcs})
target_link_libraries(amdtop
    utils
    pciaccess
    gpu
    ${CURSES_LIBRARIES})

