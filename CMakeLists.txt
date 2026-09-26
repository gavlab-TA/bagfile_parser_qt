cmake_minimum_required(VERSION 3.16)
project(bagfile_parser_qt VERSION 0.1.0 LANGUAGES CXX)

# Lets CI stamp a release tag onto the packages without editing this file.
set(BPQ_VERSION "" CACHE STRING "Override the version used for packaging")
if(BPQ_VERSION)
    set(PROJECT_VERSION "${BPQ_VERSION}")
endif()

set(BPQ_DISPLAY_NAME "Bagfile Parser")
set(BPQ_VENDOR "GAVLab")
set(BPQ_HOMEPAGE "https://github.com/gavlab-TA/bagfile_parser_qt")

# Default to Release when the generator is single-config and the user said nothing.
get_property(_bpq_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _bpq_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BPQ_MACOS_BUNDLE "Build a macOS .app bundle" ${APPLE})
# ON when staging a drag-installable .dmg: puts the .app at the root of the
# install tree next to an /Applications symlink, instead of <prefix>/Applications.
option(BPQ_MACOS_DMG_LAYOUT "Lay the install tree out for a drag-install .dmg" OFF)

include(GNUInstallDirs)

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
    src/reindex.cpp
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

# Link lz4. If absent, tell MCAP not to #include <lz4frame.h> (it does so
# unconditionally otherwise), so the build still works without LZ4 support.
if(lz4_FOUND)
    target_link_libraries(bagfile_parser_qt PRIVATE lz4::lz4)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_LZ4)
elseif(TARGET PkgConfig::LZ4)
    target_link_libraries(bagfile_parser_qt PRIVATE PkgConfig::LZ4)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_LZ4)
else()
    target_compile_definitions(bagfile_parser_qt PRIVATE MCAP_COMPRESSION_NO_LZ4)
endif()

# Link zstd. If absent, tell MCAP not to #include <zstd.h> (it does so
# unconditionally otherwise), so the build still works without Zstd support.
if(TARGET zstd::libzstd_shared)
    target_link_libraries(bagfile_parser_qt PRIVATE zstd::libzstd_shared)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
elseif(TARGET zstd::libzstd_static)
    target_link_libraries(bagfile_parser_qt PRIVATE zstd::libzstd_static)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
elseif(TARGET PkgConfig::ZSTD)
    target_link_libraries(bagfile_parser_qt PRIVATE PkgConfig::ZSTD)
    target_compile_definitions(bagfile_parser_qt PRIVATE HAS_ZSTD)
else()
    target_compile_definitions(bagfile_parser_qt PRIVATE MCAP_COMPRESSION_NO_ZSTD)
endif()


# =====================================================================
# Bundling, install rules and packaging
# =====================================================================

set(BPQ_ICON_DIR ${CMAKE_SOURCE_DIR}/packaging/icons)

