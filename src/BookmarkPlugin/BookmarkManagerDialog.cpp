/**
   \file
   \author Kenta Suzuki
*/

#include "BookmarkManagerDialog.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/Menu>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SimulationBar>
#include <cnoid/TreeWidget>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QStyle>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BookmarkManagerDialogImpl
{
public:
    BookmarkManagerDialogImpl(BookmarkManagerDialog* self);
    virtual ~BookmarkManagerDialogImpl();
    BookmarkManagerDialog* self;

    TreeWidget* treeWidget;
    Menu contextMenu;
    CheckBox* autoCheck;

    void addItem(const string& filename);
    void removeItem();
    void onAddButtonClicked();
    void onSetButtonClicked();
    void onStartButtonClicked();
    void onCustomContextMenuRequested(const QPoint& pos);
    void store(Mapping& archive);
    void restore(const Mapping& archive);
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
    self->connect(treeWidget, &TreeWidget::customContextMenuRequested, [=](const QPoint& pos){ onCustomContextMenuRequested(pos); });

    QHBoxLayout* hbox = new QHBoxLayout;
    auto addButton = new PushButton;
    addButton->setIcon(QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_DialogOpenButton)));
    addButton->sigClicked().connect([&](){ onAddButtonClicked(); });
    auto removeButton = new PushButton;
    removeButton->setIcon(QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_TrashIcon)));
    removeButton->sigClicked().connect([&](){ removeItem(); });
    auto setButton = new PushButton;
    setButton->setIcon(QIcon(MainWindow::instance()->style()->standardIcon(QStyle::SP_FileDialogNewFolder)));
    setButton->sigClicked().connect([&](){ onSetButtonClicked(); });
    autoCheck = new CheckBox;
    autoCheck->setText(_("Autoplay"));
    hbox->addWidget(addButton);
    // hbox->addWidget(setButton);
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

    Mapping& config = *AppConfig::archive()->openMapping("bookmark_manager");
    if(config.isValid()) {
        restore(config);
    }
}


BookmarkManagerDialog::~BookmarkManagerDialog()
{
    delete impl;
}


BookmarkManagerDialogImpl::~BookmarkManagerDialogImpl()
{
    store(*AppConfig::archive()->openMapping("bookmark_manager"));
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
    impl->addItem(filename);
}


void BookmarkManagerDialogImpl::addItem(const string& filename)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, filename.c_str());
    // item->setFlags(item->flags() | Qt::ItemIsEditable);
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

    if(dialog.exec()) {
        int numFiles = dialog.selectedFiles().size();
        for(int i = 0; i < numFiles; ++i) {
            QString filename = dialog.selectedFiles()[i];
            addItem(filename.toStdString());
        }
    }
}


void BookmarkManagerDialogImpl::onSetButtonClicked()
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setText(0, "new item");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    treeWidget->setCurrentItem(item);
}


void BookmarkManagerDialogImpl::onStartButtonClicked()
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
            if(autoCheck->isChecked()) {
                SimulationBar::instance()->startSimulation(true);
            }
        }
    }
}


void BookmarkManagerDialogImpl::onCustomContextMenuRequested(const QPoint& pos)
{
    // contextMenu.exec(treeWidget->mapToGlobal(pos));
}


void BookmarkManagerDialogImpl::store(Mapping& archive)
{
    archive.write("auto_play", autoCheck->isChecked());

    int size = treeWidget->topLevelItemCount();
    archive.write("num_bookmarks", size);
    for(int i = 0; i < size; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            string filename = item->text(0).toStdString();
            string fileKey = "filename_" + to_string(i);
            archive.write(fileKey, filename);
        }
    }
}


void BookmarkManagerDialogImpl::restore(const Mapping& archive)
{
    autoCheck->setChecked(archive.get("auto_play", false));

    int size = archive.get("num_bookmarks", 0);
    for(int i = 0; i < size; ++i) {
        string fileKey = "filename_" + to_string(i);
        string filename = archive.get(fileKey, "");
        if(!filename.empty()) {
            addItem(filename);
        }
    }
}