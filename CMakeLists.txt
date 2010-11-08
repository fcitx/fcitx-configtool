cmake_minimum_required (VERSION 2.6)

project(fcitx-config)

FIND_PACKAGE(Gettext REQUIRED)
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

add_subdirectory(gtk)
add_subdirectory(data)
add_subdirectory(icons)
add_subdirectory(po)
