file(GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.po) 

set(COPYRIGHT_HOLDER "Fcitx Development Team")
set(BUGS_ADDRESS "fcitx-dev@googlegroups.com")
set(POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/fcitx-config.pot)

include(srcfiles.cmake)

# Create PO Template file 'fcitx-config.pot'
add_custom_command(
    OUTPUT ${POT_FILE}
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
    ARGS -o ${POT_FILE} -s --omit-header 
    --msgid-bugs-address=${BUGS_ADDRESS} -c --keyword=_ --keyword=N_ 
    --copyright-holder=${COPYRIGHT_HOLDER} ${FCITX_CONFIG_SRCS}
    --directory=${CMAKE_SOURCE_DIR}
    # strip ${PROJECT_SOURCE_DIR} from the code pointing comments (#:)
    COMMAND sed
    ARGS -i "s,${PROJECT_SOURCE_DIR}/*,,g" ${POT_FILE}
    DEPENDS ${WORMUX_SRCS}
    )

# Update .po files and compile them to binary .gmo files
gettext_create_translations(${POT_FILE} ALL ${PO_FILES})

