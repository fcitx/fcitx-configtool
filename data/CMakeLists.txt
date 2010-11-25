add_custom_command(OUTPUT fcitx-configtool.desktop
                   COMMAND LC_ALL=C ${INTLTOOL_MERGE} -d -u ${CMAKE_CURRENT_SOURCE_DIR}/../po ${CMAKE_CURRENT_SOURCE_DIR}/fcitx-configtool.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/fcitx-configtool.desktop                 )

add_custom_target(desktopfile ALL DEPENDS fcitx-configtool.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fcitx-configtool.desktop DESTINATION share/applications/)