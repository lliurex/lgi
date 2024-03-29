
find_library(LGI_BASE_LIB "lgi")
set(LLIUREX_LGI_INCLUDE_DIRS "/usr/include/lgi-1.0/")
set(LLIUREX_LGI_LIBRARIES ${LGI_BASE_LIB})

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

add_library(Lliurex::LGI SHARED IMPORTED)
set_target_properties(Lliurex::LGI PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${LLIUREX_LGI_INCLUDE_DIRS}
    INTERFACE_LINK_LIBRARIES "Lliurex::LGI;${CAIRO_LIBRARIES};GL;X11;Xext;Xcursor"
    IMPORTED_LOCATION ${LLIUREX_LGI_LIBRARIES}
)
