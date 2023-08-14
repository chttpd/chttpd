cmake_minimum_required(VERSION 3.7)
project(chttpd
    VERSION 0.1.0
    LANGUAGES C
)


configure_file(manifest.h.in manifest.h)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)


add_executable(chttpd chttpd.c)
target_link_libraries(
    chttpd
    PUBLIC
    carrow
    clog
    microhttpd
)


# Lint
set(PRETTYC_FLAGS
    --recursive
    --verbose=0
    --repository=.
    --extensions=c,h,in
    --linelength=80
    --headers=h,in
    --includeorder=standardcfirst
    --root=.
    #--quiet
    ${PROJECT_SOURCE_DIR}
)
add_custom_target(lint
    COMMAND prettyc
    ${PRETTYC_FLAGS}
)
