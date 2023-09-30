list(APPEND examples
    tcp
    unix
)
set(profilerules ${examples})
list(TRANSFORM profilerules PREPEND profile_)


foreach (t IN LISTS examples) 
    # Test help
    add_executable(${t} ${t}.c)
    target_include_directories(${t} PUBLIC "${PROJECT_BINARY_DIR}")
    target_link_libraries(${t} PUBLIC clog chttpd carrow)
    add_custom_target(${t}_exec COMMAND ${t})
    add_custom_target(${t}_profile
        COMMAND "valgrind" ${VALGRIND_FLAGS} ./${t}
    )
    add_custom_target(${t}_debug 
        COMMAND gdb ./${t}
    )
endforeach()
