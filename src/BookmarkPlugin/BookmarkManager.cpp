/**
   @author Kenta Suzuki
*/

#include "BookmarkManager.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
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

namespace {

BookmarkManager* bookmarkInstance = nullptr;

}

namespace cnoid {

class BookmarkManager::Impl : public Dialog
{
public:

    TreeWidget* treeWidget;
    Menu contextMenu;
    CheckBox* autoCheck;

    Impl();
    ~Impl();

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


void BookmarkManager::initializeClass(ExtensionManager* ext)
{
    if(!bookmarkInstance) {
        bookmarkInstance = ext->manage(new BookmarkManager);
    }
}


BookmarkManager* BookmarkManager::instance()
{
    static BookmarkManager* bookmarkInstance = nullptr;
    if(!bookmarkInstance) {
        bookmarkInstance = new BookmarkManager;
    }
    return bookmarkInstance;
}


BookmarkManager::BookmarkManager()
{
    impl = new Impl;
}


BookmarkManager::Impl::Impl()
{
    setWindowTitle(_("BookmarkManager"));

    setFixedSize(800, 450);
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
    connect(treeWidget, &TreeWidget::customContextMenuRequested,
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

    auto buttonBox = new QDialogButtonBox(this);
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
    setLayout(vbox);

    auto config = AppConfig::archive()->openMapping("bookmark_manager");
    if(config->isValid()) {
        restore(config);
    }
}


BookmarkManager::~BookmarkManager()
{
    delete impl;
}


BookmarkManager::Impl::~Impl()
{
    store(AppConfig::archive()->openMapping("bookmark_manager"));
}


void BookmarkManager::show()
{
    impl->show();
}


void BookmarkManager::hide()
{
    impl->hide();
}


void BookmarkManager::addProjectFile(const string& filename)
{
    QTreeWidgetItem* parentItem = impl->treeWidget->currentItem();
    if(!parentItem) {
        parentItem = impl->treeWidget->invisibleRootItem();
    }
    impl->addItem(filename, parentItem);
}


QTreeWidgetItem* BookmarkManager::Impl::addItem(const string& filename, QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    item->setText(0, filename.c_str());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    treeWidget->setCurrentItem(item);

    return item;
}


void BookmarkManager::Impl::removeItem()
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


void BookmarkManager::Impl::onAddButtonClicked()
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


void BookmarkManager::Impl::onNewButtonClicked()
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


void BookmarkManager::Impl::onStartButtonClicked()
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


void BookmarkManager::Impl::onCustomContextMenuRequested(const QPoint& pos)
{
    // contextMenu.exec(treeWidget->mapToGlobal(pos));
}


void BookmarkManager::Impl::store(Mapping* archive)
{
    archive->write("auto_play", autoCheck->isChecked());
    storeChildItem(archive, treeWidget->invisibleRootItem());
}


void BookmarkManager::Impl::storeChildItem(Mapping* archive, QTreeWidgetItem* parentItem)
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


void BookmarkManager::Impl::restore(const Mapping* archive)
{
    autoCheck->setChecked(archive->get("auto_play", false));
    restoreChildItem(archive, treeWidget->invisibleRootItem());
}


void BookmarkManager::Impl::restoreChildItem(const Mapping* archive, QTreeWidgetItem* parentItem)
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
