
#main lgi lib
add_library(lgi-1.0 SHARED events.cpp exceptions.cpp layer.cpp message.cpp widget.cpp x11window.cpp application.cpp utils.cpp)


#require pkg-config support
find_package(PkgConfig REQUIRED)

#required pkg-config libs
pkg_check_modules(CAIRO REQUIRED cairo)

#add include dirs
include_directories(${CMAKE_SOURCE_DIR}/include/lgi-1.0 ${CAIRO_INCLUDE_DIRS})

#add custom lib flags
add_definitions(-std=c++11 -O3 ${CAIRO_CFLAGS_OTHER})

