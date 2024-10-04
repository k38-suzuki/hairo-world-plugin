/**
   @author Kenta Suzuki
*/

#include "BentPipeGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include "ColorButton.h"
#include "FileFormWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

BentPipeGenerator* bentInstance = nullptr;

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
    { 0, 3, 0.01, 1000.0, 0.01, 3, 0.50 },
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
    { 2, 1, 1, 360, 90 },
    { 2, 3, 1, 120, 30 },
    { 3, 3, 1, 120, 30 }
};

}

namespace cnoid {

class BentPipeGenerator::Impl : public Dialog
{
public:

    enum DoubleSpinId { MASS, BENT_RAD, IN_DIA, OUT_DIA, NUM_DSPINS };
    enum SpinId { BENT_ANGLE, BENT_STEP, STEP, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];
    ColorButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    Impl();

    bool save(const string& filename);
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);

    void onBentAngleChanged(double value);
    void onInnerDiameterChanged(const double& diameter);
    void onOuterDiameterChanged(const double& diameter);
};

}


void BentPipeGenerator::initializeClass(ExtensionManager* ext)
{
    if(!bentInstance) {
        bentInstance = ext->manage(new BentPipeGenerator);

        MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("Make a body file"));
        mm.addItem(_("BentPipe"))->sigTriggered().connect(
                    [&](){ bentInstance->impl->show(); });
    }
}


BentPipeGenerator::BentPipeGenerator()
{
    impl = new Impl;
}


BentPipeGenerator::Impl::Impl()
    : Dialog()
{
    setWindowTitle(_("BentPipe Generator"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QGridLayout* gbox = new QGridLayout;

    const QStringList list = {
        _("Mass [kg]"), _("Bent radius [m]"),
        _("Inner diameter [m]"), _("Outer diameter [m]")
    };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        dspins[i]->setRange(info.min, info.max);
        dspins[i]->setSingleStep(info.step);
        dspins[i]->setDecimals(info.decimals);
        dspins[i]->setValue(info.value);
        gbox->addWidget(new QLabel(list[i]), info.row, info.column - 1);
        gbox->addWidget(dspins[i], info.row, info.column);
    }

    const QStringList list2 = {
        _("Bent angle [deg]"), _("Bent step angle [deg]"),
         _("Step angle [deg]")
    };

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        spins[i] = new SpinBox;
        spins[i]->setRange(info.min, info.max);
        spins[i]->setValue(info.value);
        gbox->addWidget(new QLabel(list2[i]), info.row, info.column - 1);
        gbox->addWidget(spins[i], info.row, info.column);
    }

    colorButton = new ColorButton;
    colorButton->setColor(Vector3(0.5, 0.5, 0.5));
    gbox->addWidget(new QLabel(_("Color [-]")), 3, 0);
    gbox->addWidget(colorButton, 3, 1);

    formWidget = new FileFormWidget;

    auto vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    formWidget->sigClicked().connect([&](string filename){ save(filename); });
    spins[BENT_ANGLE]->sigValueChanged().connect([&](double value){ onBentAngleChanged(value); });
    dspins[IN_DIA]->sigValueChanged().connect([&](double value){ onInnerDiameterChanged(value); });
    dspins[OUT_DIA]->sigValueChanged().connect([&](double value){ onOuterDiameterChanged(value); });
}


BentPipeGenerator::~BentPipeGenerator()
{
    delete impl;
}


bool BentPipeGenerator::Impl::save(const string& filename)
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


MappingPtr BentPipeGenerator::Impl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(fromUTF8(filename));
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


