# conpiler and flags
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fms-extensions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)


# Build type and debug symbols
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS debug release)
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE "debug" CACHE STRING "Include debug symbols" FORCE)
endif ()
if (CMAKE_BUILD_TYPE!=release)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif ()
