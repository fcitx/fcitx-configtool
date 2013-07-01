/***************************************************************************
 *   Copyright (C) 2013~2014 by Lenky0401                                  *
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

#ifndef WIZARD_MAIN_WINDOW_H

#define WIZARD_MAIN_WINDOW_H

#define PAGE_INFO_NUM (6)

typedef struct
{
    GtkWidget *widget;
    gint index;
    const gchar *title;
    GtkAssistantPageType type;
    gboolean complete;
} PageInfo;

GtkWidget* create_assistant(void);

#endif
