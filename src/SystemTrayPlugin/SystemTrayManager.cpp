/**
   \file
   \author Kenta Suzuki
*/

#include "SystemTrayManager.h"
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <QStyle>
#include "gettext.h"

using namespace cnoid;

namespace {

SystemTrayIcon* createTrayIcon()
{
    SystemTrayIcon* systrayIcon = new SystemTrayIcon(
        QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_MessageBoxQuestion)), MainWindow::instance());
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
    if(!SystemTrayIcon::isSystemTrayAvailable()) {
        MessageView::instance()->putln(_("I couldn't detect any system tray on this system"));
    }
}


SystemTrayIcon* SystemTrayManager::addIcon()
{
    return createTrayIcon();
}


SystemTrayIcon* SystemTrayManager::addIcon(const QIcon& icon)
{
    SystemTrayIcon* systrayIcon = createTrayIcon();
    systrayIcon->setIcon(icon);
    return systrayIcon;
}