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
#include <vector>
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

    vector<Process*> processes;

    void execute(const Item* item, const int& id);
    void execute(const int argc, const char* argv[]);
    void kill();
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