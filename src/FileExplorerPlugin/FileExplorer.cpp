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

class FileExplorerImpl
{
public:
    FileExplorerImpl(FileExplorer* self);
    FileExplorer* self;

    enum ProgramID { NAUTILUS, GEDIT, NUM_PROGRAMS };

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
    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [&](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ execute(item, GEDIT); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ execute(item, NAUTILUS); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [&](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [&, item](){ execute(item, GEDIT); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [&, item](){ execute(item, NAUTILUS); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


FileExplorer::~FileExplorer()
{
    impl->kill();
    delete impl;
}


void FileExplorer::initialize(ExtensionManager* ext)
{
    ext->manage(new FileExplorer);
}


void FileExplorerImpl::execute(const Item* item, const int& id)
{
    static const string programs[] = { "nautilus", "gedit" };
    const int argc = 2;
    const char* argv[] = { programs[id].c_str(), item->filePath().c_str() };
    execute(argc, argv);
}


void FileExplorerImpl::execute(const int argc, const char* argv[])
{
    string messages;
    for(int i = 0; i < argc; ++i) {
        messages += " " + string(argv[i]);
    }

    Process* process = new Process;
    process->start(messages.c_str());
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
