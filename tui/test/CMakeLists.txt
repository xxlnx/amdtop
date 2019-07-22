add_executable(xlibtest xlibtest.c)

link_directories(..)
target_link_libraries(xlibtest X11 utils)
