/**
   \file
   \author Kenta Suzuki
*/

#include "PipeBuilderWidget.h"
#include <cnoid/Button>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class PipeBuilderWidgetImpl
{
public:
    PipeBuilderWidgetImpl(PipeBuilderWidget* self);
    PipeBuilderWidget* self;

    DoubleSpinBox* massSpin;
    DoubleSpinBox* innerDiameterSpin;
    DoubleSpinBox* outerDiameterSpin;
    DoubleSpinBox* lengthSpin;
    SpinBox* angleSpin;
    SpinBox* stepSpin;
    PushButton* colorButton;

    void writeYaml(const string& filename);
    void onColorButtonClicked();
    void onInnerDiameterChanged(const double& diameter);
    void onOuterDiameterChanged(const double& diameter);
    VectorXd calcInertia();
};

}


PipeBuilderWidget::PipeBuilderWidget()
{
    impl = new PipeBuilderWidgetImpl(this);
}


PipeBuilderWidgetImpl::PipeBuilderWidgetImpl(PipeBuilderWidget* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* pgbox = new QGridLayout();
    massSpin = new DoubleSpinBox();
    innerDiameterSpin = new DoubleSpinBox();
    outerDiameterSpin = new DoubleSpinBox();
    lengthSpin = new DoubleSpinBox();
    angleSpin = new SpinBox();
    stepSpin = new SpinBox();
    colorButton = new PushButton();

    massSpin->setValue(1.0);
    massSpin->setRange(0.001, 1000.0);
    massSpin->setSingleStep(0.01);
    massSpin->setDecimals(3);
    innerDiameterSpin->setValue(0.03);
    innerDiameterSpin->setRange(0.001, 1000.0);
    innerDiameterSpin->setSingleStep(0.01);
    innerDiameterSpin->setDecimals(3);
    outerDiameterSpin->setValue(0.05);
    outerDiameterSpin->setRange(0.001, 1000.0);
    outerDiameterSpin->setSingleStep(0.01);
    outerDiameterSpin->setDecimals(3);
    lengthSpin->setValue(1.0);
    lengthSpin->setRange(0.001, 1000.0);
    lengthSpin->setSingleStep(0.01);
    lengthSpin->setDecimals(3);
    angleSpin->setValue(0);
    angleSpin->setRange(0, 359);
    stepSpin->setValue(30);
    stepSpin->setRange(1, 120);

    int index = 0;
    pgbox->addWidget(new QLabel(_("Mass [kg]")), index, 0);
    pgbox->addWidget(massSpin, index, 1);
    pgbox->addWidget(new QLabel(_("Length [m]")), index, 2);
    pgbox->addWidget(lengthSpin, index++, 3);
    pgbox->addWidget(new QLabel(_("Inner diameter [m]")), index, 0);
    pgbox->addWidget(innerDiameterSpin, index, 1);
    pgbox->addWidget(new QLabel(_("Outer diameter [m]")), index, 2);
    pgbox->addWidget(outerDiameterSpin, index++, 3);
    pgbox->addWidget(new QLabel(_("Opening angle [deg]")), index, 0);
    pgbox->addWidget(angleSpin, index, 1);
    pgbox->addWidget(new QLabel(_("Step angle [deg]")), index, 2);
    pgbox->addWidget(stepSpin, index++, 3);
    pgbox->addWidget(new QLabel(_("Color [-]")), index, 0);
    pgbox->addWidget(colorButton, index++, 1);

    vbox->addLayout(pgbox);
    self->setLayout(vbox);

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    innerDiameterSpin->sigValueChanged().connect([&](double value){ onInnerDiameterChanged(value); });
    outerDiameterSpin->sigValueChanged().connect([&](double value){ onOuterDiameterChanged(value); });
}


PipeBuilderWidget::~PipeBuilderWidget()
{
    delete impl;
}


void PipeBuilderWidget::save(const string& filename)
{
    impl->writeYaml(filename);
}


