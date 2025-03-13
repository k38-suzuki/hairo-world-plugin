/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/Button>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/ExtensionManager>
#include <cnoid/MenuManager>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include "BodyCreatorDialog.h"
#include "ColorButton.h"
#include "CreatorToolBar.h"
#include "WidgetInfo.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

DoubleSpinInfo doubleSpinInfo[] = {
    { 0, 1, 0.01, 1000.0, 0.01, 3, 1.000,         "mass", nullptr },
    { 0, 3, 0.01, 1000.0, 0.01, 3, 0.038,       "height", nullptr },
    { 2, 1, 0.01, 1000.0, 0.01, 3, 0.005,  "frame_width", nullptr },
    { 2, 3, 0.01, 1000.0, 0.01, 3, 0.006, "frame_height", nullptr },
    { 3, 1, 0.01, 1000.0, 0.01, 3, 0.010,   "grid_width", nullptr },
    { 3, 3, 0.01, 1000.0, 0.01, 3, 0.100,  "grid_height", nullptr }
};

SpinInfo spinInfo[] = {
    { 1, 1, 0, 1000, 1, 50, "horizontal_grid", nullptr },
    { 1, 3, 0, 1000, 1,  5,   "vertical_grid", nullptr }
};

class GratingCreatorWidget : public QWidget
{
public:
    GratingCreatorWidget(QWidget* parent = nullptr);

private:
    void reset();
    bool save(const string& filename);

    void onColorButtonClicked();
    void onValueChanged();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();

    enum {
        MASS, HEIGHT, FRAME_WDT,
        FRAME_HGT, GRID_WDT, GRID_HGT,
        NumDoubleSpinBoxes
    };
    enum { H_GRID, V_GRID, NumSpinBoxes };

    DoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    SpinBox* spinBoxes[NumSpinBoxes];

    QLabel* sizeLabel;
    ColorButton* colorButton;
    YAMLWriter yamlWriter;
};

}


void GratingCreator::initializeClass(ExtensionManager* ext)
{
    static GratingCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new GratingCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Grating"), widget);
    }
}


GratingCreator::GratingCreator()
{

}


GratingCreator::~GratingCreator()
{

}


GratingCreatorWidget::GratingCreatorWidget(QWidget* parent)
    : QWidget(parent)
{
    yamlWriter.setKeyOrderPreservationMode(true);

    auto gridLayout = new QGridLayout;

    const QStringList list = {
        _("Mass [kg]"), _("Height [m]"), _("Frame width [m]"),
        _("Frame height [m]"), _("Grid width [m]"), _("Grid height [m]")
    };

    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = doubleSpinInfo[i];
        info.spin = doubleSpinBoxes[i] = new DoubleSpinBox;
        info.spin->setRange(info.min, info.max);
        info.spin->setSingleStep(info.step);
        info.spin->setDecimals(info.decimals);
        info.spin->setValue(info.value);
        gridLayout->addWidget(new QLabel(list[i]), info.row, info.column - 1);
        gridLayout->addWidget(info.spin, info.row, info.column);
    }
    doubleSpinBoxes[HEIGHT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    doubleSpinBoxes[FRAME_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    doubleSpinBoxes[FRAME_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    doubleSpinBoxes[GRID_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    doubleSpinBoxes[GRID_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });

    const QStringList list2 = { _("Horizontal grid [-]"), _("Vertical grid [-]") };

    for(int i = 0; i < NumSpinBoxes; ++i) {
        SpinInfo& info = spinInfo[i];
        info.spin = spinBoxes[i] = new SpinBox;        
        info.spin->setRange(info.min, info.max);
        info.spin->setValue(info.value);
        gridLayout->addWidget(new QLabel(list2[i]), info.row, info.column - 1);
        gridLayout->addWidget(info.spin, info.row, info.column);
    }
    spinBoxes[H_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    spinBoxes[V_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });

    sizeLabel = new QLabel(_(" "));
    colorButton = new ColorButton;
    colorButton->setColor(Vector3(0.5, 0.5, 0.5));

    gridLayout->addWidget(new QLabel(_("Color [-]")), 4, 0);
    gridLayout->addWidget(colorButton, 4, 1);
    gridLayout->addWidget(new QLabel(_("Size [m, m, m]")), 5, 0);
    gridLayout->addWidget(sizeLabel, 5, 1, 1, 3);

    onValueChanged();

    auto toolBar = new CreatorToolBar;
    toolBar->sigNewRequested().connect([&](){ reset(); });
    toolBar->sigSaveRequested().connect([&](const string& filename){ save(filename); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("Grating Generator"));
}


void GratingCreatorWidget::reset()
{
    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = doubleSpinInfo[i];
        info.spin = doubleSpinBoxes[i];
        info.spin->setValue(info.value);
    }

    for(int i = 0; i < NumSpinBoxes; ++i) {
        SpinInfo& info = spinInfo[i];
        info.spin = spinBoxes[i];
        info.spin->setValue(info.value);
    }

    colorButton->setColor(Vector3(0.5, 0.5, 0.5));
}


bool GratingCreatorWidget::save(const string& filename)
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


void GratingCreatorWidget::onValueChanged()
{
    double frameWidth = doubleSpinBoxes[FRAME_WDT]->value();
    double frameHeight = doubleSpinBoxes[FRAME_HGT]->value();
    double gridWidth = doubleSpinBoxes[GRID_WDT]->value();
    double gridHeight = doubleSpinBoxes[GRID_HGT]->value();
    int horizontalGrid = spinBoxes[H_GRID]->value();
    int verticalGrid = spinBoxes[V_GRID]->value();
    double height = doubleSpinBoxes[HEIGHT]->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    QString size = QString::number(w, 'f', 3)
            + ", " + QString::number(h, 'f', 3)
            + ", " + QString::number(height, 'f', 3);
    sizeLabel->setText(size);
}


MappingPtr GratingCreatorWidget::writeBody(const string& filename)
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


MappingPtr GratingCreatorWidget::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = doubleSpinBoxes[MASS]->value();

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


void GratingCreatorWidget::writeLinkShape(Listing* elementsNode)
{
    double frameWidth = doubleSpinBoxes[FRAME_WDT]->value();
    double frameHeight = doubleSpinBoxes[FRAME_HGT]->value();
    double gridWidth = doubleSpinBoxes[GRID_WDT]->value();
    double gridHeight = doubleSpinBoxes[GRID_HGT]->value();
    int horizontalGrid = spinBoxes[H_GRID]->value();
    int verticalGrid = spinBoxes[V_GRID]->value();
    double height = doubleSpinBoxes[HEIGHT]->value();

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


VectorXd GratingCreatorWidget::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);
    inertia << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

    double mass = doubleSpinBoxes[MASS]->value();
    double frameWidth = doubleSpinBoxes[FRAME_WDT]->value();
    double frameHeight = doubleSpinBoxes[FRAME_HGT]->value();
    double gridWidth = doubleSpinBoxes[GRID_WDT]->value();
    double gridHeight = doubleSpinBoxes[GRID_HGT]->value();
    int horizontalGrid = spinBoxes[H_GRID]->value();
    int verticalGrid = spinBoxes[V_GRID]->value();

    double x = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double y = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;
    double z = doubleSpinBoxes[HEIGHT]->value();

    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    inertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;

    return inertia;
}