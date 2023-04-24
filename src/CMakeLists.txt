find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

add_library(lgi SHARED events.cpp exceptions.cpp layer.cpp message.cpp widget.cpp x11window.cpp application.cpp utils.cpp)
set_target_properties(lgi PROPERTIES SOVERSION 1 VERSION "1.0.0")
target_include_directories(lgi PUBLIC ${CAIRO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/lgi-1.0)
target_link_libraries(lgi ${CAIRO_LIBRARIES})

install(TARGETS lgi LIBRARY DESTINATION lib)


