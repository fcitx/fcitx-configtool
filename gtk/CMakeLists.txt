PKG_CHECK_MODULES (DBUS_GLIB "dbus-glib-1" REQUIRED)
PKG_CHECK_MODULES (UNIQUE "unique-1.0")

include_directories (
    ${GTK2_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${GIO2_INCLUDE_DIRS}
    ${FCITX4_FCITX_INCLUDE_DIRS}
    ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
    ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
    ${DBUS_GLIB_INCLUDE_DIRS}
)

link_directories (
    ${GTK2_LIBRARY_DIRS}
    ${GLIB2_LIBRARY_DIRS}
    ${GIO2_LIBRARY_DIRS}
    ${FCITX4_FCITX_LIBRARY_DIRS}
    ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
    ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
    ${DBUS_GLIB_LIBRARY_DIRS}
)
if (UNIQUE_FOUND)
    include_directories (${UNIQUE_INCLUDE_DIRS})
    link_directories (${UNIQUE_LIBRARY_DIRS})
endif (UNIQUE_FOUND)

set( fcitx_config_gtk_sources
    sub_config_parser.c
    sub_config_widget.c
    config_widget.c
    configdesc.c
    keygrab.c
    main.c
    main_window.c
    im_widget.c
    im.c
    marshall.c
    gdm-languages.c
)

add_executable( fcitx-config-gtk ${fcitx_config_gtk_sources} )

install(TARGETS fcitx-config-gtk RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk
                       ${GTK2_LIBRARIES}
                       ${FCITX4_FCITX_UTILS_LIBRARIES}
                       ${FCITX4_FCITX_CONFIG_LIBRARIES}
                       ${FCITX4_FCITX_LIBRARIES}
                       ${GLIB2_LIBRARIES}
                       ${DBUS_GLIB_LIBRARIES}
)

if (UNIQUE_FOUND)
    target_link_libraries (fcitx-config-gtk ${UNIQUE_LIBRARIES})
endif (UNIQUE_FOUND)
