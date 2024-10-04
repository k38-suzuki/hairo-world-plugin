/**
   @author Kenta Suzuki
*/

#include "GratingGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
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

GratingGenerator* gratingInstance = nullptr;

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
    { 0, 1, 0.01, 1000.0, 0.01, 3, 1.000 },
    { 0, 3, 0.01, 1000.0, 0.01, 3, 0.038 },
    { 2, 1, 0.01, 1000.0, 0.01, 3, 0.005 },
    { 2, 3, 0.01, 1000.0, 0.01, 3, 0.006 },
    { 3, 1, 0.01, 1000.0, 0.01, 3, 0.010 },
    { 3, 3, 0.01, 1000.0, 0.01, 3, 0.100 }
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
    { 1, 1, 0, 1000, 50 },
    { 1, 3, 0, 1000,  5 }
};

}

namespace cnoid {

class GratingGenerator::Impl : public Dialog
{
public:

    enum DoubleSpinId {
        MASS, HEIGHT, FRAME_WDT,
        FRAME_HGT, GRID_WDT, GRID_HGT,
        NUM_DSPINS
    };
    enum SpinId { H_GRID, V_GRID, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];

    QLabel* sizeLabel;
    ColorButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    Impl();

    bool save(const string& filename);
    void onColorButtonClicked();
    void onValueChanged();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();
};

}


void GratingGenerator::initializeClass(ExtensionManager* ext)
{
    if(!gratingInstance) {
        gratingInstance = ext->manage(new GratingGenerator);

        MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("Make a body file"));
        mm.addItem(_("Grating"))->sigTriggered().connect(
                    [&](){ gratingInstance->impl->show(); });
    }
}


GratingGenerator::GratingGenerator()
{
    impl = new Impl;
}


