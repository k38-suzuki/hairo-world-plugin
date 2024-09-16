/**
    @author Kenta Suzuki
*/

#include "DroppableWidget.h"
#include <cnoid/BodyItem>
#include <cnoid/Format>
#include <cnoid/MessageView>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/stdx/filesystem>
#include <cnoid/WorldItem>
#include <cnoid/ToolsUtil>
#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class BodyDroppableWidget : public DroppableWidget
{
public:
    BodyDroppableWidget(QWidget* parent = nullptr)
        : DroppableWidget(parent)
    {
        setFixedSize(640, 480);

        auto vbox = new QVBoxLayout;
        auto label = new QLabel(_("Drop body(*.body) or project(*.cnoid) here."));
        label->setAlignment(Qt::AlignCenter);
        vbox->addWidget(label);
        setLayout(vbox);
    }

    ~BodyDroppableWidget() {}

    virtual bool load(const string& filename) override
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
            }
        } else {
            MessageView::instance()->putln(formatR(_("{0} are not supported."), filename));
            return false;
        }

        return true;
    }
};

}


void DroppableWidget::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        auto button = fileBar()->addButton(QIcon::fromTheme("document-send"));
        button->setToolTip(_("Drop files"));
        button->sigClicked().connect([&](){
            BodyDroppableWidget* widget = new BodyDroppableWidget;
            widget->show(); });
    }
}


DroppableWidget::DroppableWidget(QWidget* parent)
    : Widget(parent)
{
    setAcceptDrops(true);
}


DroppableWidget::~DroppableWidget()
{

}


void DroppableWidget::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}


void DroppableWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}


void DroppableWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}


void DroppableWidget::dropEvent(QDropEvent* event)
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