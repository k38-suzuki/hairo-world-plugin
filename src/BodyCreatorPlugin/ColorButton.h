/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_CREATOR_PLUGIN_COLOR_BUTTON_H
#define CNOID_BODY_CREATOR_PLUGIN_COLOR_BUTTON_H

#include <cnoid/EigenTypes>
#include <cnoid/PushButton>
#include <cnoid/Signal>

namespace cnoid {

class ColorButton : public PushButton
{
public:
    ColorButton(QWidget* parent = nullptr);

    void setColor(const Vector3& color);
    Vector3 color() const { return color_; }

    SignalProxy<void(const Vector3& color)> sigColorSelected();

private:
    Signal<void(const Vector3& color)> sigColorSelected_;
    Vector3 color_;

    void setColor(const QColor& color);
    void onButtonClicked();
};

}

#endif // CNOID_BODY_CREATOR_PLUGIN_COLOR_BUTTON_H
