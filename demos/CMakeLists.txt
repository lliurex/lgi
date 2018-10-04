

add_executable(demo1 demo1.cpp)

#require pkg-config support
find_package(PkgConfig REQUIRED)

#required pkg-config libs
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(${CMAKE_SOURCE_DIR}/include ${CAIRO_INCLUDE_DIRS})


target_link_libraries(demo1 lgi-1.0 ${CAIRO_LIBRARIES} GL X11 Xext Xcursor)

add_definitions(-std=c++11 -g ${CAIRO_CFLAGS_OTHER})
