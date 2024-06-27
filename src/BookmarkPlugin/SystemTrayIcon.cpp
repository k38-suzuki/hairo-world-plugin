/**
   @author Kenta Suzuki
*/

#include "SystemTrayIcon.h"
#include <cnoid/Action>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include "gettext.h"

using namespace cnoid;

SystemTrayIcon::SystemTrayIcon(QObject* parent)
    : QSystemTrayIcon(QIcon(":/Base/icon/setup.svg"), parent)
{
    initialize();
}


SystemTrayIcon::SystemTrayIcon(const QIcon& icon, QObject* parent)
    : QSystemTrayIcon(icon, parent)
{
    initialize();
}


SystemTrayIcon::~SystemTrayIcon()
{

}


Action* SystemTrayIcon::addAction(const QString& text)
{
    Action* action = new Action(text, MainWindow::instance());
    systrayMenu->addAction(action);
    return action;
}


Action* SystemTrayIcon::addAction(const QIcon& icon, const QString& text)
{
    Action* action = new Action(icon, text, MainWindow::instance());
    systrayMenu->addAction(action);
    return action;
}



void SystemTrayIcon::initialize()
{
    systrayMenu = new Menu(MainWindow::instance());
    this->setContextMenu(systrayMenu);
    // systrayIcon->setToolTip("");
    this->show();

    connect(this, QOverload<QSystemTrayIcon::ActivationReason>::of(&QSystemTrayIcon::activated),
        [=](QSystemTrayIcon::ActivationReason reason){ onActivated(reason); });
}


void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
        case QSystemTrayIcon::Context:
            sigContextRequested_();
            break;
        case QSystemTrayIcon::DoubleClick:
            sigDoubleClicked_();
            break;
        case QSystemTrayIcon::Trigger:
            sigTriggered_();
            break;
        case QSystemTrayIcon::MiddleClick:
            sigMiddleClicked_();
            break;
    }
}


namespace {

struct Registration {
    Registration() {
        if(!SystemTrayIcon::isSystemTrayAvailable()) {
            MessageView::instance()->putln(_("I couldn't detect any system tray on this system"));
        } else {
            SystemTrayIcon* systrayIcon = new SystemTrayIcon(QIcon(":/Base/icon/choreonoid.svg"));
            systrayIcon->addAction(_("Exit"))->sigTriggered().connect(
                [&](){ MainWindow::instance()->close(); });
        }
    }
} registration;

}