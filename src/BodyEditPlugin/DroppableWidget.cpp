/**
    @author Kenta Suzuki
*/

#include "DroppableWidget.h"
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