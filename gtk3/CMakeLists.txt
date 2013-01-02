set(fcitx_config_gtk3_sources
  sub_config_parser.c
  sub_config_widget.c
  config_widget.c
  configdesc.c
  keygrab.c
  main.c
  main_window.c
  im_widget.c
  ui_widget.c
  gdm-languages.c
  im_dialog.c
  common.c
  im_config_dialog.c
  dummy_config.c
  )
set(fcitx_config_gtk3_headers
  common.h
  configdesc.h
  config_widget.h
  dummy_config.h
  gdm-languages.h
  im_config_dialog.h
  im_dialog.h
  ui_widget.h
  im_widget.h
  keygrab.h
  locarchive.h
  main_window.h
  sub_config_parser.h
  sub_config_widget.h
  )

fcitx_translate_add_sources(${fcitx_config_gtk3_sources}
  ${fcitx_config_gtk3_headers})

if(NOT ENABLE_GTK3)
  return()
endif()

include_directories (
  ${GTK3_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
  ${GIO2_INCLUDE_DIRS}
  ${FCITX4_FCITX_INCLUDE_DIRS}
  ${FCITX4_FCITX_UTILS_INCLUDE_DIRS}
  ${FCITX4_FCITX_CONFIG_INCLUDE_DIRS}
  ${FCITX_GCLIENT_INCLUDE_DIRS}
  )

link_directories (
  ${GTK3_LIBRARY_DIRS}
  ${GLIB2_LIBRARY_DIRS}
  ${GIO2_LIBRARY_DIRS}
  ${FCITX4_FCITX_LIBRARY_DIRS}
  ${FCITX4_FCITX_UTILS_LIBRARY_DIRS}
  ${FCITX4_FCITX_CONFIG_LIBRARY_DIRS}
  ${FCITX_GCLIENT_LIBRARY_DIRS}
  )

add_executable(fcitx-config-gtk3 ${fcitx_config_gtk3_sources})

install(TARGETS fcitx-config-gtk3 RUNTIME DESTINATION bin)

target_link_libraries (fcitx-config-gtk3
  ${GTK3_LIBRARIES}
  ${FCITX4_FCITX_UTILS_LIBRARIES}
  ${FCITX4_FCITX_CONFIG_LIBRARIES}
  ${FCITX4_FCITX_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${GIO2_LIBRARIES}
  ${FCITX_GCLIENT_LIBRARIES}
  )
