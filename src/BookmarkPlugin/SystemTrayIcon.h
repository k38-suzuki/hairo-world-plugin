/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_SYSTEM_TRAY_ICON_H
#define CNOID_BOOKMARK_PLUGIN_SYSTEM_TRAY_ICON_H

#include <cnoid/Signal>
#include <QAction>
#include <QSystemTrayIcon>
#include "exportdecl.h"

namespace cnoid {

class Action;
class Menu;

class CNOID_EXPORT SystemTrayIcon : public QSystemTrayIcon
{
public:
    SystemTrayIcon(QObject* parent = nullptr);
    SystemTrayIcon(const QIcon& icon, QObject* parent = nullptr);
    virtual ~SystemTrayIcon();

    static bool isSystemTrayAvailable();

    Action* addAction();
    Action* addAction(const QString& text);
    Action* addAction(const QIcon& icon, const QString& text);

    QAction* addSection(const QString& text);
    QAction* addSection(const QIcon& icon, const QString& text);
    QAction* addSeparator();

    Menu* menu();

    SignalProxy<void()> sigContextRequested() { return sigContextRequested_; }
    SignalProxy<void()> sigDoubleClicked() { return sigDoubleClicked_; }
    SignalProxy<void()> sigTriggered() { return sigTriggered_; }
    SignalProxy<void()> sigMiddleClicked() { return sigMiddleClicked_; }

private:
    Menu* systrayMenu;

    Signal<void(void)> sigContextRequested_;
    Signal<void(void)> sigDoubleClicked_;
    Signal<void(void)> sigTriggered_;
    Signal<void(void)> sigMiddleClicked_;

    void initialize();
    void onActivated(QSystemTrayIcon::ActivationReason reason);
};

}

#endif // CNOID_BOOKMARK_PLUGIN_SYSTEM_TRAY_ICON_H