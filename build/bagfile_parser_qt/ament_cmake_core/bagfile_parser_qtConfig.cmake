# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_bagfile_parser_qt_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED bagfile_parser_qt_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(bagfile_parser_qt_FOUND FALSE)
  elseif(NOT bagfile_parser_qt_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(bagfile_parser_qt_FOUND FALSE)
  endif()
  return()
endif()
set(_bagfile_parser_qt_CONFIG_INCLUDED TRUE)

# output package information
if(NOT bagfile_parser_qt_FIND_QUIETLY)
  message(STATUS "Found bagfile_parser_qt: 0.0.0 (${bagfile_parser_qt_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'bagfile_parser_qt' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${bagfile_parser_qt_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(bagfile_parser_qt_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${bagfile_parser_qt_DIR}/${_extra}")
endforeach()
