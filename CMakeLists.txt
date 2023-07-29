cmake_minimum_required(VERSION 3.7)
project(chttpd
    VERSION 0.1.0
    LANGUAGES C
)


configure_file(manifest.h.in manifest.h)


add_executable(chttpd chttpd.c routes.c handlers.c)
target_link_libraries(
    chttpd
    PUBLIC
    carrow
    clog
    microhttpd
)