void PipeBuilderWidgetImpl::writeYaml(const string& filename)
{
    filesystem::path path(filename);

    double mass = massSpin->value();
    double innerDiameter = innerDiameterSpin->value();
    double outerDiameter = outerDiameterSpin->value();
    double length = lengthSpin->value();
    int angle = angleSpin->value();
    int step = stepSpin->value();

    if(!filename.empty()) {
        YAMLWriter writer(filename);
        string name = path.stem();

        writer.startMapping(); // start of body map
        writer.putKeyValue("format", "ChoreonoidBody");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("angleUnit", "degree");
        writer.putKeyValue("name", name);
        writer.putKey("links");
        writer.startListing(); // start of links list
        writer.startMapping(); // start of links map
        writer.putKeyValue("name", name);
        writer.putKeyValue("jointType", "free");
        writer.putKey("centerOfMass");
        writer.startFlowStyleListing(); // start of centerOfMass list
        for(int i = 0; i < 3; ++i) {
            writer.putScalar(0.0);
        }
        writer.endListing(); // end of centerOfMass list
        writer.putKeyValue("mass", mass);
        writer.putKey("inertia");
        writer.startFlowStyleListing(); // start of inertia list
        VectorXd inertia;
        inertia.resize(9);
        inertia = calcInertia();
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
        writer.endListing(); // end of inertia list
        writer.putKey("elements");
        writer.startMapping(); // start of elements map
        writer.putKey("Shape");
        writer.startMapping(); // start of Shape map
        writer.putKey("geometry");
        writer.startMapping(); // start of geometry map
        writer.putKeyValue("type", "Extrusion");
        writer.putKey("crossSection");
        writer.startFlowStyleListing(); // start of crossSection list
        int range = 360 - angle;
        double sx;
        double sy;
        for(int i = 0; i <= range; i += step) {
            double x = outerDiameter * cos(i * TO_RADIAN);
            double y = outerDiameter * sin(i * TO_RADIAN);
            if(i == 0) {
                sx = x;
                sy = y;
            }
            writer.putScalar(x);
            writer.putScalar(y);
        }
        for(int i = 0; i <= range; i += step) {
            double x = innerDiameter * cos((range - i) * TO_RADIAN);
            double y = innerDiameter * sin((range - i) * TO_RADIAN);
            writer.putScalar(x);
            writer.putScalar(y);
        }
        writer.putScalar(sx);
        writer.putScalar(sy);
        writer.endListing(); // end of crossSection list
        writer.putKey("spine");
        writer.startFlowStyleListing(); // start of spine list
        Vector6 spine;
        spine << 0.0, -length / 2.0, 0.0, 0.0, length / 2.0, 0.0;
        for(int i = 0; i < 6; ++i) {
            writer.putScalar(spine[i]);
        }
        writer.endListing(); // end of spine list
        writer.endMapping(); // end of geometry map
        writer.putKey("appearance");
        writer.startFlowStyleMapping(); // start of appearance map
        writer.putKey("material");
        writer.startMapping(); // start of material map
        writer.putKey("diffuseColor");
        QPalette palette = colorButton->palette();
        QColor color = palette.color(QPalette::Button);
        double red = (double)color.red() / 255.0;
        double green = (double)color.green() / 255.0;
        double blue = (double)color.blue() / 255.0;
        Vector3 diffuseColor(red, green, blue);
        writer.startFlowStyleListing(); // start of diffuseColor list
        for(int i = 0; i < 3; ++i) {
            writer.putScalar(diffuseColor[i]);
        }
        writer.endListing(); // end of diffuseColor list
        writer.endMapping(); // end of material map
        writer.endMapping(); // end of appearance map
        writer.endMapping(); // end of Shape map
        writer.endMapping(); // end of elements map
        writer.endMapping(); // end of links map
        writer.endListing(); // end of links list
        writer.endMapping(); // end of body map
    }
}


void PipeBuilderWidgetImpl::onColorButtonClicked()
{
    QColor selectedColor;
    QColor currentColor = colorButton->palette().color(QPalette::Button);
    QColorDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Select a color"));
    dialog.setCurrentColor(currentColor);
    dialog.setOption (QColorDialog::DontUseNativeDialog);
    if(dialog.exec()) {
        selectedColor = dialog.currentColor();
    } else {
        selectedColor = currentColor;
    }

    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    colorButton->setPalette(palette);
}


void PipeBuilderWidgetImpl::onInnerDiameterChanged(const double& diameter)
{
    double outerDiameter = outerDiameterSpin->value();
    if(diameter >= outerDiameter) {
        double innerDiameter = outerDiameter - 0.01;
        innerDiameterSpin->setValue(innerDiameter);
    }
}


void PipeBuilderWidgetImpl::onOuterDiameterChanged(const double& diameter)
{
    double innerDiameter = innerDiameterSpin->value();
    if(diameter <= innerDiameter) {
        double outerDiameter = innerDiameter + 0.01;
        outerDiameterSpin->setValue(outerDiameter);
    }
}


VectorXd PipeBuilderWidgetImpl::calcInertia()
{
    VectorXd innerInertia, outerInertia;
    innerInertia.resize(9);
    outerInertia.resize(9);

    double length = lengthSpin->value();
    double innerRadius = innerDiameterSpin->value();
    double outerRadius = outerDiameterSpin->value();

    double innerRate = innerRadius * innerRadius / outerRadius * outerRadius;
    double outerRate = 1.0 - innerRate;

    {
        double mass = massSpin->value() * innerRate;
        double radius = innerRadius;
        double mainInertia = mass * radius * radius / 2.0;
        double subInertia = mass * (3.0 * radius * radius + length * length) / 12.0;
        double ix, iy, iz;
        ix = iz = subInertia;
        iy = mainInertia;
        innerInertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;
    }

    {
        double mass = massSpin->value() * outerRate;
        double radius = outerRadius;
        double mainInertia = mass * radius * radius / 2.0;
        double subInertia = mass * (3.0 * radius * radius + length * length) / 12.0;
        double ix, iy, iz;
        ix = iz = subInertia;
        iy = mainInertia;
        outerInertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;
    }
    VectorXd inertia = outerInertia - innerInertia;

    return inertia;
}
