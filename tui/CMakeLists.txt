set(srcs main.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(amdtop ${srcs})
target_link_libraries(amdtop
    ${CURSES_LIBRARIES})
