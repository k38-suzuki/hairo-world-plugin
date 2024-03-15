/**
   @author Kenta Suzuki
*/

#include "PipeGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

PipeGenerator* instance_ = nullptr;

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

class PipeGenerator::Impl : public Dialog
{
public:
    PipeGenerator* self;

    Impl(PipeGenerator* self);

    enum DoubleSpinId { MASS, LENGTH, IN_DIA, OUT_DIA, NUM_DSPINS };
    enum SpinId { ANGLE, STEP, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];
    PushButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    bool save(const string& filename);
    void onColorButtonClicked();
    void onInnerDiameterChanged(const double& diameter);
    void onOuterDiameterChanged(const double& diameter);
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();
};

}


PipeGenerator::PipeGenerator()
{
    impl = new Impl(this);
}


PipeGenerator::Impl::Impl(PipeGenerator* self)
    : self(self)
{
    setWindowTitle(_("Pipe Generator"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QGridLayout* gbox = new QGridLayout;

    static const char* label0[] = { _("Mass [kg]"), _("Length [m]"),
                              _("Inner diameter [m]"), _("Outer diameter [m]")
                            };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        dspin->setRange(info.min, info.max);
        dspin->setSingleStep(info.step);
        dspin->setDecimals(info.decimals);
        dspin->setValue(info.value);
        gbox->addWidget(new QLabel(label0[i]), info.row, info.column - 1);
        gbox->addWidget(dspin, info.row, info.column);
    }

    static const char* label1[] = { _("Opening angle [deg]"), _("Step angle [deg]") };

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        spins[i] = new SpinBox;
        SpinBox* spin = spins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
        gbox->addWidget(new QLabel(label1[i]), info.row, info.column - 1);
        gbox->addWidget(spin, info.row, info.column);
    }

    colorButton = new PushButton;
    gbox->addWidget(new QLabel(_("Color [-]")), 3, 0);
    gbox->addWidget(colorButton, 3, 1);

    formWidget = new FileFormWidget;

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    dspins[IN_DIA]->sigValueChanged().connect([&](double value){ onInnerDiameterChanged(value); });
    dspins[OUT_DIA]->sigValueChanged().connect([&](double value){ onOuterDiameterChanged(value); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


PipeGenerator::~PipeGenerator()
{
    delete impl;
}


void PipeGenerator::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new PipeGenerator);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("Pipe"))->sigTriggered().connect(
                    [&](){ instance_->impl->show(); });
    }
}


bool PipeGenerator::Impl::save(const string& filename)
{
    if(!filename.empty()) {
        auto topNode = writeBody(filename);
        if(yamlWriter.openFile(filename)) {
            yamlWriter.putNode(topNode);
            yamlWriter.closeFile();
        }
    }

    return true;
}


void PipeGenerator::Impl::onColorButtonClicked()
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


void PipeGenerator::Impl::onInnerDiameterChanged(const double& diameter)
{
    double outerDiameter = dspins[OUT_DIA]->value();
    if(diameter >= outerDiameter) {
        double innerDiameter = outerDiameter - 0.01;
        dspins[IN_DIA]->setValue(innerDiameter);
    }
}


void PipeGenerator::Impl::onOuterDiameterChanged(const double& diameter)
{
    double innerDiameter = dspins[IN_DIA]->value();
    if(diameter <= innerDiameter) {
        double outerDiameter = innerDiameter + 0.01;
        dspins[OUT_DIA]->setValue(outerDiameter);
    }
}


MappingPtr PipeGenerator::Impl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(filename);
    string name = path.stem().string();

    node->write("format", "ChoreonoidBody");
    node->write("format_version", "2.0");
    node->write("angle_unit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;
    linksNode->append(writeLink());
    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
}


MappingPtr PipeGenerator::Impl::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = dspins[MASS]->value();

    node->write("name", "Root");
    node->write("joint_type", "free");
    write(node, "center_of_mass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", mass);
    write(node, "inertia", calcInertia());

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void PipeGenerator::Impl::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double innerDiameter = dspins[IN_DIA]->value();
    double outerDiameter = dspins[OUT_DIA]->value();
    double length = dspins[LENGTH]->value();
    int angle = spins[ANGLE]->value();
    int step = spins[STEP]->value();

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("cross_section");

    int range = 360 - angle;
    int n = ((360 - angle) / step + 1) * 2 + 1;
    double sx;
    double sy;
    for(int i = 0; i <= range; i += step) {
        double x = outerDiameter * cos(i * TO_RADIAN);
        double y = outerDiameter * sin(i * TO_RADIAN);
        if(i == 0) {
            sx = x;
            sy = y;
        }
        crossSectionList.append(x, 2, n);
        crossSectionList.append(y, 2, n);
    }
    for(int i = 0; i <= range; i += step) {
        double x = innerDiameter * cos((range - i) * TO_RADIAN);
        double y = innerDiameter * sin((range - i) * TO_RADIAN);
        crossSectionList.append(x, 2, n);
        crossSectionList.append(y, 2, n);
    }
    crossSectionList.append(sx, 2, n);
    crossSectionList.append(sy, 2, n);

    VectorXd spine(6);
    spine << 0.0, -length / 2.0, 0.0, 0.0, length / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    Listing& diffuseColorList = *materialNode->createFlowStyleListing("diffuse");
    QPalette palette = colorButton->palette();
    QColor color = palette.color(QPalette::Button);
    Vector3 c;
    c[0] = (double)color.red() / 255.0;
    c[1] = (double)color.green() / 255.0;
    c[2] = (double)color.blue() / 255.0;
    for(int i = 0; i < 3; ++i) {
        diffuseColorList.append(c[i], 3, 3);
    }

    elementsNode->append(node);
}


VectorXd PipeGenerator::Impl::calcInertia()
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
