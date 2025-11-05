include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_library(addr OBJECT addr.c 
  ${PROJECT_SOURCE_DIR}/include/carrot/addr.h
)
add_library(config OBJECT config.c config.h.in)
add_library(socket OBJECT socket.c socket.h)
add_library(router OBJECT router.c router.h)
add_library(server_conn OBJECT server_conn.c)
add_library(server OBJECT server.c)
add_library(carrot STATIC 
  ${PROJECT_SOURCE_DIR}/include/carrot/server.h
  $<TARGET_OBJECTS:config>
  $<TARGET_OBJECTS:addr>
  $<TARGET_OBJECTS:socket>
  $<TARGET_OBJECTS:router>
  $<TARGET_OBJECTS:server_conn>
  $<TARGET_OBJECTS:server>
)
target_link_libraries(carrot PUBLIC clog pcaio mrb chttp)
