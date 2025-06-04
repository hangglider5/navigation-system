# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/navigation_system_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/navigation_system_autogen.dir/ParseCache.txt"
  "navigation_system_autogen"
  )
endif()
