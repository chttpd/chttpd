cmake_minimum_required(VERSION 3.7)
project(chttpd
    VERSION 0.1.0
    LANGUAGES C
)


# Suppress the warning: Policy CMP0076 is not set: target_sources() command 
# converts relative paths to absolute. 
cmake_policy(SET CMP0076 NEW)


configure_file(manifest.h.in manifest.h)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)


add_library(chttpd)
target_sources(chttpd
    PUBLIC chttpd.c
    INTERFACE chttpd.h
    PRIVATE request.c
)
target_link_libraries(chttpd
    PUBLIC
    carrow
    clog
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
