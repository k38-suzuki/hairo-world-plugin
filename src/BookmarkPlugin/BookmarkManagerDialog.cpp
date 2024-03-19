/**
   @author Kenta Suzuki
*/

#include "BookmarkManagerDialog.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ItemManager>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/stdx/filesystem>
#include <cnoid/TreeWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class BookmarkManagerDialog::Impl
{
public:
    BookmarkManagerDialog* self;

    Impl(BookmarkManagerDialog* self);
    ~Impl();

    TreeWidget* treeWidget;
    Menu contextMenu;
    CheckBox* autoCheck;

    QTreeWidgetItem* addItem(const string& filename, QTreeWidgetItem* parentItem);
    void removeItem();
    void onAddButtonClicked();
    void onNewButtonClicked();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    void store(Mapping* archive);
    void storeChildItem(Mapping* archive, QTreeWidgetItem* parentItem);
    void restore(const Mapping* archive);
    void restoreChildItem(const Mapping* archive, QTreeWidgetItem* parentItem);
};

}


BookmarkManagerDialog::BookmarkManagerDialog()
{
    impl = new Impl(this);
}


BookmarkManagerDialog::Impl::Impl(BookmarkManagerDialog* self)
    : self(self)
{
    self->setWindowTitle(_("BookmarkManager"));

    self->setFixedSize(800, 450);
    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(true);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
    treeWidget->setDragEnabled(true);
    treeWidget->viewport()->setAcceptDrops(true);

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
    self->connect(treeWidget, &TreeWidget::customContextMenuRequested,
        [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    QHBoxLayout* hbox = new QHBoxLayout;
    auto addButton = new PushButton;
    QIcon addIcon = QIcon::fromTheme("document-open");
    if(addIcon.isNull()) {
        addButton->setText(_("Add"));
    } else {
        addButton->setIcon(addIcon);
    }
    addButton->sigClicked().connect([&](){ onAddButtonClicked(); });
    auto removeButton = new PushButton;
    QIcon removeIcon = QIcon::fromTheme("user-trash");
    if(removeIcon.isNull()) {
        removeButton->setText(_("Remove"));
    } else {
        removeButton->setIcon(removeIcon);
    }
    removeButton->sigClicked().connect([&](){ removeItem(); });
    auto newButton = new PushButton;
    QIcon newIcon = QIcon::fromTheme("document-new");
    if(newIcon.isNull()) {
        newButton->setText(_("New"));
    } else {
        newButton->setIcon(newIcon);
    }
    newButton->sigClicked().connect([&](){ onNewButtonClicked(); });
    autoCheck = new CheckBox;
    autoCheck->setText(_("Autoplay"));
    hbox->addWidget(newButton);
    hbox->addWidget(addButton);
    hbox->addWidget(autoCheck);
    hbox->addStretch();
    hbox->addWidget(removeButton);

    auto buttonBox = new QDialogButtonBox(self);
    auto startButton  = new PushButton(_("&Open"));
    startButton->setIconSize(MainWindow::instance()->iconSize());
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    // connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->onStartButtonClicked(); });
    startButton->sigClicked().connect([&](){ onStartButtonClicked(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(treeWidget);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    if(config->isValid()) {
        restore(config);
    }
}


BookmarkManagerDialog::~BookmarkManagerDialog()
{
    delete impl;
}


BookmarkManagerDialog::Impl::~Impl()
{
    store(AppConfig::archive()->openMapping("bookmark_manager"));
}


BookmarkManagerDialog* BookmarkManagerDialog::instance()
{
    static BookmarkManagerDialog* instance_ = nullptr;
    if(!instance_) {
        instance_ = new BookmarkManagerDialog;
    }
    return instance_;
}


void BookmarkManagerDialog::addProjectFile(const string& filename)
{
    QTreeWidgetItem* parentItem = impl->treeWidget->currentItem();
    if(!parentItem) {
        parentItem = impl->treeWidget->invisibleRootItem();
    }
    impl->addItem(filename, parentItem);
}


QTreeWidgetItem* BookmarkManagerDialog::Impl::addItem(const string& filename, QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    item->setText(0, filename.c_str());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    treeWidget->setCurrentItem(item);

    return item;
}


void BookmarkManagerDialog::Impl::removeItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        QTreeWidgetItem* parentItem = item->parent();
        if(parentItem) {
            parentItem->takeChildren();
        } else {
            int index = treeWidget->indexOfTopLevelItem(item);
            treeWidget->takeTopLevelItem(index);
        }
    }
}


void BookmarkManagerDialog::Impl::onAddButtonClicked()
{
    vector<string> filenames = getOpenFileNames(_("Open a project"), "cnoid");
    for(size_t i = 0; i < filenames.size(); ++i) {
        QTreeWidgetItem* parentItem = treeWidget->currentItem();
        if(!parentItem) {
            parentItem = treeWidget->invisibleRootItem();
        }
        addItem(filenames[i].c_str(), parentItem);
    }
}


void BookmarkManagerDialog::Impl::onNewButtonClicked()
{
    QTreeWidgetItem* parentItem = treeWidget->currentItem();
    if(!parentItem) {
        parentItem = treeWidget->invisibleRootItem();
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    item->setText(0, "new item");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    treeWidget->setCurrentItem(item);
}


void BookmarkManagerDialog::Impl::onStartButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        string filename = item->text(0).toStdString();
        if(!filename.empty()) {
            filesystem::path path(filename);
            string extension = path.extension().string();
            if(extension == ".cnoid") {
                ProjectManager* pm = ProjectManager::instance();
                bool result = pm->tryToCloseProject();
                if(result) {
                    pm->clearProject();
                    pm->loadProject(filename);
                    if(autoCheck->isChecked()) {
                        SimulationBar::instance()->startSimulation(true);
                    }
                }
            }
        }
    }
}


void BookmarkManagerDialog::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    // contextMenu.exec(treeWidget->mapToGlobal(pos));
}


void BookmarkManagerDialog::Impl::store(Mapping* archive)
{
    archive->write("auto_play", autoCheck->isChecked());
    storeChildItem(archive, treeWidget->invisibleRootItem());
}


void BookmarkManagerDialog::Impl::storeChildItem(Mapping* archive, QTreeWidgetItem* parentItem)
{
    ListingPtr childItemListing = new Listing;

    for(int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem* item = parentItem->child(i);
        if(item) {
            string filename = item->text(0).toStdString();

            ArchivePtr subArchive = new Archive;
            subArchive->write("file", filename);
            storeChildItem(subArchive, item);

            childItemListing->append(subArchive);
        }
    }

    archive->insert("children", childItemListing);
}


void BookmarkManagerDialog::Impl::restore(const Mapping* archive)
{
    autoCheck->setChecked(archive->get("auto_play", false));
    restoreChildItem(archive, treeWidget->invisibleRootItem());
}


void BookmarkManagerDialog::Impl::restoreChildItem(const Mapping* archive, QTreeWidgetItem* parentItem)
{
    ListingPtr childItemListing = archive->findListing("children");
    if(childItemListing->isValid()) {
        for(int i = 0; i < childItemListing->size(); ++i) {
            auto subArchive = childItemListing->at(i)->toMapping();
            string filename;
            subArchive->read("file", filename);
            QTreeWidgetItem* item = addItem(filename, parentItem);
            restoreChildItem(subArchive, item);
        }
    }
}
