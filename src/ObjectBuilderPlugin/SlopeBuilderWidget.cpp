/**
   \file
   \author Kenta Suzuki
*/

#include "SlopeBuilderWidget.h"
#include <cnoid/Button>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

struct DoubleSpinInfo
{
    int row;
    int column;
    double min;
    double max;
    double step;
    int decimals;
    double value;
};


DoubleSpinInfo doubleSpinInfo[] = {
    { 0, 1, 0.001, 1000.0, 0.01, 3, 1.0 },
    { 0, 3, 0.001, 1000.0, 0.01, 3, 1.0 },
    { 1, 1, 0.001, 1000.0, 0.01, 3, 1.0 },
    { 1, 3, 0.001, 1000.0, 0.01, 3, 1.0 }
};

}

namespace cnoid {

class SlopeBuilderWidgetImpl
{
public:
    SlopeBuilderWidgetImpl(SlopeBuilderWidget* self);
    SlopeBuilderWidget* self;

    enum DoubleSpinId {
        MASS, WIDTH, HEIGHT,
        LENGTH, NUM_DSPINS
    };

    DoubleSpinBox* dspins[NUM_DSPINS];
    PushButton* colorButton;

    void writeYaml(const string& filename);
    void onColorButtonClicked();
    VectorXd calcInertia();
};

}


SlopeBuilderWidget::SlopeBuilderWidget()
{
    impl = new SlopeBuilderWidgetImpl(this);
}


SlopeBuilderWidgetImpl::SlopeBuilderWidgetImpl(SlopeBuilderWidget* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();

    const char* dlabels[] = {
        _("Mass [kg]"),  _("Width [m]"),
        _("Height [m]"), _("Length [m]")
    };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox();
        dspins[i]->setRange(info.min, info.max);
        dspins[i]->setSingleStep(info.step);
        dspins[i]->setDecimals(info.decimals);
        dspins[i]->setValue(info.value);
        gbox->addWidget(new QLabel(dlabels[i]), info.row, info.column - 1);
        gbox->addWidget(dspins[i], info.row, info.column);
    }

    colorButton = new PushButton();
    gbox->addWidget(new QLabel(_("Color [-]")), 2, 0);
    gbox->addWidget(colorButton, 2, 1);

    vbox->addLayout(gbox);
    self->setLayout(vbox);

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
}


SlopeBuilderWidget::~SlopeBuilderWidget()
{
    delete impl;
}


void SlopeBuilderWidget::save(const string& filename)
{
    impl->writeYaml(filename);
}


void SlopeBuilderWidgetImpl::writeYaml(const string& filename)
{
    filesystem::path path(filename);

    double mass = dspins[MASS]->value();
    double length = dspins[LENGTH]->value();
    double width = dspins[WIDTH]->value();
    double height = dspins[HEIGHT]->value();

    if(!filename.empty()) {
        YAMLWriter writer(filename);
        string name = path.stem().string();

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

        double hl = length / 2.0;
        double hw = width / 2.0;
        double hh = height / 2.0;

        writer.putScalar(-hl);
        writer.putScalar(-hh);
        writer.putScalar(hl);
        writer.putScalar(-hh);
        writer.putScalar(hl);
        writer.putScalar(hh);
        writer.putScalar(-hl);
        writer.putScalar(-hh);

        writer.endListing(); // end of crossSection list
        writer.putKey("spine");
        writer.startFlowStyleListing(); // start of spine list
        Vector6 spine;
        spine << 0.0, -hw, 0.0, 0.0, hw, 0.0;
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


void SlopeBuilderWidgetImpl::onColorButtonClicked()
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


VectorXd SlopeBuilderWidgetImpl::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);
    inertia << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

    double mass = dspins[MASS]->value();
    double x = dspins[LENGTH]->value();
    double y = dspins[WIDTH]->value();
    double z = dspins[HEIGHT]->value();

    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    inertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;

    return inertia;
}
