/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_DROPPABLE_BUTTON_H
#define CNOID_BOOKMARK_PLUGIN_DROPPABLE_BUTTON_H

#include <cnoid/Buttons>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT DroppableButton : public ToolButton
{
public:
    static void initialize(ExtensionManager* ext);

    DroppableButton(QWidget* parent = nullptr);
    virtual ~DroppableButton();

    virtual bool load(const std::string& filename);

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_DROPPABLE_BUTTON_H