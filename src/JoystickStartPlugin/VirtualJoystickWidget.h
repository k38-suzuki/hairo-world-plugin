/**
   @author Kenta Suzuki
*/

#ifndef CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H
#define CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H

#include <QWidget>

namespace cnoid {

class VirtualJoystickWidget : public QWidget
{
public:
    VirtualJoystickWidget(QWidget* parent = nullptr);
    virtual ~VirtualJoystickWidget();

    enum ViewMode { NormalView, DraggerView };

    void setViewMode(ViewMode mode);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

private:
    class Impl;
    Impl* impl;
};

typedef VirtualJoystickWidget::ViewMode ViewMode;

}

#endif // CNOID_JOYSTICKSTART_PLUGIN_VIRTUAL_JOYSTICK_WIDGET_H