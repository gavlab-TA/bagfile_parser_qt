cmake_minimum_required(VERSION 3.5)
project(TinyMAT)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

set(TinyMAT_SRCS
    src/tinymatwriter.cpp
    include/TinyMAT/tinymatwriter.h
)

add_library(${PROJECT_NAME} ${TinyMAT_SRCS})

ament_export_targets(${PROJECT_NAME} HAS_LIBRARY_TARGET)

include_directories(include)

## Install executable
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

## Install headers
#install(FILES include/serial/serial.h include/serial/v8stdint.h
#  DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/serial)

install(DIRECTORY include/
    DESTINATION include 
    FILES_MATCHING PATTERN "*.h"
    PATTERN ".svn" EXCLUDE    
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
