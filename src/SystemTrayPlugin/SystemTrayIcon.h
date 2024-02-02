/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SYSTEM_TRAY_PLUGIN_SYSTEM_TRAY_ICON_H
#define CNOID_SYSTEM_TRAY_PLUGIN_SYSTEM_TRAY_ICON_H

#include <cnoid/Signal>
#include <QSystemTrayIcon>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT SystemTrayIcon : public QSystemTrayIcon
{
public:
    SystemTrayIcon(QObject* parent = 0);
    SystemTrayIcon(const QIcon& icon, QObject* parent = 0);
    virtual ~SystemTrayIcon();

    SignalProxy<void()> sigContextRequested() { return sigContextRequested_; }
    SignalProxy<void()> sigDoubleClicked() { return sigDoubleClicked_; }
    SignalProxy<void()> sigTriggered() { return sigTriggered_; }
    SignalProxy<void()> sigMiddleClicked() { return sigMiddleClicked_; }

private:
    Signal<void(void)> sigContextRequested_;
    Signal<void(void)> sigDoubleClicked_;
    Signal<void(void)> sigTriggered_;
    Signal<void(void)> sigMiddleClicked_;

    void initialize();
    void onActivated(QSystemTrayIcon::ActivationReason reason);
};

}

#endif
