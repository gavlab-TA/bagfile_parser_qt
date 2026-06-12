cmake_minimum_required(VERSION 3.16)
project(bagfile_parser_qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

# --- MCAP headers (vendored in external/mcap/) ---
set(MCAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/mcap/include)

# --- System dependencies ---
find_package(PkgConfig QUIET)
find_package(Threads REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)

# matio: try cmake config (vcpkg), fall back to pkg-config (Linux/macOS)
find_package(matio CONFIG QUIET)
if(NOT matio_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(MATIO REQUIRED IMPORTED_TARGET matio)
endif()
if(NOT matio_FOUND AND NOT TARGET PkgConfig::MATIO)
    message(FATAL_ERROR "Could not find matio via find_package or pkg-config")
endif()

# lz4 (optional): try cmake config, fall back to pkg-config
find_package(lz4 CONFIG QUIET)
if(NOT lz4_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(LZ4 IMPORTED_TARGET liblz4)
endif()

# zstd (optional): try cmake config, fall back to pkg-config
find_package(zstd CONFIG QUIET)
if(NOT TARGET zstd::libzstd_shared AND NOT TARGET zstd::libzstd_static AND PkgConfig_FOUND)
    pkg_check_modules(ZSTD IMPORTED_TARGET libzstd)
endif()

add_executable(bagfile_parser_qt
    src/main.cpp
    src/converter.cpp
    src/schema_parser.cpp
    src/cdr_reader.cpp
    src/mat_writer.cpp
    src/csv_writer.cpp
    src/gui.cpp
    include/bagfile_parser_qt/gui.hpp
)

target_include_directories(bagfile_parser_qt PRIVATE
    include
)
target_include_directories(bagfile_parser_qt SYSTEM PRIVATE
    ${MCAP_INCLUDE_DIR}
)

# Link matio
if(matio_FOUND)
    target_link_libraries(bagfile_parser_qt PRIVATE matio::matio)
else()
    target_link_libraries(bagfile_parser_qt PRIVATE PkgConfig::MATIO)
endif()

target_link_libraries(bagfile_parser_qt PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Threads::Threads
)

# Link lz4
if(lz4_FOUND)
    target_link_libraries(bagfile_parser_qt PRIVATE lz4::lz4)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_LZ4)
elseif(TARGET PkgConfig::LZ4)
    target_link_libraries(bagfile_parser_qt PRIVATE PkgConfig::LZ4)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_LZ4)
endif()

# Link zstd
if(TARGET zstd::libzstd_shared)
    target_link_libraries(bagfile_parser_qt PRIVATE zstd::libzstd_shared)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
elseif(TARGET zstd::libzstd_static)
    target_link_libraries(bagfile_parser_qt PRIVATE zstd::libzstd_static)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
elseif(TARGET PkgConfig::ZSTD)
    target_link_libraries(bagfile_parser_qt PRIVATE PkgConfig::ZSTD)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
endif()

install(TARGETS bagfile_parser_qt DESTINATION bin)
