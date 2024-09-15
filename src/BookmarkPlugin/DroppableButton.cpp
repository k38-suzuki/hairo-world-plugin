/**
    @author Kenta Suzuki
*/

#include "DroppableButton.h"
#include <cnoid/BodyItem>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/WorldItem>
#include <cnoid/ToolsUtil>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

void DroppableButton::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        auto button = new DroppableButton;
        button->setIcon(QIcon::fromTheme("document-send"));
        button->setToolTip(_("Drop files"));
        fileBar()->addWidget(button);
    }
}


DroppableButton::DroppableButton(QWidget* parent)
    : ToolButton(parent)
{
    setAcceptDrops(true);
}


DroppableButton::~DroppableButton()
{

}


void DroppableButton::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}


void DroppableButton::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}


void DroppableButton::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}


void DroppableButton::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()) {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        for(int i = 0; i < urlList.size() && i < 32; ++i) {
            pathList.append(urlList.at(i).toLocalFile());
        }

        for(int i = 0; i < pathList.size(); ++i) {
            string filename = pathList.at(i).toStdString();
            if(load(filename)) {
                event->acceptProposedAction();
            }
        }
    }
}


bool DroppableButton::load(const string& filename)
{
    filesystem::path path(filename);
    string extension = path.extension();

    if(extension == ".body") {
        auto bodyItem = new BodyItem;
        bodyItem->load(filename);

        RootItem* rootItem = RootItem::instance();
        ItemList<WorldItem> worldItems = rootItem->selectedItems();
        if(!worldItems.size()) {
            rootItem->addChildItem(bodyItem);
        } else {
            worldItems[0]->addChildItem(bodyItem);
        }
    } else if(extension == ".cnoid") {
        ProjectManager* projectManager = ProjectManager::instance();
        if(projectManager->tryToCloseProject()) {
            projectManager->clearProject();
            projectManager->loadProject(filename);
        } else {
            return false;
        }
    }

    return true;
}