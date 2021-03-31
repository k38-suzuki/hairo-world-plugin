/**
   \file
   \author Kenta Suzuki
*/

#include "GratingBuilderWidget.h"
#include <cnoid/Button>
#include <cnoid/EigenTypes>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class GratingBuilderWidgetImpl
{
public:
    GratingBuilderWidgetImpl(GratingBuilderWidget* self);
    GratingBuilderWidget* self;

    DoubleSpinBox* massSpin;
    SpinBox* mainHeightSpin;
    SpinBox* mainPitchSpin;
    SpinBox* mainThicknessSpin;
    SpinBox* twistPitchSpin;
    PushButton* colorButton;

    void writeYaml(const string& filename);
    void onColorButtonClicked();
    VectorXd calcInertia();
};

}


GratingBuilderWidget::GratingBuilderWidget()
{
    impl = new GratingBuilderWidgetImpl(this);
}


GratingBuilderWidgetImpl::GratingBuilderWidgetImpl(GratingBuilderWidget* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();

    massSpin = new DoubleSpinBox();
    mainHeightSpin = new SpinBox();
    mainPitchSpin = new SpinBox();
    mainThicknessSpin = new SpinBox();
    twistPitchSpin = new SpinBox();
    colorButton = new PushButton();

    massSpin->setValue(1.0);
    massSpin->setRange(0.001, 1000.0);
    massSpin->setSingleStep(0.01);
    mainHeightSpin->setValue(1.0);
    mainHeightSpin->setRange(1, 1000);
    mainHeightSpin->setSingleStep(1);
    mainPitchSpin->setValue(1.0);
    mainPitchSpin->setRange(1, 1000);
    mainPitchSpin->setSingleStep(1);
    mainThicknessSpin->setValue(1.0);
    mainThicknessSpin->setRange(1, 1000);
    mainThicknessSpin->setSingleStep(1);
    twistPitchSpin->setValue(1.0);
    twistPitchSpin->setRange(1, 1000);
    twistPitchSpin->setSingleStep(1);

    int index = 0;
    gbox->addWidget(new QLabel(_("Mass [kg]")), index, 0);
    gbox->addWidget(massSpin, index, 1);
    gbox->addWidget(new QLabel(_("Height [mm]")), index, 2);
    gbox->addWidget(mainHeightSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Main-bar pitch [mm]")), index, 0);
    gbox->addWidget(mainPitchSpin, index, 1);
    gbox->addWidget(new QLabel(_("Main-bar thickness [mm]")), index, 2);
    gbox->addWidget(mainThicknessSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Twist-bar pitch [mm]")), index, 0);
    gbox->addWidget(twistPitchSpin, index++, 1);
    gbox->addWidget(new QLabel(_("Color [-]")), index, 0);
    gbox->addWidget(colorButton, index++, 1);

    vbox->addLayout(gbox);
    self->setLayout(vbox);

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
}


GratingBuilderWidget::~GratingBuilderWidget()
{
    delete impl;
}


void GratingBuilderWidget::save(const string& filename)
{
    impl->writeYaml(filename);
}


void GratingBuilderWidgetImpl::writeYaml(const string& filename)
{
    filesystem::path path(filename);

    double mass = massSpin->value();
    double mainHeight = mainHeightSpin->value() * 0.001;
    double mainPitch = mainPitchSpin->value() * 0.001;
    double mainThickness = mainThicknessSpin->value() * 0.001;
    double twistPitch = twistPitchSpin->value() * 0.001;

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
        writer.putKey("Visual");
        writer.startMapping(); // start of Shape map
        writer.putKey("geometry");
        writer.startMapping(); // start of geometry map
        writer.putKeyValue("type", "Extrusion");
        writer.putKey("crossSection");
        writer.startFlowStyleListing(); // start of crossSection list

        writer.putScalar(0); writer.putScalar(0);
        writer.putScalar(1); writer.putScalar(0);
        writer.putScalar(1); writer.putScalar(1);
        writer.putScalar(0); writer.putScalar(1);
        writer.putScalar(0); writer.putScalar(0);

        writer.endListing(); // end of crossSection list
        writer.putKey("spine");
        writer.startFlowStyleListing(); // start of spine list
        Vector6 spine;
        spine << 0.0, 0.0, -mainHeight / 2.0, 0.0, 0.0, mainHeight / 2.0;
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


void GratingBuilderWidgetImpl::onColorButtonClicked()
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


VectorXd GratingBuilderWidgetImpl::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);

    return inertia;
}
