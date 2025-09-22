include_directories(${CMAKE_CURRENT_SOURCE_DIR})


add_library(main OBJECT main.c)
add_library(chttpd STATIC 
  chttpd.c ${PROJECT_SOURCE_DIR}/include/chttpd/chttpd.h
  $<TARGET_OBJECTS:main>
)
target_link_libraries(chttpd PUBLIC clog pcaio chttp)