GratingGenerator::Impl::Impl()
    : Dialog()
{
    setWindowTitle(_("Grating Generator"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QGridLayout* gbox = new QGridLayout;

    static const char* label0[] = {
        _("Mass [kg]"), _("Height [m]"), _("Frame width [m]"),
        _("Frame height [m]"), _("Grid width [m]"), _("Grid height [m]")
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

    static const char* label1[] = { _("Horizontal grid [-]"), _("Vertical grid [-]") };

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        spins[i] = new SpinBox;
        SpinBox* spin = spins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
        gbox->addWidget(new QLabel(label1[i]), info.row, info.column - 1);
        gbox->addWidget(spin, info.row, info.column);
    }

    sizeLabel = new QLabel(_(" "));
    colorButton = new ColorButton;
    colorButton->setColor(Vector3(0.5, 0.5, 0.5));

    dspins[MASS]->setValue(1.0);
    dspins[FRAME_WDT]->setValue(0.005);
    dspins[FRAME_HGT]->setValue(0.006);
    dspins[GRID_WDT]->setValue(0.01);
    dspins[GRID_HGT]->setValue(0.1);
    spins[H_GRID]->setValue(50);
    spins[V_GRID]->setValue(5);
    dspins[HEIGHT]->setValue(0.038);

    gbox->addWidget(new QLabel(_("Color [-]")), 4, 0);
    gbox->addWidget(colorButton, 4, 1);
    gbox->addWidget(new QLabel(_("Size [m, m, m]")), 5, 0);
    gbox->addWidget(sizeLabel, 5, 1, 1, 3);

    formWidget = new FileFormWidget;

    auto vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    onValueChanged();

    dspins[FRAME_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[FRAME_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[GRID_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[GRID_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    spins[H_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    spins[V_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[HEIGHT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


GratingGenerator::~GratingGenerator()
{
    delete impl;
}


bool GratingGenerator::Impl::save(const string& filename)
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


void GratingGenerator::Impl::onValueChanged()
{
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();
    double height = dspins[HEIGHT]->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    QString size = QString::number(w, 'f', 3)
            + ", " + QString::number(h, 'f', 3)
            + ", " + QString::number(height, 'f', 3);
    sizeLabel->setText(size);
}


MappingPtr GratingGenerator::Impl::writeBody(const string& filename)
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


MappingPtr GratingGenerator::Impl::writeLink()
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


void GratingGenerator::Impl::writeLinkShape(Listing* elementsNode)
{
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();
    double height = dspins[HEIGHT]->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    double sx = -1.0 * w / 2.0;
    double sy = -1.0 * h / 2.0;
    VectorXd spine(6);
    spine << 0.0, 0.0, -height / 2.0, 0.0, 0.0, height / 2.0;

    MappingPtr visualNode = new Mapping;
    visualNode->write("type", "Visual");

    {
        ListingPtr elementsNode = new Listing;

        MappingPtr node = new Mapping;

        node->write("type", "Shape");

        MappingPtr geometryNode = new Mapping;
        geometryNode->write("type", "Extrusion");
        Listing& crossSectionList = *geometryNode->createFlowStyleListing("cross_section");

        int n = (verticalGrid * 8 + 14) * horizontalGrid + 18;

        crossSectionList.append(sx, 2, n);
        crossSectionList.append(sy, 2, n);

        for(int i = 0; i < horizontalGrid; ++i) {
            double x = sx + frameWidth + (frameWidth + gridWidth) * i;
            crossSectionList.append(                                    x - 0.0002, 2, n);
            crossSectionList.append(                                            sy, 2, n);
            crossSectionList.append(                                    x - 0.0002, 2, n);
            crossSectionList.append(sy + (frameHeight + gridHeight) * verticalGrid, 2, n);
            crossSectionList.append(                                    x - 0.0001, 2, n);
            crossSectionList.append(sy + (frameHeight + gridHeight) * verticalGrid, 2, n);
            crossSectionList.append(                                    x - 0.0001, 2, n);
            crossSectionList.append(                                            sy, 2, n);

            crossSectionList.append(x, 2, n);
            crossSectionList.append(sy, 2, n);

            for(int j = 0; j < verticalGrid; ++j) {
                double y = sy + (frameHeight + gridHeight) * j;
                crossSectionList.append(    x + gridWidth - 0.000001, 2, n);
                crossSectionList.append(                           y, 2, n);
                crossSectionList.append(    x + gridWidth - 0.000001, 2, n);
                crossSectionList.append(             y + frameHeight, 2, n);
                crossSectionList.append(                           x, 2, n);
                crossSectionList.append(             y + frameHeight, 2, n);
                crossSectionList.append(                           x, 2, n);
                crossSectionList.append(y + frameHeight + gridHeight, 2, n);
            }

            crossSectionList.append(x + gridWidth, 2, n);
            crossSectionList.append(sy + (frameHeight + gridHeight) * verticalGrid, 2, n);
            crossSectionList.append(x + gridWidth, 2, n);
            crossSectionList.append(sy, 2, n);
        }

        crossSectionList.append(-sx, 2, n);
        crossSectionList.append( sy, 2, n);
        crossSectionList.append(-sx, 2, n);
        crossSectionList.append(-sy, 2, n);
        crossSectionList.append( sx, 2, n);
        crossSectionList.append(-sy, 2, n);

        crossSectionList.append(                          sx, 2, n);
        crossSectionList.append(-sy - frameHeight + 0.000002, 2, n);
        crossSectionList.append(              -sx - 0.000001, 2, n);
        crossSectionList.append(-sy - frameHeight + 0.000002, 2, n);
        crossSectionList.append(              -sx - 0.000001, 2, n);
        crossSectionList.append(-sy - frameHeight + 0.000001, 2, n);
        crossSectionList.append(                          sx, 2, n);
        crossSectionList.append(-sy - frameHeight + 0.000001, 2, n);

        crossSectionList.append(sx, 2, n);
        crossSectionList.append(sy, 2, n);

        write(geometryNode, "spine", spine);

        node->insert("geometry", geometryNode);

        MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
        MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
        Listing& diffuseColorList = *materialNode->createFlowStyleListing("diffuse");
        Vector3 c = colorButton->color();
        for(int i = 0; i < 3; ++i) {
            diffuseColorList.append(c[i], 3, 3);
        }

        elementsNode->append(node);

        if(!elementsNode->empty()) {
            visualNode->insert("elements", elementsNode);
        }
    }

    MappingPtr collisionNode = new Mapping;
    collisionNode->write("type", "Collision");

    {
        ListingPtr elementsNode = new Listing;

        MappingPtr node = new Mapping;

        node->write("type", "Shape");

        MappingPtr geometryNode = new Mapping;
        geometryNode->write("type", "Extrusion");
        Listing& crossSectionList = *geometryNode->createFlowStyleListing("cross_section");

        int n = 10;

        crossSectionList.append( sx, 2, n);
        crossSectionList.append( sy, 2, n);
        crossSectionList.append(-sx, 2, n);
        crossSectionList.append( sy, 2, n);
        crossSectionList.append(-sx, 2, n);
        crossSectionList.append(-sy, 2, n);
        crossSectionList.append( sx, 2, n);
        crossSectionList.append(-sy, 2, n);
        crossSectionList.append( sx, 2, n);
        crossSectionList.append( sy, 2, n);

        write(geometryNode, "spine", spine);

        node->insert("geometry", geometryNode);

        elementsNode->append(node);

        if(!elementsNode->empty()) {
            collisionNode->insert("elements", elementsNode);
        }
    }

    elementsNode->append(visualNode);
    elementsNode->append(collisionNode);
}


VectorXd GratingGenerator::Impl::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);
    inertia << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

    double mass = dspins[MASS]->value();
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();

    double x = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double y = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;
    double z = dspins[HEIGHT]->value();

    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    inertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;

    return inertia;
}
