#ifndef FCITXCONFIGPAGE_H
#define FCITXCONFIGPAGE_H

#include <QWidget>
#include <ui_FcitxConfigPage.h>

struct _ConfigFileDesc;
class QTabWidget;

class FcitxConfigPage : public QWidget
{
    Q_OBJECT
public:
    FcitxConfigPage(QWidget* parent, struct _ConfigFileDesc* cfdesc, const QString& name);
    virtual ~FcitxConfigPage();
private:
    struct _ConfigFileDesc* m_cfdesc;
    QString m_name;
    QTabWidget* m_tabWidget;
    Ui::FcitxConfigPage* m_ui;
};

#endif