# --- Locate Qt's deployment tool (windeployqt / macdeployqt) ---
# These ship next to qmake in every Qt install, but Qt exports no target for
# them, so derive the path from qmake's location.
function(bpq_find_deploy_tool _tool _out_var)
    if(TARGET Qt${QT_VERSION_MAJOR}::qmake)
        get_target_property(_qmake Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
        find_program(${_out_var} NAMES ${_tool} HINTS "${_qt_bin}" NO_DEFAULT_PATH)
    endif()
    find_program(${_out_var} NAMES ${_tool})
    set(${_out_var} "${${_out_var}}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------
# macOS: build a self-contained .app bundle
# ---------------------------------------------------------------------
if(APPLE AND BPQ_MACOS_BUNDLE)
    set_source_files_properties(${BPQ_ICON_DIR}/bagfile_parser_qt.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(bagfile_parser_qt PRIVATE ${BPQ_ICON_DIR}/bagfile_parser_qt.icns)

    set(MACOSX_BUNDLE_BUNDLE_NAME          "${BPQ_DISPLAY_NAME}")
    set(MACOSX_BUNDLE_EXECUTABLE_NAME      "bagfile_parser_qt")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       "edu.auburn.gavlab.bagfileparserqt")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_ICON_FILE            "bagfile_parser_qt.icns")
    set(MACOSX_BUNDLE_COPYRIGHT            "${BPQ_VENDOR}")

    # The bundle and its inner executable stay named bagfile_parser_qt so the
    # documented CLI invocation works unchanged; CFBundleDisplayName carries the
    # pretty name for Finder and the menu bar.
    set_target_properties(bagfile_parser_qt PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/packaging/Info.plist.in")
endif()

# ---------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------
if(APPLE AND BPQ_MACOS_BUNDLE)
    if(BPQ_MACOS_DMG_LAYOUT)
        set(BPQ_BUNDLE_DEST ".")
    else()
        set(BPQ_BUNDLE_DEST "Applications")
    endif()
    install(TARGETS bagfile_parser_qt BUNDLE DESTINATION "${BPQ_BUNDLE_DEST}")

    # Pull Qt (and any other non-system dylib) into the bundle so it runs on a
    # machine without Qt installed.
    bpq_find_deploy_tool(macdeployqt BPQ_MACDEPLOYQT)
    if(BPQ_MACDEPLOYQT)
        install(CODE "
            message(STATUS \"Running macdeployqt on the installed bundle\")
            execute_process(
                COMMAND \"${BPQ_MACDEPLOYQT}\"
                        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${BPQ_BUNDLE_DEST}/bagfile_parser_qt.app\"
                RESULT_VARIABLE _rc)
            if(NOT _rc EQUAL 0)
                message(WARNING \"macdeployqt failed (\${_rc}); the .app may need Qt installed to run\")
            endif()
            # macdeployqt rewrites load commands inside the copied dylibs, which
            # breaks their signatures. arm64 refuses to run an incorrectly signed
            # binary, so ad-hoc re-sign the whole bundle afterwards.
            execute_process(
                COMMAND codesign --force --deep --sign -
                        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${BPQ_BUNDLE_DEST}/bagfile_parser_qt.app\"
                RESULT_VARIABLE _sign_rc)
            if(NOT _sign_rc EQUAL 0)
                message(WARNING \"ad-hoc codesign failed (\${_sign_rc}); the .app may not launch on Apple silicon\")
            endif()
        ")
    else()
        message(STATUS "macdeployqt not found - the .app will require a local Qt install")
    endif()

    # CLI entry point: <prefix>/bin/bagfile_parser_qt -> the bundle's executable.
    if(NOT BPQ_MACOS_DMG_LAYOUT)
    install(CODE "
        file(MAKE_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
        file(CREATE_LINK
            \"\${CMAKE_INSTALL_PREFIX}/${BPQ_BUNDLE_DEST}/bagfile_parser_qt.app/Contents/MacOS/bagfile_parser_qt\"
            \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/bagfile_parser_qt\"
            SYMBOLIC)
        list(APPEND CMAKE_INSTALL_MANIFEST_FILES
             \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/bagfile_parser_qt\")
    ")
    endif()

elseif(WIN32)
    install(TARGETS bagfile_parser_qt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Any DLLs vcpkg staged next to the freshly built .exe (only present when a
    # dynamic triplet is in use; a static triplet leaves this empty).
    install(CODE "
        file(GLOB _runtime_dlls \"$<TARGET_FILE_DIR:bagfile_parser_qt>/*.dll\")
        if(_runtime_dlls)
            file(INSTALL \${_runtime_dlls}
                 DESTINATION \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
        endif()
    ")

    # Qt DLLs, platform plugins and (for MinGW) the compiler runtime.
    bpq_find_deploy_tool(windeployqt BPQ_WINDEPLOYQT)
    # --compiler-runtime is what pulls in libstdc++/libgcc for a MinGW build. On
    # MSVC it is redundant: InstallRequiredSystemLibraries already copies the CRT
    # DLLs, while windeployqt additionally ships the whole 24 MB vc_redist.x64.exe
    # installer next to them.
    if(MINGW)
        set(BPQ_WINDEPLOYQT_RUNTIME_FLAG --compiler-runtime)
    else()
        set(BPQ_WINDEPLOYQT_RUNTIME_FLAG --no-compiler-runtime)
    endif()
    if(BPQ_WINDEPLOYQT)
        install(CODE "
            message(STATUS \"Running windeployqt on the installed executable\")
            execute_process(
                COMMAND \"${BPQ_WINDEPLOYQT}\"
                        --no-translations --no-system-d3d-compiler --no-opengl-sw
                        ${BPQ_WINDEPLOYQT_RUNTIME_FLAG}
                        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/bagfile_parser_qt.exe\"
                RESULT_VARIABLE _rc)
            if(NOT _rc EQUAL 0)
                message(FATAL_ERROR \"windeployqt failed (\${_rc}) - the install would not be runnable\")
            endif()
        ")
    else()
        message(WARNING "windeployqt not found - the install will require Qt on PATH")
    endif()

    include(InstallRequiredSystemLibraries)

    # Some Qt versions copy vc_redist.x64.exe regardless; the CRT DLLs beside it
    # make it dead weight (24 MB of it).
    install(CODE "
        file(GLOB _redist \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/vc_redist*.exe\")
        if(_redist)
            file(REMOVE \${_redist})
            message(STATUS \"Removed the bundled vc_redist installer (CRT DLLs are deployed directly)\")
        endif()
    ")

else()
    install(TARGETS bagfile_parser_qt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Freedesktop launcher + icons so it shows up in the applications menu.
    # The launcher is written at install time, not configure time, so that
    # `cmake --install --prefix ...` produces correct absolute paths.
    set(BPQ_DESKTOP_REL "${CMAKE_INSTALL_DATADIR}/applications/bagfile_parser_qt.desktop")
    set(BPQ_ICON_REL "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/bagfile_parser_qt.svg")
    install(CODE "
        set(_dest \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${BPQ_DESKTOP_REL}\")
        message(STATUS \"Installing: \${_dest}\")
        file(READ \"${CMAKE_SOURCE_DIR}/packaging/bagfile_parser_qt.desktop.in\" _tpl)
        string(REPLACE \"@BPQ_DESKTOP_EXEC@\"
               \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/bagfile_parser_qt\" _tpl \"\${_tpl}\")
        string(REPLACE \"@BPQ_DESKTOP_ICON@\"
               \"\${CMAKE_INSTALL_PREFIX}/${BPQ_ICON_REL}\" _tpl \"\${_tpl}\")
        file(WRITE \"\${_dest}\" \"\${_tpl}\")
        list(APPEND CMAKE_INSTALL_MANIFEST_FILES
             \"\${CMAKE_INSTALL_PREFIX}/${BPQ_DESKTOP_REL}\")
    ")

    install(FILES ${BPQ_ICON_DIR}/bagfile_parser_qt.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install(FILES ${BPQ_ICON_DIR}/bagfile_parser_qt.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
endif()

# ---------------------------------------------------------------------
# CPack: Windows installer (NSIS) + portable archives
# ---------------------------------------------------------------------
set(CPACK_PACKAGE_NAME              "${BPQ_DISPLAY_NAME}")
set(CPACK_PACKAGE_VENDOR            "${BPQ_VENDOR}")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL      "${BPQ_HOMEPAGE}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert ROS 2 MCAP bag files to MAT and CSV")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${BPQ_DISPLAY_NAME}")
set(CPACK_PACKAGE_EXECUTABLES       "bagfile_parser_qt" "${BPQ_DISPLAY_NAME}")
set(CPACK_STRIP_FILES TRUE)

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_FILE_NAME "bagfile_parser_qt-${PROJECT_VERSION}-windows-x64")

    # NSIS wants native (backslashed) paths for its icon settings.
    file(TO_NATIVE_PATH "${BPQ_ICON_DIR}/bagfile_parser_qt.ico" CPACK_NSIS_MUI_ICON)
    set(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_MUI_ICON}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\bagfile_parser_qt.exe")

    set(CPACK_NSIS_DISPLAY_NAME "${BPQ_DISPLAY_NAME}")
    set(CPACK_NSIS_PACKAGE_NAME "${BPQ_DISPLAY_NAME} ${PROJECT_VERSION}")
    set(CPACK_NSIS_HELP_LINK    "${BPQ_HOMEPAGE}")
    set(CPACK_NSIS_URL_INFO_ABOUT "${BPQ_HOMEPAGE}")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    # Offers a "add to PATH" checkbox, so the CLI works from any terminal.
    set(CPACK_NSIS_MODIFY_PATH ON)
    # CPack's own PATH code gives up once PATH passes 1024 characters.
    include(${CMAKE_CURRENT_SOURCE_DIR}/packaging/nsis_template.cmake)
    list(PREPEND CMAKE_MODULE_PATH "${BPQ_NSIS_TEMPLATE_DIR}")
    set(CPACK_CREATE_DESKTOP_LINKS "bagfile_parser_qt")

elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    # Deliberately version-free: GitHub's /releases/latest/download/<name> only
    # resolves an exact asset name, so a version in the filename would break
    # every documented download link on each new release. The version is on the
    # release page and in the app's Info.plist.
    set(CPACK_PACKAGE_FILE_NAME "bagfile_parser_qt-macos-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_DMG_VOLUME_NAME "${BPQ_DISPLAY_NAME} ${PROJECT_VERSION}")
    set(CPACK_DMG_FORMAT "UDZO")
else()
    set(CPACK_PACKAGE_FILE_NAME
        "bagfile_parser_qt-${PROJECT_VERSION}-linux-${CMAKE_SYSTEM_PROCESSOR}")

    # The .deb is the deliverable. Everything about it is derived from the build
    # machine, so a user on another architecture gets a correct package simply by
    # running scripts/package.sh themselves. On a system with no dpkg (Fedora,
    # Arch, ...) fall back to a tarball so there is still something to hand over.
    find_program(BPQ_DPKG dpkg)
    if(NOT BPQ_DPKG)
        message(STATUS "dpkg not found - packaging as a .tar.gz instead of a .deb")
        set(CPACK_GENERATOR "TGZ")
    else()
        set(CPACK_GENERATOR "DEB")
        # Debian package names must be lowercase and space-free.
        set(CPACK_DEBIAN_PACKAGE_NAME "bagfile-parser-qt")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${BPQ_VENDOR}")
        set(CPACK_DEBIAN_PACKAGE_SECTION "science")
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${BPQ_HOMEPAGE}")
        # Reads the ELF's NEEDED entries and turns them into versioned Depends:,
        # so apt pulls the right Qt/matio/lz4/zstd for the target release.
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        # Yields bagfile-parser-qt_<version>_<arch>.deb with the arch dpkg reports.
        set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    endif()
endif()

include(CPack)
