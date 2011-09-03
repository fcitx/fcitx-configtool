include_directories (${GTK2_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_CONFIG_INCLUDE_DIRS})
include_directories (${FCITX4_FCITX_INCLUDE_DIRS})
include_directories (${UNIQUE_INCLUDE_DIRS})

set( fcitx_config_gtk_sources
    sub_config_parser.c
    sub_config_widget.c
    config_widget.c
    configdesc.c
    keygrab.c
    main.c
    main_window.c
)

add_executable( fcitx-config-gtk ${fcitx_config_gtk_sources} )

install(TARGETS fcitx-config-gtk RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk ${GTK2_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX4_FCITX_CONFIG_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${FCITX4_FCITX_LIBRARIES})
target_link_libraries (fcitx-config-gtk ${UNIQUE_LIBRARIES})