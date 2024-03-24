/**
   @author Kenta Suzuki
*/

#include "SystemTrayManager.h"
#include <cnoid/Action>
#include <cnoid/ExtensionManager>
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

class SystemTrayManager::Impl
{
public:

    Impl();
};

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


SystemTrayManager::SystemTrayManager()
{
    impl = new Impl;
}


SystemTrayManager::Impl::Impl()
{

}


SystemTrayManager::~SystemTrayManager()
{
    delete impl;
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
