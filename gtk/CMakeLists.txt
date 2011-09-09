include_directories (
    ${GTK2_INCLUDE_DIRS}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
)

link_directories (
    ${GTK2_LIBRARY_DIRS}
    ${FCITX4_FCITX_LIBRARY_DIRS}
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
)
if (HAVE_UNIQUE)
    include_directories (${UNIQUE_INCLUDE_DIRS})
    link_directories (${UNIQUE_LIBRARY_DIRS})
endif (HAVE_UNIQUE)

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

target_link_libraries (fcitx-config-gtk
                       ${GTK2_LIBRARIES}
                       ${FCITX4_FCITX_UTILS_LIBRARIES}
                       ${FCITX4_FCITX_CONFIG_LIBRARIES}
                       ${FCITX4_FCITX_LIBRARIES}
)

if (HAVE_UNIQUE)
    target_link_libraries (fcitx-config-gtk ${UNIQUE_LIBRARIES})
endif (HAVE_UNIQUE)