# CPack
# https://cmake.org/cmake/help/v3.3/module/CPackDeb.html
set (CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
set (CPACK_SET_DESTDIR true)
set (CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set (CPACK_PACKAGE_CONTACT "Vahid Mardani <vahid.mardani@gmail.com>")
set (CPACK_GENERATOR DEB)
set (CPACK_DEBIAN_PACKAGE_DEPENDS "yacap, clog, caio")
include(CPack)
