/**
   \file
   \author Kenta Suzuki
*/

#include "FileExplorer.h"
#include <cnoid/BodyItem>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/SceneItem>
#include <sys/types.h>
#include <unistd.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

void onItemTriggered(const Item* item, int index)
{
    string message = index ? "nautilus" : "gedit";
    message += " " +  item->filePath();

    pid_t pid = fork();
    if(pid == -1) {
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
        int ret = system(message.c_str());
        exit(EXIT_SUCCESS);
    }
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
