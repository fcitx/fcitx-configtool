/***************************************************************************
 *   Copyright (C) 2013~2014 by Lenky0401                                  *
 *   Copyright (C) 2010~2012 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef WIZARD_IM_DIALOG_H
#define WIZARD_IM_DIALOG_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <fcitx-gclient/fcitxinputmethod.h>
#include "wizard_im_widget.h"

G_BEGIN_DECLS

#define FCITX_TYPE_WIZARD_IM_DIALOG fcitx_wizard_im_dialog_get_type()

#define FCITX_WIZARD_IM_DIALOG(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), FCITX_TYPE_WIZARD_IM_DIALOG, FcitxWizardImDialog))

#define FCITX_WIZARD_IM_DIALOG_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), FCITX_TYPE_WIZARD_IM_DIALOG, FcitxWizardImDialogClass))

#define FCITX_IS_WIZARD_IM_DIALOG(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FCITX_TYPE_WIZARD_IM_DIALOG))

#define FCITX_IS_WIZARD_IM_DIALOG_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), FCITX_TYPE_WIZARD_IM_DIALOG))

#define FCITX_WIZARD_IM_DIALOG_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), FCITX_TYPE_WIZARD_IM_DIALOG, FcitxWizardImDialogClass))

typedef struct _FcitxWizardImDialog FcitxWizardImDialog;
typedef struct _FcitxWizardImDialogClass FcitxWizardImDialogClass;

struct _FcitxWizardImDialog {
    GtkDialog parent;
    FcitxInputMethod* improxy;
    GtkListStore* availimstore;
    GtkWidget* availimview;
    GtkWidget* filterentry;
    GtkTreeModel* filtermodel;
    GtkWidget* onlycurlangcheckbox;
    GtkTreeModel* sortmodel;
    GtkBuilder* builder;
    GHashTable* langset;
    gchar* language;
    FcitxWizardImWidget* owner;
};

struct _FcitxWizardImDialogClass {
    GtkDialogClass parent_class;
};

GtkWidget*
fcitx_wizard_im_dialog_new(GtkWindow *parent, FcitxWizardImWidget* owner);


G_END_DECLS


#endif