MappingPtr BentPipeGenerator::Impl::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = dspins[MASS]->value();

    node->write("name", "Root");
    node->write("joint_type", "fixed");
    write(node, "center_of_mass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", mass);
    // write(node, "inertia", calcInertia());

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void BentPipeGenerator::Impl::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double r_bent = dspins[BENT_RAD]->value();
    double d_in = dspins[IN_DIA]->value();
    double d_out = dspins[OUT_DIA]->value();
    double r_in = d_in / 2.0;
    double r_out = d_out / 2.0;

    int bent_angle = spins[BENT_ANGLE]->value();
    int bent_step = spins[BENT_STEP]->value();
    int step = spins[STEP]->value();

    double angle1 = bent_step * TO_RADIAN;
    double angle2 = step * TO_RADIAN;

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "IndexedFaceSet");
    Listing& verticesList = *geometryNode->createFlowStyleListing("vertices");

    int n1 = bent_angle / bent_step + 1;
    int n2 = 360 / step + 1;
    int n = n1 * n2 * 2 * 3;

    for(int i = 0; i < n1; ++i) {
        for(int j = 0; j < n2; ++j) {
            double x1 = (r_bent + r_in * cos(j * angle2)) * cos(i * angle1);
            double y1 = (r_bent + r_in * cos(j * angle2)) * sin(i * angle1);
            double z1 = r_in * sin(j * angle2);
            verticesList.append(x1, 3, n);
            verticesList.append(y1, 3, n);
            verticesList.append(z1, 3, n);
        }

        for(int j = 0; j < n2; ++j) {
            double x2 = (r_bent + r_out * cos(j * angle2)) * cos(i * angle1);
            double y2 = (r_bent + r_out * cos(j * angle2)) * sin(i * angle1);
            double z2 = r_out * sin(j * angle2);
            verticesList.append(x2, 3, n);
            verticesList.append(y2, 3, n);
            verticesList.append(z2, 3, n);
        }
    }

    Listing& facesList = *geometryNode->createFlowStyleListing("faces");

    int n3 = n1 * n2 * 3 * 5;

    for(int i = 0; i < n1; ++i) {
        for(int j = 0; j < n2 - 1; ++j) {
            int i0 = i * (n2 * 2) + j;
            int i1 = i * (n2 * 2) + j + 1;
            int i2 = i * (n2 * 2) + j + n2;
            int i3 = i * (n2 * 2) + j + n2 + 1;

            int i4 = (i + 1) * (n2 * 2) + j;
            int i5 = (i + 1) * (n2 * 2) + j + 1;
            int i6 = (i + 1) * (n2 * 2) + j + n2;
            int i7 = (i + 1) * (n2 * 2) + j + n2 + 1;

            if(i == 0) {
                facesList.append(i0, 5, n3);
                facesList.append(i2, 5, n3);
                facesList.append(i3, 5, n3);
                facesList.append(i1, 5, n3);
                facesList.append(-1, 5, n3);
            } else if(n1 - 1) {
                facesList.append(i0, 5, n3);
                facesList.append(i1, 5, n3);
                facesList.append(i3, 5, n3);
                facesList.append(i2, 5, n3);
                facesList.append(-1, 5, n3);
            }

            if(i < n1 - 1) {
                facesList.append(i0, 5, n3);
                facesList.append(i1, 5, n3);
                facesList.append(i5, 5, n3);
                facesList.append(i4, 5, n3);
                facesList.append(-1, 5, n3);

                facesList.append(i2, 5, n3);
                facesList.append(i6, 5, n3);
                facesList.append(i7, 5, n3);
                facesList.append(i3, 5, n3);
                facesList.append(-1, 5, n3);
            }
        }
    }

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    Listing& diffuseColorList = *materialNode->createFlowStyleListing("diffuse");
    Vector3 c = colorButton->color();
    for(int i = 0; i < 3; ++i) {
        diffuseColorList.append(c[i], 3, 3);
    }

    elementsNode->append(node);
}


void BentPipeGenerator::Impl::onBentAngleChanged(double value)
{
    double bent_step = spins[BENT_STEP]->value();
    if(value < bent_step) {
        spins[BENT_ANGLE]->setValue(bent_step);
    }
}


void BentPipeGenerator::Impl::onInnerDiameterChanged(const double& diameter)
{
    double d_out = dspins[OUT_DIA]->value();
    if(diameter >= d_out) {
        double d_in = d_out - 0.01;
        dspins[IN_DIA]->setValue(d_in);
    }
}


void BentPipeGenerator::Impl::onOuterDiameterChanged(const double& diameter)
{
    double d_in = dspins[IN_DIA]->value();
    if(diameter <= d_in) {
        double d_out = d_in + 0.01;
        dspins[OUT_DIA]->setValue(d_out);
    }
}