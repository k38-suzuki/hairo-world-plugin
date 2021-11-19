/**
   \file
   \author Kenta Suzuki
*/

#include "ProcessManager.h"
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

ProcessManager* manager = nullptr;
const string programs[] = { "nautilus", "gedit" };

}


namespace cnoid {

class ProcessManagerImpl
{
public:
    ProcessManagerImpl(ProcessManager* self);
    virtual ~ProcessManagerImpl();
    ProcessManager* self;

    vector<Process*> processes;

    void execute(const int argc, const char* argv[]);
};

}


ProcessManager::ProcessManager()
{
    impl = new ProcessManagerImpl(this);
}


ProcessManagerImpl::ProcessManagerImpl(ProcessManager* self)
    : self(self)
{
    processes.clear();
}


ProcessManager::~ProcessManager()
{
    delete impl;
}


ProcessManagerImpl::~ProcessManagerImpl()
{
    for(size_t i = 0; i < processes.size(); ++i) {
        Process* process = processes[i];
        if(process->state() != QProcess::NotRunning) {
            process->kill();
            process->waitForFinished(100);
        }
    }
}


void ProcessManager::initialize(ExtensionManager* ext)
{
    if(!manager) {
        manager = ext->manage(new ProcessManager);
    }

    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ manager->execute(item, ProgramId::GEDIT); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ manager->execute(item, ProgramId::NAUTILUS); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ manager->execute(item, ProgramId::GEDIT); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ manager->execute(item, ProgramId::NAUTILUS); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void ProcessManager::execute(const Item* item, const int& id)
{
    const int argc = 2;
    const char* argv[] = { programs[id].c_str(), item->filePath().c_str() };
    impl->execute(argc, argv);
}


void ProcessManager::execute(const int argc, const char* argv[])
{
    impl->execute(argc, argv);
}


void ProcessManagerImpl::execute(const int argc, const char* argv[])
{
    string messages;
    for(int i = 0; i < argc; ++i) {
        messages += " " + string(argv[i]);
    }

    Process* process = new Process();
    process->start(messages.c_str());
    if(process->waitForStarted()) {}

    processes.push_back(process);
}
