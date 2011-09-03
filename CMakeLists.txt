cmake_minimum_required (VERSION 2.6)

project(fcitx-configtool)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

include(FindPkgConfig)
FIND_PACKAGE(Gettext REQUIRED)
find_package(Fcitx REQUIRED)
PKG_CHECK_MODULES (GTK2 "gtk+-2.0" )
PKG_CHECK_MODULES(UNIQUE "unique-1.0" )
FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE intltool-merge)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined,--as-needed")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined,--as-needed")

set(datadir ${CMAKE_INSTALL_PREFIX}/share)
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
set(exec_prefix "${CMAKE_INSTALL_PREFIX}")

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(gtk)
add_subdirectory(po)
