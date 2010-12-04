add_custom_command(OUTPUT fcitx-config-gtk.desktop
                   COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${CMAKE_CURRENT_SOURCE_DIR}/../po ${CMAKE_CURRENT_SOURCE_DIR}/fcitx-config-gtk.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config-gtk.desktop                 )

add_custom_target(desktopfile ALL DEPENDS fcitx-config-gtk.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config-gtk.desktop DESTINATION share/applications/)
