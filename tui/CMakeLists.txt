find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(srcs
    main.c
    window.c
    devicewindow.c
    tabwindow.c
    mainwindow.c
    statuswindow.c)


add_executable(amdtop ${srcs})
target_link_libraries(amdtop
    utils
    ${CURSES_LIBRARIES})
