#ifndef __FCITX_ADDON_SELECTOR_H__
#define __FCITX_ADDON_SELECTOR_H__

#include <QWidget>

struct FcitxAddon;
class FcitxAddonSelector : public QWidget
{
    Q_OBJECT
public:
    FcitxAddonSelector(QWidget* parent);
    void load();
    void save();
    void addAddon(FcitxAddon* fcitxAddon);
 
Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray& componentName);
    
private:
    class Private;
    Private* d;
};

#endif