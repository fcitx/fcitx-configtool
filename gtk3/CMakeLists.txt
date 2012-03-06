PKG_CHECK_MODULES (GTK3 "gtk+-3.0" REQUIRED)

include_directories (
    ${GTK3_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
)

link_directories (
    ${GTK3_LIBRARY_DIRS}
    ${GIO2_LIBRARY_DIRS}
    ${FCITX4_FCITX_LIBRARY_DIRS}
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
)

set( fcitx_config_gtk3_sources
    sub_config_parser.c
    sub_config_widget.c
    config_widget.c
    configdesc.c
    keygrab.c
    main.c
    main_window.c
    im_widget.c
    im.c
    gdm-languages.c
)

add_executable( fcitx-config-gtk3 ${fcitx_config_gtk3_sources} )

install(TARGETS fcitx-config-gtk3 RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk3
                       ${GTK3_LIBRARIES}
                       ${FCITX4_FCITX_UTILS_LIBRARIES}
                       ${FCITX4_FCITX_CONFIG_LIBRARIES}
                       ${FCITX4_FCITX_LIBRARIES}
                       ${GIO2_LIBRARIES}
)

