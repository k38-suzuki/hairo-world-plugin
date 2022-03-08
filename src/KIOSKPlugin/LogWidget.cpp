/**
   \file
   \author Kenta Suzuki
*/

#include "LogWidget.h"
#include <cnoid/Action>
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/TimeBar>
#include <cnoid/TreeWidget>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

struct ButtonInfo {
    int row;
    int column;
    int rowSpan;
    int columnSpan;
    const char* icon;
};

ButtonInfo buttonInfo[] = {
    { 0, 0, 1, 1,                     "" },
    { 0, 1, 1, 1,                     "" },
    { 0, 2, 1, 1, ":/Base/icon/play.svg" }
};

}


namespace cnoid {

class LogWidgetImpl
{
public:
    LogWidgetImpl(LogWidget* self);
    LogWidget* self;

    enum ButtonID { REMOVE, LOCK, START, NUM_BUTTONS };
    enum InfoID { LOG, FILE, NUM_INFO };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];
    Menu menu;

    void addItem(const string& filename, const string& text);
    void removeItem();
    void onLockButtonToggled(const bool& on);
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    bool onOpenButtonClicked(const string& filename);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


LogWidget::LogWidget()
{
    impl = new LogWidgetImpl(this);
}


LogWidgetImpl::LogWidgetImpl(LogWidget* self)
    : self(self)
{
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(false);

    QStringList header_labels = { _("Log"), _("File") };
    treeWidget->setHeaderLabels(header_labels);

    static const char* labels[] = { _("-"), _("Lock") };

    MainWindow* mw = MainWindow::instance();
    QHBoxLayout* hbox = new QHBoxLayout;
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton;
        PushButton* button = buttons[i];
        ButtonInfo info = buttonInfo[i];
        if(i < START) {
            button->setText(labels[i]);
        } else if(info.icon != "") {
            button->setIconSize(mw->iconSize());
            button->setIcon(QIcon(info.icon));
        }
        hbox->addWidget(button);
        if(i == LOCK) {
            hbox->addStretch();
        }
    }

    buttons[LOCK]->setCheckable(true);

    buttons[REMOVE]->sigClicked().connect([&](){ removeItem(); });
    buttons[LOCK]->sigToggled().connect([&](bool on){ onLockButtonToggled(on); });
    buttons[START]->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(hbox);
    vbox->addWidget(treeWidget);
    self->setLayout(vbox);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Action* removeAct = new Action();
    removeAct->setText(_("Remove"));
    menu.addAction(removeAct);
    Action* openAct = new Action();
    openAct->setText(_("Open"));
    menu.addAction(openAct);

    removeAct->sigTriggered().connect([&](){ removeItem(); });
    openAct->sigTriggered().connect([&](){ onStartButtonClicked(); });
    self->connect(treeWidget, &TreeWidget::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });
}


LogWidget::~LogWidget()
{
    delete impl;
}


void LogWidget::addItem(const string& filename, const string& text)
{
    impl->addItem(filename, text);
}


void LogWidgetImpl::addItem(const string& filename, const string& text)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(FILE, filename.c_str());
    treeWidget->setCurrentItem(item);
    LineEdit* logLine = new LineEdit;
    logLine->setPlaceholderText(_("Log"));
    if(!text.empty()) {
        logLine->setText(text.c_str());
    }
    treeWidget->setItemWidget(item, LOG, logLine);
}


void LogWidgetImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void LogWidgetImpl::onLockButtonToggled(const bool& on)
{
    buttons[REMOVE]->setEnabled(!on);

    int size = treeWidget->topLevelItemCount();
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            LineEdit* logLine = dynamic_cast<LineEdit*>(treeWidget->itemWidget(item, LOG));
            if(logLine) {
                logLine->setEnabled(!on);
            }
        }
    }
}


void LogWidgetImpl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(FILE).toStdString();
        bool on = onOpenButtonClicked(filename);
        if(!on) {
            return;
        }
    }
}


void LogWidgetImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    menu.exec(treeWidget->mapToGlobal(pos));
}


bool LogWidgetImpl::onOpenButtonClicked(const string& filename)
{
    bool result = true;
    MainWindow* mw = MainWindow::instance();
    MessageView* mv = MessageView::instance();
    ProjectManager* pm = ProjectManager::instance();
    TimeBar* tb = TimeBar::instance();
    int numItems = RootItem::instance()->countDescendantItems();
    string currentProjectFile = pm->currentProjectFile();
    QString projectFile = QString::fromStdString(currentProjectFile);
    QFileInfo info(projectFile);
    string currentProjectName = info.baseName().toStdString();
    if(numItems > 0){
        QString title = _("Warning");
        QString message;
        QMessageBox::StandardButton clicked;
        if(currentProjectFile.empty()){
            if(numItems == 1){
                message = _("A project item exists. "
                            "Do you want to save it as a project file before loading a new project?");
            } else {
                message = _("Project items exist. "
                            "Do you want to save them as a project file before loading a new project?");
            }
            clicked = QMessageBox::warning(
                mw, title, message, QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Ignore);
        } else {
            message = _("Project \"%1\" exists. Do you want to save it before loading a new project?");
            clicked = QMessageBox::warning(
                mw, title, message.arg(currentProjectName.c_str()),
                QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Ignore);
        }
        if(clicked == QMessageBox::Cancel){
            result = false;
        }
        if(clicked == QMessageBox::Save){
            pm->overwriteCurrentProject();
        }
    }

    if(result) {
        pm->clearProject();
        mv->flush();
        pm->loadProject(filename);
        tb->stopPlayback(true);
        tb->startPlayback(0.0);
    }
    return result;
}


void LogWidget::store(Mapping& archive)
{
    impl->store(archive);
}


void LogWidgetImpl::store(Mapping& archive)
{
    int size = treeWidget->topLevelItemCount();
    archive.write("num_logs", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            LineEdit* logLine = dynamic_cast<LineEdit*>(treeWidget->itemWidget(item, LOG));
            if(logLine) {
                string log = logLine->text().toStdString();
                string logKey = "log" + to_string(i);
                archive.write(logKey, log);
            }
            string filename = item->text(FILE).toStdString();
            string fileKey = "log_file" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
    archive.write("log_lock", buttons[LOCK]->isChecked());
}


void LogWidget::restore(const Mapping& archive)
{
    impl->restore(archive);
}


void LogWidgetImpl::restore(const Mapping& archive)
{
    int size = archive.get("num_logs", 0);
    for(int i = 0; i < size; ++i) {
        string logKey = "log" + to_string(i);
        string log = archive.get(logKey, "");
        string fileKey = "log_file" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename, log);
        }
    }
    buttons[LOCK]->setChecked(archive.get("log_lock", false));
}
