#include "FcitxConfigPage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <fcitx-config/fcitx-config.h>
#include <QLabel>
#include <QSpinBox>
#include <KColorButton>
#include <QCheckBox>
#include <KFontComboBox>
#include <KComboBox>
#include <KKeySequenceWidget>
#include <KLineEdit>
#include <libintl.h>
#include "config.h"
#include <QScrollArea>

FcitxConfigPage::FcitxConfigPage(QWidget* parent, _ConfigFileDesc* cfdesc, const QString& name):
    QWidget(parent), m_cfdesc(cfdesc), m_name(name), m_ui(new Ui::FcitxConfigPage)
{
    m_ui->setupUi(this);
    
    bindtextdomain(m_cfdesc->domain, LOCALEDIR);
    bind_textdomain_codeset(m_cfdesc->domain, "UTF-8");
    
    ConfigGroupDesc *cgdesc = NULL;
    ConfigOptionDesc *codesc = NULL;
    for(cgdesc = cfdesc->groupsDesc;
        cgdesc != NULL;
        cgdesc = (ConfigGroupDesc*)cgdesc->hh.next)
    {
        codesc = cgdesc->optionsDesc;
        if (codesc == NULL)
            continue;

        QWidget* main = new QWidget(this);
        QVBoxLayout* mainLayout = new QVBoxLayout;
        main->setLayout(mainLayout);
        
        QScrollArea *scrollarea = new QScrollArea;
        QVBoxLayout* scrollLayout = new QVBoxLayout;
        scrollarea->setLayout(scrollLayout);
        mainLayout->addWidget(scrollarea);
        
        QWidget* form = new QWidget;
        QFormLayout* formLayout = new QFormLayout;
        form->setLayout(formLayout);
        form->setVisible(true);
        
        int i = 0;
        for ( ; codesc != NULL;
            codesc = (ConfigOptionDesc*)codesc->hh.next, i++)
        {
            QString s;
            if (codesc->desc && strlen(codesc->desc) != 0)
                s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->desc));
            else
                s = QString::fromUtf8(dgettext(m_cfdesc->domain, codesc->optionName));
            
            QWidget *inputWidget = NULL;
            void *argument = NULL;

            switch(codesc->type)
            {
                case T_Integer:
                    inputWidget = new QSpinBox(this);
                    argument = inputWidget;
                    break;
                case T_Color:
                    inputWidget = new KColorButton(this);
                    argument = inputWidget;
                    break;
                case T_Boolean:
                    inputWidget = new QCheckBox(this);
                    argument = inputWidget;
                    break;
                case T_Font:
                    {
                        inputWidget = new KFontComboBox(this);
                        argument = inputWidget;
                    }
                    break;
                case T_Enum:
                    {
                        int i;
                        ConfigEnum *e = &codesc->configEnum;
                        KComboBox* combobox = new KComboBox(this);
                        inputWidget = combobox;
                        for (i = 0; i < e->enumCount; i ++)
                        {
                            combobox->addItem(QString::fromUtf8(dgettext(m_cfdesc->domain, e->enumDesc[i])));
                        }
                        argument = inputWidget;
                    }
                    break;
                case T_Hotkey:
                    {
                        inputWidget = new KKeySequenceWidget(this);
                        argument = inputWidget;
                    }
                    break;
                case T_File:
                case T_Char:
                case T_String:
                case T_I18NString:
                    inputWidget = new KLineEdit(this);
                    argument = inputWidget;
                    break;
            }
            if (inputWidget)
                formLayout->addRow(new QLabel(s, this), inputWidget);
        }
        scrollLayout->addWidget(form);
        
        m_ui->tabWidget->addTab(main, QString::fromUtf8(dgettext(m_cfdesc->domain, cgdesc->groupName)));
    }
}

FcitxConfigPage::~FcitxConfigPage()
{

}

#include "moc_FcitxConfigPage.cpp"