/**
   \file
   \author Kenta Suzuki
*/

#include "FileExplorer.h"
#include <cnoid/BodyItem>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/Process>
#include <cnoid/SceneItem>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QTreeView>
#include <vector>
#include "Notepad.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

FileExplorer* explorerInstance = nullptr;

class FileExplorerImpl
{
public:
    FileExplorerImpl(FileExplorer* self);
    FileExplorer* self;

    QFileSystemModel* model;
    QTreeView* treeView;
    vector<Process*> processes;

    void execute(const Item* item, const int& id);
    void execute(const int argc, const char* argv[]);
    void kill();
    void openFile(const QString& fileName);
    void openDir(const QString& fileName);
    void on_treeView_doubleClicked(const QModelIndex& index);
};

}


FileExplorer::FileExplorer()
{
    impl = new FileExplorerImpl(this);
}


FileExplorerImpl::FileExplorerImpl(FileExplorer* self)
    : self(self)
{
    model = new QFileSystemModel;

    treeView = new QTreeView;
    QObject::connect(treeView, &QTreeView::doubleClicked,
        [&](const QModelIndex& index){ on_treeView_doubleClicked(index); });

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
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 0); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 1); });
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openDir(item->filePath().c_str()); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [&](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            // menuManager.addItem(_("File"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 0); });
            // menuManager.addItem(_("Directory"))->sigTriggered().connect(
            //     [&, item](){ explorerInstance->impl->execute(item, 1); });
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openFile(item->filePath().c_str()); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ explorerInstance->impl->openDir(item->filePath().c_str()); });
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
    QFileInfo info(fileName);
    model->setRootPath(info.absolutePath());
    treeView->setModel(model);
    treeView->setRootIndex(model->index(info.absolutePath()));
    treeView->setWindowTitle(info.absolutePath());
    treeView->show();
}


void FileExplorerImpl::on_treeView_doubleClicked(const QModelIndex& index)
{
    if(!model->isDir(index)) {
        QString filePath = model->filePath(index);
        if(filePath.isEmpty()) {
            return;
        }
        openFile(filePath);
    }
}