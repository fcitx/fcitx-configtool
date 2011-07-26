find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
 
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
else(NOT GETTEXT_MSGFMT_EXECUTABLE)

    set(catalogname kcm_fcitx)
    add_custom_target(translations ALL)
    
    file(GLOB PO_FILES
        *.po
    )

    foreach(_poFile ${PO_FILES})
        get_filename_component(_lang ${_poFile} NAME_WE)
        set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
        add_custom_command(TARGET translations
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${_poFile})

        install(FILES ${_gmoFile}
            DESTINATION share/locale/${_lang}/LC_MESSAGES/
            RENAME ${catalogname}.mo)
    endforeach(_poFile ${PO_FILES})
 
endif(NOT GETTEXT_MSGFMT_EXECUTABLE)

