/**
   \file
   \author Kenta Suzuki
*/

#include "StartupDialog.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ConnectionSet>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include "gettext.h"
#include "JProcess.h"

using namespace cnoid;
using namespace std;

namespace {

StartupDialog* dialog = nullptr;


struct ActionInfo {
    char* label;
    bool checkable;
};


ActionInfo actionInfo[] = {
    { _("Enable startup"), true },
    { _("Set a project"), false }
};

}

namespace cnoid {

class StartupDialogImpl
{
public:
    StartupDialogImpl(StartupDialog* self);
    StartupDialog* self;

    enum ActionId { ENABLE, CONFIG, NUM_ACTIONS };

    LineEdit* projectLine;
    JProcess* process;
    bool connected;
    Connection startConnection;
    Action* actions[NUM_ACTIONS];
    QSystemTrayIcon* trayIcon;

    void onSignalConnected(const bool& on);
    void onSelectButtonClicked();
    void onEnableActToggled(const bool& on);
    void onAccepted();
    void onRejected();

private Q_SLOTS:
    void onProcessStarted();
    void onProcessKilled();
};

}


StartupDialog::StartupDialog()
{
    impl = new StartupDialogImpl(this);
}


StartupDialogImpl::StartupDialogImpl(StartupDialog* self)
    : self(self)
{
    self->setWindowTitle(_("Startup Config"));

    Mapping* config = AppConfig::archive()->openMapping("Startup");
    string project = config->get("project", "");

    projectLine = new LineEdit();
    if(!project.empty()) {
        projectLine->setText(QString::fromStdString(project));
    }

    trayIcon = new QSystemTrayIcon();
    Menu* menu = new Menu();
    for(int i = 0; i < NUM_ACTIONS; ++i) {
        ActionInfo info = actionInfo[i];
        actions[i] = new Action(info.label, menu);
        Action* action = actions[i];
        action->setCheckable(info.checkable);
        menu->addAction(action);
    }

    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon(":/Base/icon/choreonoid.svg"));
    trayIcon->show();

    process = new JProcess();
    connected = false;

    PushButton* selectButton = new PushButton(_("Select"));

    QGridLayout* gbox = new QGridLayout();
    gbox->addWidget(new QLabel(_("Project")), 1, 0);
    gbox->addWidget(projectLine, 1, 1);
    gbox->addWidget(selectButton, 1, 2);

    QPushButton* okButton = new QPushButton(_("&Ok"));
    QPushButton* cancelButton = new QPushButton(_("&Cancel"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ResetRole);
    self->connect(buttonBox, SIGNAL(accepted()), self, SLOT(accept()));
    self->connect(buttonBox, SIGNAL(rejected()), self, SLOT(reject()));

    selectButton->sigClicked().connect([&](){ onSelectButtonClicked(); });
    actions[ENABLE]->sigToggled().connect([&](bool on){ onEnableActToggled(on); });
    actions[CONFIG]->sigTriggered().connect([&](){ dialog->show(); });

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);
}


StartupDialog::~StartupDialog()
{
    delete impl;
}


void StartupDialog::initializeClass(ExtensionManager* ext)
{
    if(!dialog) {
        dialog = ext->manage(new StartupDialog());
    }
}


void StartupDialogImpl::onSignalConnected(const bool& on)
{
    if(on && !connected) {
        startConnection = process->sigFinished().connect([&](int exitCode, QProcess::ExitStatus exitStatus){ onProcessStarted(); });
        connected = true;
    } else if(!on && connected) {
        startConnection.disconnect();
        connected = false;
    }
}


void StartupDialogImpl::onSelectButtonClicked()
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
        projectLine->setText(filename);
    }
}


void StartupDialogImpl::onEnableActToggled(const bool& on)
{
    MainWindow* mw = MainWindow::instance();
    actions[CONFIG]->setEnabled(!on);
    onSignalConnected(on);
    if(on) {
        mw->hide();
        onProcessStarted();
    } else {
        mw->show();
        onProcessKilled();
    }
}


void StartupDialogImpl::onProcessStarted()
{
    string command = "choreonoid ";
    string project = projectLine->text().toStdString();
    if(!project.empty()) {
        command += project;
    }
    onProcessKilled();
    process->start(command.c_str());
}


void StartupDialogImpl::onProcessKilled()
{
    if(process->state() != QProcess::NotRunning) {
        process->kill();
        process->waitForFinished(100);
    }
}


void StartupDialog::onAccepted()
{
    impl->onAccepted();
}


void StartupDialogImpl::onAccepted()
{
    Mapping* config = AppConfig::archive()->openMapping("Startup");
    string project = projectLine->text().toStdString();
    config->write("project", project);
}


void StartupDialog::onRejected()
{
    impl->onRejected();
}


void StartupDialogImpl::onRejected()
{

}
