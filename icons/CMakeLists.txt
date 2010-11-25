set(ICON_SIZE
    16
    22
    24
    32
    48
    128
    )

foreach(size ${ICON_SIZE})
    install(FILES ${size}/fcitx-configtool.png DESTINATION share/icons/hicolor/${size}x${size}/fcitx-configtool.png)
endforeach(size ${ICON_SIZE})
