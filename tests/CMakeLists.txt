# Testing
include(CTest)
list(APPEND testrules
    querystring
    request_parser
    router

)
list(TRANSFORM testrules PREPEND test_)
set(profilerules ${testrules})
list(TRANSFORM profilerules APPEND _profile)
add_custom_target(profile DEPENDS ${profilerules})


foreach (t IN LISTS testrules) 
    # Test help
    add_executable(${t} ${t}.c)
    target_include_directories(${t} PUBLIC "${PROJECT_BINARY_DIR}")
    add_test(NAME ${t} COMMAND ${t})
    target_link_libraries(${t} PUBLIC clog mrb carrow)
    add_custom_target(${t}_exec 
        COMMAND ctest --verbose -R ${t}
    )
    add_custom_target(${t}_debug 
        COMMAND gdb ${t}
    )
    add_custom_target(${t}_profile
        COMMAND "valgrind" ${VALGRIND_FLAGS} ctest -R ${t}
    )
endforeach()
