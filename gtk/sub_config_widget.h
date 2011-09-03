#ifndef _FCITX_SUB_CONFIG_WIDGET
#define _FCITX_SUB_CONFIG_WIDGET

#include <gtk/gtk.h>
#include "sub_config_parser.h"

G_BEGIN_DECLS

#define FCITX_TYPE_SUB_CONFIG_WIDGET fcitx_sub_config_widget_get_type()

#define FCITX_SUB_CONFIG_WIDGET(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_SUB_CONFIG_WIDGET, FcitxSubConfigWidget))

#define FCITX_SUB_CONFIG_WIDGET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_SUB_CONFIG_WIDGET, FcitxSubConfigWidgetClass))

#define FCITX_IS_SUB_CONFIG_WIDGET(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_SUB_CONFIG_WIDGET))

#define FCITX_IS_SUB_CONFIG_WIDGET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_SUB_CONFIG_WIDGET))

#define FCITX_SUB_CONFIG_WIDGET_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_SUB_CONFIG_WIDGET, FcitxSubConfigWidgetClass))

typedef struct {
    GtkVBox parent;
    FcitxSubConfig* subconfig;
    GtkWidget* view;
} FcitxSubConfigWidget;

typedef struct {
    GtkVBoxClass parent_class;
} FcitxSubConfigWidgetClass;

GType fcitx_sub_config_widget_get_type (void);

FcitxSubConfigWidget* fcitx_sub_config_widget_new (FcitxSubConfig* subconfig);

G_END_DECLS

#endif /* _FCITX_SUB_CONFIG_WIDGET */
