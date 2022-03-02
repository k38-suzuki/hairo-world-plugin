/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/ToolBar>
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

class ConfigDialog : public Dialog
{
public:
    ConfigDialog();

    enum ButtonID { ADD, REMOVE, OPEN, CANCEL, OK, NUM_BUTTONS };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];
    Menu menu;

    void addItem(const string& filename);
    void removeItem();
    void onAddButtonClicked();
    void onOpenButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    bool openDialogToLoadProject(const string& filename);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};


class BookmarkManagerImpl
{
public:
    BookmarkManagerImpl(BookmarkManager* self, ExtensionManager* ext);
    virtual ~BookmarkManagerImpl();
    BookmarkManager* self;

    ConfigDialog* dialog;

    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


BookmarkManager::BookmarkManager(ExtensionManager* ext)
{
    impl = new BookmarkManagerImpl(this, ext);
}


BookmarkManagerImpl::BookmarkManagerImpl(BookmarkManager* self, ExtensionManager* ext)
    : self(self)
{
    dialog = new ConfigDialog();

    ToolBar* bar = new ToolBar(N_("BookmarkBar"));
    ext->addToolBar(bar);
    ToolButton* button = bar->addButton(QIcon(":/Bookmark/icon/bookmark.svg"));
    button->setToolTip(_("Show the bookmark manager"));
    button->sigClicked().connect([&](){ dialog->show(); });

    Mapping& config = *AppConfig::archive()->openMapping("BookMark");
    if(config.isValid()) {
        restore(config);
    }
}


BookmarkManager::~BookmarkManager()
{
    delete impl;
}


BookmarkManagerImpl::~BookmarkManagerImpl()
{
    store(*AppConfig::archive()->openMapping("BookMark"));
    delete dialog;
}


void BookmarkManager::initialize(ExtensionManager* ext)
{
    ext->manage(new BookmarkManager(ext));
}


void BookmarkManagerImpl::store(Mapping& archive)
{
    dialog->store(archive);
}


void BookmarkManagerImpl::restore(const Mapping& archive)
{
    dialog->restore(archive);
}


ConfigDialog::ConfigDialog()
{
    setWindowTitle(_("BookmarkManager"));

    treeWidget = new TreeWidget();
    treeWidget->setHeaderHidden(false);
    QStringList labels = { _("Project file") };
    treeWidget->setHeaderLabels(labels);

    static const char* blabels[] = { _("+"), _("-"), _("&Open"), _("&Cancel"), _("&Ok") };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton(blabels[i]);
        PushButton* button = buttons[i];
        if(i == OK) {
            button->setDefault(true);
        }
        buttonBox->addButton(button, info.role);
    }

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttons[ADD]->sigClicked().connect([&](){ onAddButtonClicked(); });
    buttons[REMOVE]->sigClicked().connect([&](){ removeItem(); });
    buttons[OPEN]->sigClicked().connect([&](){ onOpenButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(treeWidget);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Action* addAct = new Action();
    addAct->setText(_("Add"));
    menu.addAction(addAct);
    Action* removeAct = new Action();
    removeAct->setText(_("Remove"));
    menu.addAction(removeAct);
    Action* openAct = new Action();
    openAct->setText(_("Open"));
    menu.addAction(openAct);

    addAct->sigTriggered().connect([&](){ onAddButtonClicked(); });
    removeAct->sigTriggered().connect([&](){ removeItem(); });
    openAct->sigTriggered().connect([&](){ onOpenButtonClicked(); });
    connect(treeWidget, &TreeWidget::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });
}


void ConfigDialog::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void ConfigDialog::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void ConfigDialog::onAddButtonClicked()
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


void ConfigDialog::onOpenButtonClicked()
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


void ConfigDialog::onCustomContextMenuRequested(const QPoint& pos)
{
    menu.exec(treeWidget->mapToGlobal(pos));
}


bool ConfigDialog::openDialogToLoadProject(const string& filename)
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


void ConfigDialog::store(Mapping& archive)
{
    int size = treeWidget->topLevelItemCount();
    archive.write("num_bookmark", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string key = "bookmark_" + to_string(i);
            archive.write(key, filename);
        }
    }
}


void ConfigDialog::restore(const Mapping& archive)
{
    int size = archive.get("num_bookmark", 0);
    for(int i = 0; i < size; ++i) {
        string key = "bookmark_" + to_string(i);
        string filename = archive.get(key, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }
}
