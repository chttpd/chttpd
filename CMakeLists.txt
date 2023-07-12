cmake_minimum_required(VERSION 3.7)
project(chttpd
    LANGUAGES C
)

add_executable(chttpd chttpd.c)
target_link_libraries(
    chttpd
    PUBLIC
    carrow
    clog
    microhttpd
)
