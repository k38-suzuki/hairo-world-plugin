/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkManagerDialog.h"
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/TreeWidget>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

struct ButtonInfo {
    QDialogButtonBox::ButtonRole role;
};


ButtonInfo buttonInfo[] = {
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::RejectRole },
    { QDialogButtonBox::AcceptRole }
};


}

namespace cnoid {

class BookmarkManagerDialogImpl
{
public:
    BookmarkManagerDialogImpl(BookmarkManagerDialog* self);
    virtual ~BookmarkManagerDialogImpl();
    BookmarkManagerDialog* self;

    enum ButtonId { ADD, REMOVE, OPEN, CANCEL, OK, NUM_BUTTONS };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];

    void addItem(const string& filename);
    void removeItem();
    void onAddButtonClicked();
    void onOpenButtonClicked();
    bool openDialogToLoadProject(const string& filename);
    void onAccepted();
    void onRejected();
};

}


BookmarkManagerDialog::BookmarkManagerDialog()
{
    impl = new BookmarkManagerDialogImpl(this);
}


BookmarkManagerDialogImpl::BookmarkManagerDialogImpl(BookmarkManagerDialog* self)
    : self(self)
{
    self->setWindowTitle(_("BookmarkManager"));

    treeWidget = new TreeWidget();
    treeWidget->setHeaderHidden(false);
    QStringList labels = { _("Project file") };
    treeWidget->setHeaderLabels(labels);

    Mapping* config = AppConfig::archive()->openMapping("Bookmark");
    int size = config->get("numBookmark", 0);
    for(int i = 0; i < size; ++i) {
        string key = "bookmark_" + to_string(i);
        string filename = config->get(key, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }

    const char* blabels[] = { _("+"), _("-"), _("&Open"), _("&Cancel"), _("&Ok") };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton(blabels[i]);
        PushButton* button = buttons[i];
        if(i == OK) {
            button->setDefault(true);
        }
        buttonBox->addButton(button, info.role);
    }

    self->connect(buttonBox, SIGNAL(accepted()), self, SLOT(accept()));
    self->connect(buttonBox, SIGNAL(rejected()), self, SLOT(reject()));
    buttons[ADD]->sigClicked().connect([&](){ onAddButtonClicked(); });
    buttons[REMOVE]->sigClicked().connect([&](){ removeItem(); });
    buttons[OPEN]->sigClicked().connect([&](){ onOpenButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(treeWidget);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);
}


BookmarkManagerDialog::~BookmarkManagerDialog()
{
    delete impl;
}


BookmarkManagerDialogImpl::~BookmarkManagerDialogImpl()
{
    int size = treeWidget->topLevelItemCount();
    Mapping* config = AppConfig::archive()->openMapping("Bookmark");
    config->write("numBookmark", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string key = "bookmark_" + to_string(i);
            config->write(key, filename);
        }
    }
}


void BookmarkManagerDialogImpl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void BookmarkManagerDialogImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void BookmarkManagerDialogImpl::onAddButtonClicked()
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
        int numFiles = dialog.selectedFiles().size();
        for(int i = 0; i < numFiles; ++i) {
            QString filename = dialog.selectedFiles()[i];
            addItem(filename.toStdString());
        }
    }
}


void BookmarkManagerDialogImpl::onOpenButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        bool on = openDialogToLoadProject(filename);
        if(!on) {
            return;
        }
    }
}


bool BookmarkManagerDialogImpl::openDialogToLoadProject(const string& filename)
{
    bool result = true;
    MainWindow* mw = MainWindow::instance();
    MessageView* mv = MessageView::instance();
    ProjectManager* pm = ProjectManager::instance();
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
    }
    return result;
}


void BookmarkManagerDialog::onAccepted()
{
    impl->onAccepted();
}


void BookmarkManagerDialogImpl::onAccepted()
{

}


void BookmarkManagerDialog::onRejected()
{
    impl->onRejected();
}


void BookmarkManagerDialogImpl::onRejected()
{

}
