/**
   \file
   \author Kenta Suzuki
*/

#include "PipeBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

PipeBuilderDialog* pipeDialog = nullptr;

namespace {

struct DoubleSpinInfo
{
    int row;
    int column;
    double min;
    double max;
    double step;
    double decimals;
    double value;
};


DoubleSpinInfo doubleSpinInfo[] = {
    { 0, 1, 0.01, 1000.0, 0.01, 3, 1.00 },
    { 0, 3, 0.01, 1000.0, 0.01, 3, 1.00 },
    { 1, 1, 0.01, 1000.0, 0.01, 3, 0.03 },
    { 1, 3, 0.01, 1000.0, 0.01, 3, 0.05 }

};


struct SpinInfo
{
    int row;
    int column;
    int min;
    int max;
    int value;
};


SpinInfo spinInfo[] = {
    { 2, 1, 0, 359,  0 },
    { 2, 3, 1, 120, 30 }
};

}


namespace cnoid {

class PipeBuilderDialogImpl
{
public:
    PipeBuilderDialogImpl(PipeBuilderDialog* self);
    PipeBuilderDialog* self;

    enum DoubleSpinId { MASS, LENGTH, IN_DIA, OUT_DIA, NUM_DSPINS };

    enum SpinId { ANGLE, STEP, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];
    PushButton* colorButton;
    FileFormWidget* formWidget;

    bool writeYaml(const string& filename);
    void onColorButtonClicked();
    void onInnerDiameterChanged(const double& diameter);
    void onOuterDiameterChanged(const double& diameter);
    VectorXd calcInertia();
    void onAccepted();
    void onRejected();
};

}


PipeBuilderDialog::PipeBuilderDialog()
{
    impl = new PipeBuilderDialogImpl(this);
}


PipeBuilderDialogImpl::PipeBuilderDialogImpl(PipeBuilderDialog* self)
    : self(self)
{
    self->setWindowTitle(_("Pipe Builder"));
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();

    const char* dlabels[] = { _("Mass [kg]"), _("Length [m]"),
                              _("Inner diameter [m]"), _("Outer diameter [m]")
                            };

    const char* slabels[] = { _("Opening angle [deg]"), _("Step angle [deg]") };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox();
        DoubleSpinBox* dspin = dspins[i];
        dspin->setRange(info.min, info.max);
        dspin->setSingleStep(info.step);
        dspin->setDecimals(info.decimals);
        dspin->setValue(info.value);
        gbox->addWidget(new QLabel(dlabels[i]), info.row, info.column - 1);
        gbox->addWidget(dspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        spins[i] = new SpinBox();
        SpinBox* spin = spins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
        gbox->addWidget(new QLabel(slabels[i]), info.row, info.column - 1);
        gbox->addWidget(spin, info.row, info.column);
    }

    colorButton = new PushButton();
    gbox->addWidget(new QLabel(_("Color [-]")), 3, 0);
    gbox->addWidget(colorButton, 3, 1);

    formWidget = new FileFormWidget();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    PushButton* okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);

    vbox->addLayout(gbox);
    vbox->addWidget(formWidget);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));
    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    dspins[IN_DIA]->sigValueChanged().connect([&](double value){ onInnerDiameterChanged(value); });
    dspins[OUT_DIA]->sigValueChanged().connect([&](double value){ onOuterDiameterChanged(value); });
    formWidget->sigClicked().connect([&](string filename){ pipeDialog->save(filename); });
}


PipeBuilderDialog::~PipeBuilderDialog()
{
    delete impl;
}


PipeBuilderDialog* PipeBuilderDialog::instance()
{
    if(!pipeDialog) {
        pipeDialog = new PipeBuilderDialog();
    }
    return pipeDialog;
}


bool PipeBuilderDialog::save(const string& filename)
{
    return impl->writeYaml(filename);
}


bool PipeBuilderDialogImpl::writeYaml(const string& filename)
{
    filesystem::path path(filename);

    double mass = dspins[MASS]->value();
    double innerDiameter = dspins[IN_DIA]->value();
    double outerDiameter = dspins[OUT_DIA]->value();
    double length = dspins[LENGTH]->value();
    int angle = spins[ANGLE]->value();
    int step = spins[STEP]->value();

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
    return true;
}


void PipeBuilderDialogImpl::onColorButtonClicked()
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


void PipeBuilderDialogImpl::onInnerDiameterChanged(const double& diameter)
{
    double outerDiameter = dspins[OUT_DIA]->value();
    if(diameter >= outerDiameter) {
        double innerDiameter = outerDiameter - 0.01;
        dspins[IN_DIA]->setValue(innerDiameter);
    }
}


void PipeBuilderDialogImpl::onOuterDiameterChanged(const double& diameter)
{
    double innerDiameter = dspins[IN_DIA]->value();
    if(diameter <= innerDiameter) {
        double outerDiameter = innerDiameter + 0.01;
        dspins[OUT_DIA]->setValue(outerDiameter);
    }
}


VectorXd PipeBuilderDialogImpl::calcInertia()
{
    VectorXd innerInertia, outerInertia;
    innerInertia.resize(9);
    outerInertia.resize(9);

    double length = dspins[LENGTH]->value();
    double innerRadius = dspins[IN_DIA]->value();
    double outerRadius = dspins[OUT_DIA]->value();

    double innerRate = innerRadius * innerRadius / outerRadius * outerRadius;
    double outerRate = 1.0 - innerRate;

    {
        double mass = dspins[MASS]->value() * innerRate;
        double radius = innerRadius;
        double mainInertia = mass * radius * radius / 2.0;
        double subInertia = mass * (3.0 * radius * radius + length * length) / 12.0;
        double ix, iy, iz;
        ix = iz = subInertia;
        iy = mainInertia;
        innerInertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;
    }

    {
        double mass = dspins[MASS]->value() * outerRate;
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


void PipeBuilderDialog::onAccepted()
{
    impl->onAccepted();
}


void PipeBuilderDialogImpl::onAccepted()
{

}


void PipeBuilderDialog::onRejected()
{
    impl->onRejected();
}


void PipeBuilderDialogImpl::onRejected()
{

}
