/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"
#include "ui_Module.h"
#include "Module.h"

#include <QtCore/QFile>
#include <QPaintEngine>
#include <QDir>
#include <QDebug>

#include <KAboutData>
#include <KPluginFactory>
#include <KStandardDirs>
#include <kdebug.h>
#include <fcitx-utils/utils.h>
#include <fcitx/addon.h>
#include <fcitx-config/xdg.h>
#include "FcitxAddonSelector.h"
#include <libintl.h>

K_PLUGIN_FACTORY_DECLARATION(KcmFcitxFactory);

const UT_icd addonicd= {sizeof(FcitxAddon), 0, 0, 0};

Module::Module(QWidget *parent, const QVariantList &args) :
    KCModule(KcmFcitxFactory::componentData(), parent, args),
    ui(new Ui::Module),
    addonSelector(0)
{
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");

    KAboutData *about = new KAboutData("kcm_fcitx", 0,
                                       ki18n("Fcitx Configuration Module"),
                                       VERSION_STRING_FULL,
                                       ki18n("Configure Fcitx"),
                                       KAboutData::License_GPL_V2,
                                       ki18n("Copyright 2011 Xuetian Weng"),
                                       KLocalizedString(), QByteArray(),
                                       "wengxt@gmail.com");

    about->addAuthor(ki18n("Xuetian Weng"), ki18n("CS Slayer"), "wengxt@gmail.com");
    setAboutData(about);
    
    ui->setupUi(this);
    addonSelector = ui->addonSelector;
}

Module::~Module()
{
}

void Module::load()
{
    kDebug() << "Load Addon Info";
    utarray_new(addons, &addonicd);
    LoadAddonInfo(addons);
    
    for (FcitxAddon* addon = (FcitxAddon *) utarray_front(addons);
         addon != NULL;
         addon = (FcitxAddon *) utarray_next(addons, addon))
    {
        this->addonSelector->addAddon(addon);
    }

}

void Module::save()
{
}

void Module::defaults()
{
}

void Module::changed()
{
    KCModule::changed();
}
