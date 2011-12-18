PKG_CHECK_MODULES (GTK3 "gtk+-3.0" REQUIRED)
PKG_CHECK_MODULES(UNIQUE3 "unique-3.0")

include_directories (
    ${GTK3_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
    ../gtk/
)

link_directories (
    ${GTK3_LIBRARY_DIRS}
    ${GIO2_LIBRARY_DIRS}
    ${FCITX4_FCITX_LIBRARY_DIRS}
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
)
if (UNIQUE3_FOUND)
    include_directories (${UNIQUE3_INCLUDE_DIRS})
    link_directories (${UNIQUE3_LIBRARY_DIRS})
endif (UNIQUE3_FOUND)

set( fcitx_config_gtk3_sources
    ../gtk/sub_config_parser.c
    ../gtk/sub_config_widget.c
    ../gtk/config_widget.c
    ../gtk/configdesc.c
    ../gtk/keygrab.c
    ../gtk/main.c
    ../gtk/main_window.c
    ../gtk/im_widget.c
    ../gtk/im.c
    ../gtk/gdm-languages.c
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

if (UNIQUE3_FOUND)
    target_link_libraries (fcitx-config-gtk3 ${UNIQUE3_LIBRARIES})
endif (UNIQUE3_FOUND)

