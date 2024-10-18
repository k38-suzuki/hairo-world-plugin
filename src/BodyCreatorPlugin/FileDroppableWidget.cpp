/**
    @author Kenta Suzuki
*/

#include "FileDroppableWidget.h"
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


FileDroppableWidget::FileDroppableWidget(QWidget* parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
}


FileDroppableWidget::~FileDroppableWidget()
{

}


void FileDroppableWidget::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}


void FileDroppableWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}


void FileDroppableWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}


void FileDroppableWidget::dropEvent(QDropEvent* event)
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
                sigFileDropped_(filename);
                event->acceptProposedAction();
            }
        }
    }
}


bool FileDroppableWidget::load(const string& filename)
{
    return true;
}