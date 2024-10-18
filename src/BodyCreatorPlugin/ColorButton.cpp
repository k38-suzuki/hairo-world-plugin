/**
   @author Kenta Suzuki
*/

#include "ColorButton.h"
#include <cnoid/MainWindow>
#include <QColorDialog>
#include <QPalette>
#include "gettext.h"

using namespace cnoid;


ColorButton::ColorButton(QWidget* parent)
    : PushButton(parent)
{
    this->sigClicked().connect([&](){ onButtonClicked(); });
}


void ColorButton::setColor(const Vector3& color)
{
    color_ = color;
    QColor currentColor;
    currentColor.setRgb(color_[0] * 255.0, color_[1] * 255.0, color_[2] * 255.0);
    setColor(currentColor);
}


void ColorButton::setColor(const QColor& color)
{
    QPalette palette;
    palette.setColor(QPalette::Button, color);
    setPalette(palette);
}


SignalProxy<void(const Vector3& color)> ColorButton::sigColorSelected()
{
    return sigColorSelected_;
}


void ColorButton::onButtonClicked()
{
    QColorDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Select a color"));
    dialog.setOption (QColorDialog::DontUseNativeDialog);

    if(dialog.exec()) {
        QColor currentColor = dialog.currentColor();
        setColor(Vector3(currentColor.red() / 255.0, currentColor.green() / 255.0, currentColor.blue() / 255.0));
        sigColorSelected_(color_);
    }
}
