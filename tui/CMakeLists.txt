find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(APP_NAME "amdtop")

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

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
    fenceinfo.c
    aboutinfo.c)

add_definitions(-DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH})

add_definitions(-DSYSTEM_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}"
    -DC_COMPILER_ID="${CMAKE_C_COMPILER_ID}")

add_executable(${APP_NAME} ${srcs})
target_link_libraries( ${APP_NAME}
    utils
    pciaccess
    gpu
    m
    ${CURSES_LIBRARIES})


set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_NAME  ${APP_NAME})

set(CPACK_DEBIAN_PACKAGE_NAME ${APP_NAME})

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses5-dev, libpciaccess-dev")

set(CPACK_DEBIAN_PACKAGE_SECTTION "utils")

set(CPACK_DEBIAN_PACKAGE_PRIORITY "Optional")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AMD GPU Monitor Tool")

set(CPACK_PACKAGE_CONTACT "wangyang7902@gmail")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Wang Yang")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/tui/debian/postinst;")

message(print ${CMAKE_SOURCE_DIR}})

install(TARGETS ${APP_NAME} DESTINATION local/bin)


include(CPack)
