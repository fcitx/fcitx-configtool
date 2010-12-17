cmake_minimum_required (VERSION 2.6)

project(fcitx-configtool)

FIND_PACKAGE(Gettext REQUIRED)
FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE intltool-merge)

add_subdirectory(gtk)
add_subdirectory(po)
