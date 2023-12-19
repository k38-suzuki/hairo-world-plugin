/**
   \file
   \author Kenta Suzuki
*/

#include "FileExplorer.h"
#include <cnoid/BodyItem>
#include <cnoid/Dialog>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/Process>
#include <cnoid/SceneItem>
#include <cnoid/Separator>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QListView>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <vector>
#include "Notepad.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

FileExplorer* explorerInstance = nullptr;

class ExplorerDialog : public Dialog
{
public:
    ExplorerDialog(QWidget* parent = nullptr);

    void openDir(const QString& fileName);

    void on_listView_doubleClicked(const QModelIndex& index);
    void on_treeView_clicked(const QModelIndex& index);

    QFileSystemModel* fileModel;
    QFileSystemModel* dirModel;
    QListView* listView;
    QTreeView* treeView;
};

class FileExplorerImpl
{
public:
    FileExplorerImpl(FileExplorer* self);
    FileExplorer* self;

    vector<Process*> processes;

    void execute(const Item* item, const int& id);
    void execute(const int argc, const char* argv[]);
    void kill();
    void openFile(const QString& fileName);
    void openDir(const QString& fileName);
};

}


FileExplorer::FileExplorer()
{
    impl = new FileExplorerImpl(this);
}


FileExplorerImpl::FileExplorerImpl(FileExplorer* self)
    : self(self)
{
    processes.clear();
}


FileExplorer::~FileExplorer()
{
    impl->kill();
    delete impl;
}


void FileExplorer::initializeClass(ExtensionManager* ext)
{
    if(!explorerInstance) {
        explorerInstance = ext->manage(new FileExplorer);
    }

    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [&](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->execute(item, 0); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->execute(item, 1); });
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->openDir(item->filePath().c_str()); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [&](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->execute(item, 0); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->execute(item, 1); });
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->openDir(item->filePath().c_str()); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void FileExplorerImpl::execute(const Item* item, const int& id)
{
    static const string program[] = { "gedit", "nautilus" };
    const int argc = 2;
    const char* argv[] = { program[id].c_str(), item->filePath().c_str() };
    execute(argc, argv);
}


void FileExplorerImpl::execute(const int argc, const char* argv[])
{
    string actualCommand = argv[0];
    QStringList arguments = { argv[1] };

    Process* process = new Process;
    process->start(actualCommand.c_str(), arguments);
    if(process->waitForStarted()) {}
    processes.push_back(process);
}


void FileExplorerImpl::kill()
{
    for(size_t i = 0; i < processes.size(); ++i) {
        Process* process = processes[i];
        if(process) {
            if(process->state() != QProcess::NotRunning) {
                process->kill();
                process->waitForFinished(100);
            }
        }
    }
}


void FileExplorerImpl::openFile(const QString& fileName)
{
    Notepad* notepad = new Notepad;
    notepad->loadFile(fileName);
    notepad->show();
}


void FileExplorerImpl::openDir(const QString& fileName)
{
    ExplorerDialog* dialog = new ExplorerDialog;
    dialog->openDir(fileName);
    dialog->show();
}


ExplorerDialog::ExplorerDialog(QWidget* parent)
    : Dialog(parent)
{
    fileModel = new QFileSystemModel;
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    dirModel = new QFileSystemModel;
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    listView = new QListView;
    listView->setModel(fileModel);
    treeView = new QTreeView;
    treeView->setModel(dirModel);

    connect(listView, &QListView::doubleClicked,
        [&](const QModelIndex& index){ on_listView_doubleClicked(index); });
    connect(treeView, &QTreeView::clicked,
        [&](const QModelIndex& index){ on_treeView_clicked(index); });

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(treeView);
    splitter->addWidget(listView);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_(""));
}


void ExplorerDialog::openDir(const QString& fileName)
{
    QFileInfo info(fileName);
    QString filePath = info.absolutePath();
    dirModel->setRootPath(filePath);
    treeView->setRootIndex(dirModel->index(filePath));
    on_treeView_clicked(dirModel->index(filePath));
    setWindowTitle(filePath);
}


void ExplorerDialog::on_listView_doubleClicked(const QModelIndex& index)
{
    if(!fileModel->isDir(index)) {
        QString fileName = fileModel->filePath(index);
        if(fileName.isEmpty()) {
            return;
        }

        Notepad* notepad = new Notepad;
        notepad->loadFile(fileName);
        notepad->show();
    }
}


void ExplorerDialog::on_treeView_clicked(const QModelIndex& index)
{
    QString filePath = dirModel->fileInfo(index).absoluteFilePath();
    listView->setRootIndex(fileModel->setRootPath(filePath));
}