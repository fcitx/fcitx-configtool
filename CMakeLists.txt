cmake_minimum_required (VERSION 2.6)

project(fcitx-configtool)

option(ENABLE_GTK2 "Enable GTK2 Version" Off)
option(ENABLE_GTK3 "Enable GTK3 Version" On)

find_package(PkgConfig REQUIRED)

if (ENABLE_GTK3)
PKG_CHECK_MODULES (FCITX_GCLIENT "fcitx-gclient >= 4.2.4" REQUIRED)
PKG_CHECK_MODULES (GTK3 "gtk+-3.0" REQUIRED)
endif (ENABLE_GTK3)

if (ENABLE_GTK2)
PKG_CHECK_MODULES (GTK2 "gtk+-2.0 >= 2.22" REQUIRED)
endif (ENABLE_GTK2)

PKG_CHECK_MODULES(ISO_CODES "iso-codes" REQUIRED)
_pkgconfig_invoke("iso-codes" ISO_CODES PREFIX "" "--variable=prefix")

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

include(FindPkgConfig)
FIND_PACKAGE(Gettext REQUIRED)
find_package(Fcitx 4.2.4 REQUIRED)
FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE intltool-merge)
add_definitions("-D_GNU_SOURCE")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" FCITX4_MAJOR_VERSION "${FCITX4_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" FCITX4_MINOR_VERSION "${FCITX4_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" FCITX4_PATCH_VERSION "${FCITX4_VERSION}")

set(datadir ${CMAKE_INSTALL_PREFIX}/share)
set(localedir ${CMAKE_INSTALL_PREFIX}/share/locale)
set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
set(liblocaledir ${CMAKE_INSTALL_PREFIX}/lib/locale)

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (ENABLE_GTK3)
    add_subdirectory(gtk3)
endif (ENABLE_GTK3)

if (ENABLE_GTK2)
    add_subdirectory(gtk)
endif (ENABLE_GTK2)
add_subdirectory(po)
