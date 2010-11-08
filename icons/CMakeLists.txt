set(ICON_SIZE
    16
    22
    24
    32
    48
    128
    )

foreach(size ${ICON_SIZE})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${size}/fcitx-config.png DESTINATION share/icons/hicolor/${size}x${size}/fcitx-config.png)
endforeach(size ${ICON_SIZE})
