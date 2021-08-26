/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkManagerView.h"
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/TreeWidget>
#include <cnoid/ViewManager>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BookmarkManagerViewImpl
{
public:
  BookmarkManagerViewImpl(BookmarkManagerView* self);
  BookmarkManagerView* self;

  TreeWidget* treeWidget;
  MessageView* mv;
  ProjectManager* pm;

  enum ItemColumn {
      NO,
      FILE,
      NUM_COLUMN
  };

  void addItem(const string& name);
  void numberingItems();
  void onAddButtonClicked();
  void onRemoveButtonClicked();
  void onClearButtonClicked();
  void onOpenButtonClicked();

  bool storeState(Archive& archive);
  bool restoreState(const Archive& archive);
  void onActivated();
  void onDeactivated();
};

}


BookmarkManagerView::BookmarkManagerView()
{
    impl = new BookmarkManagerViewImpl(this);
}


BookmarkManagerViewImpl::BookmarkManagerViewImpl(BookmarkManagerView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::BOTTOM);
    treeWidget = new TreeWidget();
    treeWidget->setHeaderHidden(false);
    QStringList labels = { "No.", "Project file"  };
    treeWidget->setHeaderLabels(labels);

    mv = MessageView::instance();
    pm = ProjectManager::instance();

    PushButton* addButton = new PushButton(_("+"));
    PushButton* removeButton = new PushButton(_("-"));
    PushButton* clearButton = new PushButton(_("Clear"));
    PushButton* openButton = new PushButton(_("Open"));
    QHBoxLayout* hbox = new QHBoxLayout();

    hbox->addWidget(addButton);
    hbox->addWidget(removeButton);
    hbox->addWidget(clearButton);
    hbox->addStretch();
    hbox->addWidget(openButton);

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(hbox);
    vbox->addWidget(treeWidget);
    self->setLayout(vbox);

    addButton->sigClicked().connect([&](){ onAddButtonClicked(); });
    removeButton->sigClicked().connect([&](){ onRemoveButtonClicked(); });
    clearButton->sigClicked().connect([&](){ onClearButtonClicked(); });
    openButton->sigClicked().connect([&](){ onOpenButtonClicked(); });
}


BookmarkManagerView::~BookmarkManagerView()
{
    delete impl;
}


void BookmarkManagerView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<BookmarkManagerView>(
        "BookmarkManagerView", N_("BookmarkManager"), ViewManager::SINGLE_OPTIONAL);
}


bool BookmarkManagerView::openDialogToLoadProject(const string& filename)
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


void BookmarkManagerViewImpl::addItem(const string& name)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    QString filename = QString::fromStdString(name);
    item->setText(FILE, filename);
    treeWidget->setCurrentItem(item);
}


void BookmarkManagerViewImpl::numberingItems()
{
    int numItems = treeWidget->topLevelItemCount();
    for(int i = 0; i < numItems; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            QString no = QString::number(i);
            item->setText(NO, no);
        }
    }
}


void BookmarkManagerViewImpl::onAddButtonClicked()
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
    numberingItems();
}


void BookmarkManagerViewImpl::onRemoveButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
    numberingItems();
}


void BookmarkManagerViewImpl::onClearButtonClicked()
{
    int numItems = treeWidget->topLevelItemCount();
    for(int i = 0; i < numItems; ++i) {
        onRemoveButtonClicked();
    }
}


void BookmarkManagerViewImpl::onOpenButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(FILE).toStdString();
        bool on = BookmarkManagerView::openDialogToLoadProject(filename);
        if(!on) {
            return;
        }
    }
}


bool BookmarkManagerView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool BookmarkManagerViewImpl::storeState(Archive& archive)
{
    int numItems = treeWidget->topLevelItemCount();
    archive.write("num_projects", numItems);
    for(int i = 0; i < numItems; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string key = "project" + to_string(i);
            string filename = item->text(FILE).toStdString();
            archive.write(key, filename);
        }
    }
    return true;
}


bool BookmarkManagerView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool BookmarkManagerViewImpl::restoreState(const Archive& archive)
{
    onClearButtonClicked();
    int numItems;
    archive.read("num_projects", numItems);
    for(int i = 0; i < numItems; ++i) {
        string key = "project" + to_string(i);
        string filename;
        archive.read(key, filename);
        addItem(filename);
    }
    numberingItems();
    return true;
}


void BookmarkManagerView::onActivated()
{
    impl->onActivated();
}


void BookmarkManagerViewImpl::onActivated()
{

}


void BookmarkManagerView::onDeactivated()
{
    impl->onDeactivated();
}


void BookmarkManagerViewImpl::onDeactivated()
{

}
