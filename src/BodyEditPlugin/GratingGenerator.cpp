/**
   \file
   \author Kenta Suzuki
*/

#include "GratingGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
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

GratingGenerator* ggeneratorInstance = nullptr;

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

class GratingGeneratorImpl : public Dialog
{
public:
    GratingGeneratorImpl(GratingGenerator* self);
    GratingGenerator* self;

    enum DoubleSpinId {
        MASS, HEIGHT, FRAME_WDT,
        FRAME_HGT, GRID_WDT, GRID_HGT,
        NUM_DSPINS
    };
    enum SpinId { H_GRID, V_GRID, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];

    QLabel* sizeLabel;
    PushButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    bool save(const string& filename);
    void onColorButtonClicked();
    void onValueChanged();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();
};

}


GratingGenerator::GratingGenerator()
{
    impl = new GratingGeneratorImpl(this);
}


GratingGeneratorImpl::GratingGeneratorImpl(GratingGenerator* self)
    : self(self)
{
    setWindowTitle(_("Grating Builder"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;

    static const char* dlabels[] = {
        _("Mass [kg]"), _("Height [m]"), _("Frame width [m]"),
        _("Frame height [m]"), _("Grid width [m]"), _("Grid height [m]")
    };

    static const char* slabels[] = { _("Horizontal grid [-]"), _("Vertical grid [-]") };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
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
        spins[i] = new SpinBox;
        SpinBox* spin = spins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
        gbox->addWidget(new QLabel(slabels[i]), info.row, info.column - 1);
        gbox->addWidget(spin, info.row, info.column);
    }

    sizeLabel = new QLabel(_(" "));
    colorButton = new PushButton;

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

    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    onValueChanged();

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
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


void GratingGenerator::initializeClass(ExtensionManager* ext)
{
    if(!ggeneratorInstance) {
        ggeneratorInstance = ext->manage(new GratingGenerator);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("Grating"))->sigTriggered().connect(
                    [&](){ ggeneratorInstance->impl->show(); });
    }
}


bool GratingGeneratorImpl::save(const string& filename)
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


void GratingGeneratorImpl::onColorButtonClicked()
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


void GratingGeneratorImpl::onValueChanged()
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


MappingPtr GratingGeneratorImpl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(filename);
    string name = path.stem().string();

    node->write("format", "ChoreonoidBody");
    node->write("formatVersion", "1.0");
    node->write("angleUnit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;
    linksNode->append(writeLink());
    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
}


MappingPtr GratingGeneratorImpl::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = dspins[MASS]->value();

    node->write("name", "Root");
    node->write("jointType", "free");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", mass);
    write(node, "inertia", calcInertia());

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void GratingGeneratorImpl::writeLinkShape(Listing* elementsNode)
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
        Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");  

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
        MappingPtr materialNode = new Mapping;
        Listing& diffuseColorList = *materialNode->createFlowStyleListing("diffuseColor");
        QPalette palette = colorButton->palette();
        QColor color = palette.color(QPalette::Button);
        Vector3 c;
        c[0] = (double)color.red() / 255.0;
        c[1] = (double)color.green() / 255.0;
        c[2] = (double)color.blue() / 255.0;
        for(int i = 0; i < 3; ++i) {
            diffuseColorList.append(c[i], 3, 3);
        }
        appearanceNode->insert("material", materialNode);

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
        Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");  

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


VectorXd GratingGeneratorImpl::calcInertia()
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