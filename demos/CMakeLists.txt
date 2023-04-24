
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

add_executable(demo1 demo1.cpp)
target_link_libraries(demo1 lgi)

target_include_directories(demo1 PUBLIC ${CAIRO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(demo1 ${CAIRO_LIBRARIES} GL X11 Xext Xcursor)

#include_directories(${CMAKE_SOURCE_DIR}/include ${CAIRO_INCLUDE_DIRS})
#target_link_libraries(demo1 lgi-1.0 ${CAIRO_LIBRARIES} GL X11 Xext Xcursor)

