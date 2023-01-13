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
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/TreeWidget>
#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

BookmarkManager* instance_ = nullptr;

}

namespace cnoid {

class BookmarkManagerImpl : public Dialog
{
public:
    BookmarkManagerImpl(BookmarkManager* self);
    virtual ~BookmarkManagerImpl();
    BookmarkManager* self;

    TreeWidget* treeWidget;
    Menu contextMenu;

    void addItem(const string& filename);
    void removeItem();
    void onAddButtonClicked();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


BookmarkManager::BookmarkManager()
{
    impl = new BookmarkManagerImpl(this);
}


BookmarkManagerImpl::BookmarkManagerImpl(BookmarkManager* self)
    : self(self)
{
    setWindowTitle(_("BookmarkManager"));

    setFixedSize(800, 450);
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(false);

    treeWidget->setHeaderLabel(_("File"));

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    Action* addAct = new Action;
    addAct->setText(_("Add"));
    contextMenu.addAction(addAct);
    Action* removeAct = new Action;
    removeAct->setText(_("Remove"));
    contextMenu.addAction(removeAct);
    Action* openAct = new Action;
    openAct->setText(_("Open"));
    contextMenu.addAction(openAct);

    addAct->sigTriggered().connect([&](){ onAddButtonClicked(); });
    removeAct->sigTriggered().connect([&](){ removeItem(); });
    openAct->sigTriggered().connect([&](){ onStartButtonClicked(); });
    connect(treeWidget, &TreeWidget::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    auto buttonBox = new QDialogButtonBox(this);
    auto startButton  = new PushButton(_("&Open"));
    startButton->setIconSize(MainWindow::instance()->iconSize());
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    // connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->onStartButtonClicked(); });
    startButton->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(treeWidget);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    Mapping& config = *AppConfig::archive()->openMapping("bookmark_manager");
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
    store(*AppConfig::archive()->openMapping("bookmark_manager"));
}


void BookmarkManager::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new BookmarkManager);
    }

    // MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
    // mm.addItem(_("BookmarkManager"))->sigTriggered().connect(
    //     [&](){ instance_->impl->show(); });
}


BookmarkManager* BookmarkManager::instance()
{
    return instance_;
}


void BookmarkManager::showBookmarkManagerDialog()
{
    instance_->impl->show();
}


void BookmarkManagerImpl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    treeWidget->setCurrentItem(item);
}


void BookmarkManagerImpl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void BookmarkManagerImpl::onAddButtonClicked()
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

    if(dialog.exec()) {
        int numFiles = dialog.selectedFiles().size();
        for(int i = 0; i < numFiles; ++i) {
            QString filename = dialog.selectedFiles()[i];
            addItem(filename.toStdString());
        }
    }
}


void BookmarkManagerImpl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        ProjectManager* pm = ProjectManager::instance();
        bool result = pm->tryToCloseProject();
        if(result) {
            pm->clearProject();
            MessageView::instance()->flush();
            pm->loadProject(filename);
            SimulationBar::instance()->startSimulation(true);
        }
    }
}


void BookmarkManagerImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    contextMenu.exec(treeWidget->mapToGlobal(pos));
}


void BookmarkManagerImpl::store(Mapping& archive)
{
    int size = treeWidget->topLevelItemCount();
    archive.write("num_bookmarks", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string fileKey = "file_name_" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
}


void BookmarkManagerImpl::restore(const Mapping& archive)
{
    int size = archive.get("num_bookmarks", 0);
    for(int i = 0; i < size; ++i) {
        string fileKey = "file_name_" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }
}