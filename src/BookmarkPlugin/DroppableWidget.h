/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_DROPPABLE_WIDGET_H
#define CNOID_BOOKMARK_PLUGIN_DROPPABLE_WIDGET_H

#include <cnoid/Widget>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT DroppableWidget : public Widget
{
public:
    static void initialize(ExtensionManager* ext);

    DroppableWidget(QWidget* parent = nullptr);
    virtual ~DroppableWidget();

    virtual bool load(const std::string& filename) = 0;

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_DROPPABLE_WIDGET_H