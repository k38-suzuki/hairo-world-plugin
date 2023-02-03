/**
   \file
   \author Kenta Suzuki
*/

#include "SystemTrayManager.h"
#include <cnoid/Action>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include "gettext.h"

using namespace cnoid;

namespace {

SystemTrayIcon* createTrayIcon()
{
    SystemTrayIcon* systrayIcon = new SystemTrayIcon(QIcon(":/Base/icon/setup.svg"), MainWindow::instance());
    systrayIcon->show();

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
    } else {
        SystemTrayIcon* systrayIcon = SystemTrayManager::addIcon(QIcon(":/Base/icon/choreonoid.svg"));

        Action* exitAct = new Action;
        exitAct->setText(_("Exit"));
        exitAct->sigTriggered().connect([&](){ MainWindow::instance()->close(); });
        Menu* systrayMenu = new Menu(MainWindow::instance());
        systrayMenu->addAction(exitAct);

        systrayIcon->setContextMenu(systrayMenu);
        systrayIcon->setToolTip(_("Exit Choreonoid"));
        systrayIcon->show();
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