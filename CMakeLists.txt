
# ***  lgi main cmake ****
cmake_minimum_required (VERSION 3.0)
project (LGI)
add_subdirectory(src)
add_subdirectory(demos)

add_custom_target(doc COMMAND doxygen) 

install(DIRECTORY "include/lgi-1.0"
    DESTINATION "include/"
)
