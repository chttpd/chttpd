list(APPEND examples
  serverdemo
  # clientdemo
)


foreach (t IN LISTS examples) 
  # Test help
  add_executable(${t} 
    ${t}.c
  )
  target_link_libraries(${t} pcaio clog carrot)
  target_include_directories(${t} PUBLIC "${PROJECT_BINARY_DIR}")

  add_custom_target(${t}-exec 
    COMMAND ./${t}
    DEPENDS ${t}
  )
  add_custom_target(${t}-debug 
    COMMAND gdb 
      --command=${PROJECT_SOURCE_DIR}/examples/${t}.gdb
      ./${t} 
    DEPENDS ${t}
  )

  add_custom_target(${t}-profile
    COMMAND "valgrind" ${VALGRIND_FLAGS} ./${t}
    DEPENDS ${t}
  )
endforeach ()
