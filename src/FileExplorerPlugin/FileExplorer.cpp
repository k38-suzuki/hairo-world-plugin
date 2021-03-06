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
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

Process process;

bool onProcessKilled()
{
    if(process.state() != QProcess::NotRunning){
        process.kill();
        return process.waitForFinished(100);
    }
    return false;
}


void onItemTriggered(const Item* item, const int& index)
{
    string message = index ? "nautilus" : "gedit";
    message += " " +  item->filePath();
    onProcessKilled();
    process.start(message.c_str());
    if(process.waitForStarted()) {}
}

}


FileExplorer::FileExplorer()
{

}


FileExplorer::~FileExplorer()
{

}


void FileExplorer::initializeClass(ExtensionManager* ext)
{
    ItemTreeView::instance()->customizeContextMenu<BodyItem>(
        [](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ onItemTriggered(item, 0); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ onItemTriggered(item, 1); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });

    ItemTreeView::instance()->customizeContextMenu<SceneItem>(
        [](SceneItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("File"))->sigTriggered().connect(
                [item](){ onItemTriggered(item, 0); });
            menuManager.addItem(_("Directory"))->sigTriggered().connect(
                [item](){ onItemTriggered(item, 1); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


void FileExplorer::finalizeClass()
{
    onProcessKilled();
}
