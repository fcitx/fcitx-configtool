include(FindPkgConfig)

find_package (GTK2 REQUIRED gtk)

message (STATUS "GTK2 includes:" ${GTK2_INCLUDE_DIRS})
message (STATUS "GTK2 libraries:" ${GTK2_LIBRARIES})

PKG_CHECK_MODULES(FCITX_CONFIG REQUIRED "fcitx-config >= 1.0" )
PKG_CHECK_MODULES(FCITX REQUIRED "fcitx" )
PKG_CHECK_MODULES(UNIQUE REQUIRED "unique-1.0" )

include_directories (${GTK2_INCLUDE_DIRS})
include_directories (${FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX_INCLUDE_DIRS})
include_directories (${UNIQUE_INCLUDE_DIRS})

set( fcitx_config_gtk_sources
    addon_stuff.c
    config_widget.c
    configdesc.c
    keygrab.c
    main.c
    main_window.c
    menu.c
    skin_stuff.c
    table_stuff.c
    )

set(exec_prefix "${CMAKE_INSTALL_PREFIX}/bin")
set(datadir "${CMAKE_INSTALL_PREFIX}/share")
set(localedir "${datadir}/locale")

add_definitions(-DDATADIR="${datadir}")
add_definitions(-DLOCALEDIR="${localedir}")
add_definitions(-DBINDIR="${exec_prefix}")

add_executable( fcitx-config-gtk ${fcitx_config_gtk_sources} )

install(TARGETS fcitx-config-gtk RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk ${GTK2_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX_CONFIG_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${UNIQUE_LIBRARIES})
