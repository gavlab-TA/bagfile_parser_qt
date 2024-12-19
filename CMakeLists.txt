cmake_minimum_required(VERSION 3.8)
project(bagfile_parser_qt VERSION 0.0.1 LANGUAGES CXX)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

cmake_policy(SET CMP0071 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosbag2_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

include_directories(include)

# List of header files that need moc
set(MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/main_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/configure_window.hpp
)

# List to store the generated moc files
set(MOC_SOURCES)

# Generate MOC files for each header in MOC_HEADERS
foreach(HEADER ${MOC_HEADERS})
  get_filename_component(HEADER_NAME ${HEADER} NAME_WE)
  set(MOC_FILE src/moc/moc_${HEADER_NAME}.cpp)

  # Add the custom command to generate the moc file for each header
  add_custom_command(
    OUTPUT ${MOC_FILE}
    COMMAND moc ${HEADER} -o ${CMAKE_SOURCE_DIR}/${MOC_FILE}
    DEPENDS ${HEADER}
    COMMENT "Generating MOC file for ${HEADER}"
  )

  # Append the generated MOC file to the list of sources
  list(APPEND MOC_SOURCES ${MOC_FILE})
endforeach()

set(MOC_SOURCES
  ${CMAKE_SOURCE_DIR}/src/moc/moc_main_window.cpp
  ${CMAKE_SOURCE_DIR}/src/moc/moc_configure_window.cpp
)

set(WINDOW_SRCS 
  src/main_window.cpp
  src/configure_window.cpp
)

add_executable(bagfile_parser_qt src/main.cpp ${WINDOW_SRCS} ${MOC_SOURCES})
target_link_libraries(bagfile_parser_qt Qt5::Widgets Qt5::Core)

ament_target_dependencies(bagfile_parser_qt rclcpp rosbag2_cpp)

install(TARGETS bagfile_parser_qt DESTINATION lib/${PROJECT_NAME})

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

write_file(${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/package_path.txt "${CMAKE_SOURCE_DIR}")
ament_package()
