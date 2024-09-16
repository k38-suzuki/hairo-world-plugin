/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_FILE_DROPPABLE_WIDGET_H
#define CNOID_BOOKMARK_PLUGIN_FILE_DROPPABLE_WIDGET_H

#include <cnoid/Signal>
#include <cnoid/Widget>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT FileDroppableWidget : public Widget
{
public:
    FileDroppableWidget(QWidget* parent = nullptr);
    virtual ~FileDroppableWidget();

    SignalProxy<void(std::string& filename)> sigFileDropped() { return sigFileDropped_; }

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

private:
    bool load(const std::string& filename);

    Signal<void(std::string& filename)> sigFileDropped_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_FILE_DROPPABLE_WIDGET_H