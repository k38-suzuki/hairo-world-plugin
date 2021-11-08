/**
   \file
   \author Kenta Suzuki
*/

#include "StartupManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/Process>
#include <fmt/format.h>
#include <QSystemTrayIcon>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

Mapping* config;
string project;

}


namespace cnoid {

class StartupManagerImpl
{
public:
    StartupManagerImpl(StartupManager* self);
    StartupManager* self;

    enum ActionId { ENABLE, CONFIG, NUM_ACTIONS };

    Process* process;
    Action* actions[NUM_ACTIONS];
    QSystemTrayIcon* trayIcon;

    void onConfigActTriggered();
    void onEnableActToggled(const bool& on);
    void onProcessStarted();
    void onProcessKilled();
    void onProcessFinished();
};

}


StartupManager::StartupManager()
{
    impl = new StartupManagerImpl(this);
}


StartupManagerImpl::StartupManagerImpl(StartupManager* self)
    : self(self)
{
    trayIcon = new QSystemTrayIcon();
    Menu* menu = new Menu();
    const char* labels[] = { _("Reboot mode"), _("Set a project") };
    for(int i = 0; i < NUM_ACTIONS; ++i) {
        actions[i] = new Action(labels[i], menu);
        Action* action = actions[i];
        action->setCheckable(i == 0 ? true : false);
        menu->addAction(action);
    }

    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon(":/Base/icon/choreonoid.svg"));
    trayIcon->show();

    process = new Process();

    actions[ENABLE]->sigToggled().connect([&](bool on){ onEnableActToggled(on); });
    actions[CONFIG]->sigTriggered().connect([&](){ onConfigActTriggered(); });
}


StartupManager::~StartupManager()
{
    delete impl;
}


void StartupManager::initializeClass(ExtensionManager* ext)
{
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        MessageView::instance()
                ->putln(fmt::format("I couldn't detect any system tray on this system."));
    }

    config = AppConfig::archive()->openMapping("startup");
    project = config->get("project", "");
    StartupManager* sm = ext->manage(new StartupManager());
}


void StartupManagerImpl::onConfigActTriggered()
{
    MainWindow* mw = MainWindow::instance();
    FileDialog dialog(mw);
    dialog.setWindowTitle(_("Select a Choreonoid project file"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("Project files (*.cnoid)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    if(dialog.exec()){
        QString filename = dialog.selectedFiles().front();
        project = filename.toStdString();
        config->write("project", project);
    }
}


void StartupManagerImpl::onEnableActToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    actions[CONFIG]->setEnabled(!on);
    if(on) {
        mw->hide();
        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                      [=](int exitCode, QProcess::ExitStatus exitStatus){ onProcessStarted(); });
        onProcessStarted();
    } else {
        mw->show();
        process->disconnect();
        onProcessKilled();
    }
}


void StartupManagerImpl::onProcessStarted()
{
    string command = "choreonoid ";
    if(!project.empty()) {
        command += project;
    }
    process->start(command.c_str());
}


void StartupManagerImpl::onProcessKilled()
{
    if(process->state() != QProcess::NotRunning) {
        process->kill();
        process->waitForFinished(100);
    }
}
