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
find_package(Boost REQUIRED COMPONENTS filesystem system)

include_directories(include)

# List of header files that need moc
set(MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/main_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/configure_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/remove_packages_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/select_topics_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/dependency_manager_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/recommended_depends_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/string_display_window.hpp
    ${CMAKE_SOURCE_DIR}/include/bagfile_parser_qt/windows/remove_depends_window.hpp
)

# List to store the generated moc files
set(MOC_SOURCES)

# Generate MOC files for each header in MOC_HEADERS
foreach(HEADER ${MOC_HEADERS})
  get_filename_component(HEADER_NAME ${HEADER} NAME_WE)
  set(MOC_FILE "${CMAKE_SOURCE_DIR}/src/windows/moc/moc_${HEADER_NAME}.cpp")

  # Ensure the moc directory exists
  file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/windows/moc)

  # Add the custom command to generate the moc file for each header
  add_custom_command(
    OUTPUT ${MOC_FILE}
    COMMAND moc ${HEADER} -o ${MOC_FILE}
    DEPENDS ${HEADER}
    COMMENT "Generating MOC file for ${HEADER}"
  )

  # Append the generated MOC file to the list of sources
  list(APPEND MOC_SOURCES ${MOC_FILE})
endforeach()

set(MOC_SOURCES
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_main_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_configure_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_remove_packages_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_select_topics_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_dependency_manager_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_recommended_depends_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_string_display_window.cpp
  ${CMAKE_SOURCE_DIR}/src/windows/moc/moc_remove_depends_window.cpp
)

set(WINDOW_SRCS 
  src/windows/main_window.cpp
  src/windows/configure_window.cpp
  src/windows/remove_packages_window.cpp
  src/windows/select_topics_window.cpp
  src/windows/dependency_manager_window.cpp
  src/windows/recommended_depends_window.cpp
  src/windows/string_display_window.cpp
  src/windows/remove_depends_window.cpp
)

set(PARSER_SRCS
  src/parser_generator/bag_analyzer.cpp
  src/parser_generator/message_analyzer.cpp
  src/parser_generator/parser_generator.cpp
  src/parser_generator/matlab_generator.cpp
)

add_executable(bagfile_parser_qt src/main.cpp ${WINDOW_SRCS} ${MOC_SOURCES} ${PARSER_SRCS})
target_link_libraries(bagfile_parser_qt Qt5::Widgets Qt5::Core Boost::filesystem Boost::system)

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
