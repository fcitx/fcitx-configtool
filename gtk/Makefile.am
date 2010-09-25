bin_PROGRAMS=fcitx-config-gtk
fcitx_config_gtk_SOURCES= main.c \
						  main_window.c main_window.h \
						  config_widget.c config_widget.h \
						  skin_stuff.c skin_stuff.h \
						  table_stuff.c table_stuff.h \
						  addon_stuff.c addon_stuff.h \
						  configdesc.c configdesc.h \
						  menu.c menu.h \
						  utarray.h uthash.h

DEFS = -DPKGDATADIR=\"$(pkgdatadir)\" -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\" -DBINDIR=\"$(bindir)\"

INCLUDES = \
	@GTK2_CFLAGS@ \
	@FCITX_CONFIG_CFLAGS@

LDADD = \
	@GTK2_LIBS@ \
	@FCITX_CONFIG_LIBS@
