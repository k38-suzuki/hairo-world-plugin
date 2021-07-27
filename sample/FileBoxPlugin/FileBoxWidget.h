/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Widget>

#ifndef CNOID_FILEBOXPLUGIN_FILEBOXWIDGET_H
#define CNOID_FILEBOXPLUGIN_FILEBOXWIDGET_H

namespace cnoid {

class FileBoxWidget : public Widget
{
public:
    FileBoxWidget();
    virtual ~FileBoxWidget();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent* event) override;
};

}

#endif // CNOID_FILEBOXPLUGIN_FILEBOXWIDGET_H
