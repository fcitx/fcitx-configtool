PKG_CHECK_MODULES (GTK2 "gtk+-2.0" )
PKG_CHECK_MODULES(UNIQUE "unique-1.0" )
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

add_executable( fcitx-config-gtk ${fcitx_config_gtk_sources} )

install(TARGETS fcitx-config-gtk RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk ${GTK2_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX_CONFIG_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${UNIQUE_LIBRARIES})