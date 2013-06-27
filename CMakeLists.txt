cmake_minimum_required(VERSION 2.6)

project(fcitx-configtool)

option(ENABLE_GTK2 "Enable GTK2 Version" Off)
option(ENABLE_GTK3 "Enable GTK3 Version" On)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(PkgConfig REQUIRED)

if(ENABLE_GTK3)
  pkg_check_modules(FCITX_GCLIENT "fcitx-gclient>=4.2.6" REQUIRED)
  pkg_check_modules(GTK3 "gtk+-3.0>=3.4" REQUIRED)
endif()

if(ENABLE_GTK2)
  pkg_check_modules(GTK2 "gtk+-2.0>=2.22" REQUIRED)
endif()

pkg_check_modules(ISO_CODES "iso-codes" REQUIRED)
_pkgconfig_invoke("iso-codes" ISO_CODES PREFIX "" "--variable=prefix")

find_package(Fcitx 4.2.8 REQUIRED)
_fcitx_add_uninstall_target()
_fcitx_translate_add_handler(
  "${PROJECT_SOURCE_DIR}/cmake/fcitx-extract-glade.sh" glade)

add_definitions("-D_GNU_SOURCE")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-local-typedefs -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-sign-compare -Wno-unused-local-typedefs -Wno-unused-parameter -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
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

add_subdirectory(po)
add_subdirectory(gtk3)
add_subdirectory(gtk)
