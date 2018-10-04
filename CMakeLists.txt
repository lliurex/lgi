
# ***  lgi main cmake ****
cmake_minimum_required (VERSION 2.6)
project (LGI)
add_subdirectory(src)
add_subdirectory(demos)

add_custom_target(doc COMMAND doxygen) 
