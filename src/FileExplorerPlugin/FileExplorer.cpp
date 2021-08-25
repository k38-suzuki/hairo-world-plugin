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

using namespace std;
using namespace cnoid;

namespace {

FileExplorer* explorer = nullptr;

}


namespace cnoid {

class FileExplorerImpl
{
public:
    FileExplorerImpl(FileExplorer* self);
    FileExplorer* self;

    vector<Process*> processes;

    void execute(const Item* item, const int& type);
    void finalize();
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
    delete impl;
}


void FileExplorer::initializeClass(ExtensionManager* ext)
{
    if(!explorer) {
        explorer = new FileExplorer();
    }

    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ explorer->execute(item, ToolType::NAUTILUS); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ explorer->execute(item, ToolType::GEDIT); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ explorer->execute(item, ToolType::NAUTILUS); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ explorer->execute(item, ToolType::GEDIT); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void FileExplorer::finalizeClass()
{
//    explorer->finalize();
}


void FileExplorer::execute(const Item* item, const int& type)
{
    impl->execute(item, type);
}


void FileExplorerImpl::execute(const Item* item, const int& type)
{
    string message = type ? "nautilus" : "gedit";
    message += " " +  item->filePath();
    Process* process = new Process();
    process->start(message.c_str());
    if(process->waitForStarted()) {}

    processes.push_back(process);

}


void FileExplorer::finalize()
{
    impl->finalize();
}


void FileExplorerImpl::finalize()
{
    for(size_t i = 0; i < processes.size(); ++i) {
        Process* process = processes[i];
        if(process->state() != QProcess::NotRunning){
            process->kill();
            process->waitForFinished(100);
        }
    }
}
