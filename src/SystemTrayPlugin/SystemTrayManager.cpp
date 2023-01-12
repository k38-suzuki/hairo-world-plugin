/**
   \file
   \author Kenta Suzuki
*/

#include "SystemTrayManager.h"
#include <cnoid/MainWindow>
#include <cnoid/MessageView>
#include <QSystemTrayIcon>
#include "gettext.h"

using namespace cnoid;

namespace {

QSystemTrayIcon* createTrayIcon()
{
    QSystemTrayIcon* systrayIcon = new QSystemTrayIcon(MainWindow::instance());
    systrayIcon->show();

    Menu* systrayMenu = new Menu(MainWindow::instance());
    systrayIcon->setContextMenu(systrayMenu);
    return systrayIcon;
}

}

namespace cnoid {

class SystemTrayManagerImpl
{
public:
    SystemTrayManagerImpl(SystemTrayManager* self);
    SystemTrayManager* self;
};

}


SystemTrayManager::SystemTrayManager()
{
    impl = new SystemTrayManagerImpl(this);
}


SystemTrayManagerImpl::SystemTrayManagerImpl(SystemTrayManager* self)
    : self(self)
{

}


SystemTrayManager::~SystemTrayManager()
{
    delete impl;
}


void SystemTrayManager::initializeClass(ExtensionManager* ext)
{
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        MessageView::instance()->putln(_("I couldn't detect any system tray on this system"));
    }
}


Menu* SystemTrayManager::addMenu()
{
    QSystemTrayIcon* systrayIcon = createTrayIcon();
    return dynamic_cast<Menu*>(systrayIcon->contextMenu());
}


Menu* SystemTrayManager::addMenu(const QIcon& icon)
{
    QSystemTrayIcon* systrayIcon = createTrayIcon();
    systrayIcon->setIcon(icon);
    return dynamic_cast<Menu*>(systrayIcon->contextMenu());
}