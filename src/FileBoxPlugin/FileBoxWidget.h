/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Widget>

#ifndef CNOID_FILE_BOX_PLUGIN_FILE_BOX_WIDGET_H
#define CNOID_FILE_BOX_PLUGIN_FILE_BOX_WIDGET_H

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

#endif // CNOID_FILE_BOX_PLUGIN_FILE_BOX_WIDGET_H
