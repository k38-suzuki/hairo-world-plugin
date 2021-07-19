/**
   \file
   \author Kenta Suzuki
*/

#include "FileBoxWidget.h"
#include <cnoid/BodyItem>
#include <cnoid/ItemTreeView>
#include <cnoid/MessageView>
#include <cnoid/RootItem>
#include <cnoid/SceneItem>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

void loadBody(const string& fileName)
{
    ItemTreeView* itemTreeView = ItemTreeView::instance();

    BodyItemPtr bodyItem(new BodyItem());
    if(bodyItem->load(fileName, "CHOREONOID-BODY")) {
        bodyItem->setChecked(true);

        RootItem* rootItem = RootItem::instance();
        ItemList<Item> items = rootItem->selectedItems();

        if(items.size()) {
            for(int j = 0; j < items.size(); ++j) {
                Item* item = items[j];
                item->addChildItem(bodyItem);
            }
        } else {
            RootItem* rootItem = itemTreeView->itemTreeWidget()->projectRootItem();
            rootItem->addChildItem(bodyItem);
        }
    }
}


void loadScene(const string& fileName)
{
    ItemTreeView* itemTreeView = ItemTreeView::instance();

    SceneItemPtr sceneItem(new SceneItem());
    if(sceneItem->load(fileName)) {
        sceneItem->setChecked(true);

        RootItem* rootItem = RootItem::instance();
        ItemList<Item> items = rootItem->selectedItems();
        if(items.size()) {
            for(int j = 0; j < items.size(); ++j) {
                Item* item = items[j];
                item->addChildItem(sceneItem);
            }
        } else {
            RootItem* rootItem = itemTreeView->itemTreeWidget()->projectRootItem();
            rootItem->addChildItem(sceneItem);
        }
    }
}

}


FileBoxWidget::FileBoxWidget()
{
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(new QLabel(_("Drop Body/Scene files here.")));
    setLayout(vbox);
    setAcceptDrops(true);
}


FileBoxWidget::~FileBoxWidget()
{

}


void FileBoxWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}


void FileBoxWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    QList<QUrl> urls = mime->urls();

    for(int i = 0; i < urls.size(); ++i) {
        QString fileName = urls[i].toLocalFile();

        QFileInfo info(fileName);
        QString ext = info.suffix().toLower();
        QStringList bodyExts = { "body", "yaml", "yml"/*, "wrl"*/};
        QStringList sceneExts = {"blend", "dae", "dxf", "obj", "scen", "stl", "wrl", "x"};

        for(int j = 0; j < bodyExts.size(); ++j) {
            QString bodyExt = bodyExts[j];
            if(ext == bodyExt) {
                loadBody(fileName.toStdString());
            }
        }

        for(int j = 0; j < sceneExts.size(); ++j) {
            QString sceneExt = sceneExts[j];
            if(ext == sceneExt) {
                loadScene(fileName.toStdString());
            }
        }
    }
}
