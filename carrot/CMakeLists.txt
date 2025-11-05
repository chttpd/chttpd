include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_library(addr OBJECT addr.c 
  ${PROJECT_SOURCE_DIR}/include/carrot/addr.h
)
add_library(socket OBJECT socket.c socket.h)
add_library(router OBJECT router.c router.h)
add_library(server OBJECT server.c server.h)
add_library(carrot STATIC 
  config.h.in
  ${PROJECT_SOURCE_DIR}/include/carrot/server.h
  $<TARGET_OBJECTS:addr>
  $<TARGET_OBJECTS:socket>
  $<TARGET_OBJECTS:router>
  $<TARGET_OBJECTS:server>
)
target_link_libraries(carrot PUBLIC clog pcaio mrb chttp)
