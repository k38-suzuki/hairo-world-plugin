/**
   @author Kenta Suzuki
*/

#include "SystemTrayIcon.h"

using namespace cnoid;

SystemTrayIcon::SystemTrayIcon(QObject* parent)
    : QSystemTrayIcon(parent)
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


void SystemTrayIcon::initialize()
{
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
