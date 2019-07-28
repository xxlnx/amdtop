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
    ringinfo.c
    firmwareinfo.c
    systeminfo.c
    displayinfo.c
    aboutinfo.c)


add_executable(amdtop ${srcs})
target_link_libraries(amdtop
    utils
    pciaccess
    gpu
    m
    ${CURSES_LIBRARIES})

set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_PACKAGE_NAME "amdtop")

set(CPACK_DEBIAN_PACKAGE_NAME "amdtop")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses5-dev, libpciaccess-dev")

set(CPACK_DEBIAN_PACKAGE_SECTTION "utils")

set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AMD GPU Monitor Tool")

set(CPACK_PACKAGE_CONTACT "wangyang7902@gmail")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Wang Yang")

install(TARGETS amdtop DESTINATION local/bin)

include(CPack)

