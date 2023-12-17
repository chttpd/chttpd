cmake_minimum_required(VERSION 3.7)
project(chttpd
    VERSION 0.1.0
    LANGUAGES C
)


# Suppress the warning: Policy CMP0076 is not set: target_sources() command 
# converts relative paths to absolute. 
cmake_policy(SET CMP0076 NEW)

# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_VERBOSE_MAKEFILE ON)
# TODO: handle debug flag
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fms-extensions -D_GNU_SOURCE=1 -Wall")


configure_file(manifest.h.in manifest.h)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)


add_library(chttpd)
target_sources(chttpd
    PUBLIC chttpd.c
    INTERFACE chttpd.h
    PRIVATE networking.c
            helpers.c
            router.c
            connection.c
            request.c
            response.c
            body.c
            form.c
)
target_link_libraries(chttpd
    PUBLIC
    caio
    clog
    mrb
)


# Install
install(TARGETS chttpd DESTINATION "lib")
install(FILES chttpd.h DESTINATION "include")


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


# Valgrind, Profiling
set(VALGRIND_FLAGS
    -s
    --tool=memcheck 
    --leak-check=yes 
    --show-reachable=yes 
    --num-callers=20 
    --track-fds=yes 
)


# Examples
if (NOT DEFINED ENV{SKIP_EXAMPLES})
    add_subdirectory(examples)
endif()


# Tests
if (NOT DEFINED ENV{SKIP_TESTS})
    enable_testing()
    # list(APPEND CMAKE_CTEST_ARGUMENTS "--verbose")
    add_subdirectory(tests)
    list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
endif()
