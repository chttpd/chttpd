include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_library(addr OBJECT addr.c 
  ${PROJECT_SOURCE_DIR}/include/chttpd/addr.h
)
add_library(config OBJECT config.c config.h.in)
add_library(socket OBJECT socket.c socket.h)
add_library(connection OBJECT connection.c connection.h)
add_library(router OBJECT router.c router.h)
add_library(response OBJECT response.c)
add_library(request OBJECT request.c)
add_library(main OBJECT main.c)
add_library(chttpd STATIC 
  chttpd.c ${PROJECT_SOURCE_DIR}/include/chttpd/chttpd.h
  $<TARGET_OBJECTS:config>
  $<TARGET_OBJECTS:addr>
  $<TARGET_OBJECTS:socket>
  $<TARGET_OBJECTS:connection>
  $<TARGET_OBJECTS:router>
  $<TARGET_OBJECTS:response>
  $<TARGET_OBJECTS:request>
  $<TARGET_OBJECTS:main>
)
target_link_libraries(chttpd PUBLIC clog pcaio mrb chttp)
