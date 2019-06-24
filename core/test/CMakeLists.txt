add_executable(libtest main.c)
link_directories(..)
target_link_libraries(libtest gpu)
