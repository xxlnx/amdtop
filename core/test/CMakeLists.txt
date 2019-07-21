add_executable(libtest main.c)
add_executable(testdrmdebug drmdebug.c)
add_executable(testdrmmode testdrmmode.c)

link_directories(..)
target_link_libraries(libtest gpu)
target_link_libraries(testdrmdebug gpu)
target_link_libraries(testdrmmode gpu)
