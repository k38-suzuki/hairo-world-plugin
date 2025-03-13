/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/Button>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
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
    { 0, 1, 0.001, 1000.0, 0.01, 3, 0.15,     "tread", nullptr },
    { 0, 3, 0.001, 1000.0, 0.01, 3, 0.75,     "width", nullptr },
    { 1, 1, 0.001, 1000.0, 0.01, 3, 0.23,     "riser", nullptr },
    { 1, 3, 0.001, 1000.0, 0.01, 3, 0.05,  "stringer", nullptr },
    { 2, 1, 0.001, 1000.0, 0.01, 3, 0.02, "thickness", nullptr }
};

class StairsCreatorWidget : public QWidget
{
public:
    StairsCreatorWidget(QWidget* parent = nullptr);

private:
    void reset();
    bool save(const string& filename);

    void onColorButtonClicked();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    void writeStringerShape(Listing* elementsNode);
    void writeStepShape(Listing* elementsNode);

    enum {
        TREAD, WIDTH, RISER,
        STRINGER, THICKNESS,
        NumDoubleSpinBoxes
    };

    DoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    SpinBox* stepsSpinBox;
    ColorButton* colorButton;
    YAMLWriter yamlWriter;
};

}


void StairsCreator::initializeClass(ExtensionManager* ext)
{
    static StairsCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new StairsCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Stairs"), widget);
    }
}


StairsCreator::StairsCreator()
{

}


StairsCreator::~StairsCreator()
{

}


StairsCreatorWidget::StairsCreatorWidget(QWidget* parent)
    : QWidget(parent)
{
    yamlWriter.setKeyOrderPreservationMode(true);

    auto gridLayout = new QGridLayout;

    const QStringList list = {
        _("Tread [m]"), _("Stair width [m]"),
        _("Riser [m]"), _("Width of stringer [m]"),
        _("Tread thickness [m]")
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

    stepsSpinBox = new SpinBox;
    stepsSpinBox->setRange(1, 9999);
    stepsSpinBox->setValue(10);
    gridLayout->addWidget(new QLabel(_("Number of steps [-]")), 2, 2);
    gridLayout->addWidget(stepsSpinBox, 2, 3);

    colorButton = new ColorButton;
    colorButton->setColor(Vector3(0.5, 0.5, 0.5));
    gridLayout->addWidget(new QLabel(_("Color [-]")), 3, 0);
    gridLayout->addWidget(colorButton, 3, 1);

    auto toolBar = new CreatorToolBar;
    toolBar->sigNewRequested().connect([&](){ reset(); });
    toolBar->sigSaveRequested().connect([&](const string& filename){ save(filename); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("Stairs Generator"));
}


void StairsCreatorWidget::reset()
{
    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = doubleSpinInfo[i];
        info.spin = doubleSpinBoxes[i];
        info.spin->setValue(info.value);
    }

    stepsSpinBox->setValue(10);

    colorButton->setColor(Vector3(0.5, 0.5, 0.5));
}


bool StairsCreatorWidget::save(const string& filename)
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


MappingPtr StairsCreatorWidget::writeBody(const string& filename)
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


MappingPtr StairsCreatorWidget::writeLink()
{
    MappingPtr node = new Mapping;

    node->write("name", "Root");
    write(node, "translation", Vector3(0.0, 0.0, 0.0));
    node->write("joint_type", "fixed");
    node->write("material", "Ground");
    node->write("AMOR", true);

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void StairsCreatorWidget::writeLinkShape(Listing* elementsNode)
{
    double tread = doubleSpinBoxes[TREAD]->value();
    double width = doubleSpinBoxes[WIDTH]->value();
    double riser = doubleSpinBoxes[RISER]->value();
    double stringer = doubleSpinBoxes[STRINGER]->value();
    double steps = stepsSpinBox->value();

    double y = (width + stringer) / 2.0;
    ListingPtr elementsNode1 = new Listing;
    writeStringerShape(elementsNode1);

    for(int i = 0; i < 2; ++i) {
        MappingPtr node = new Mapping;
        if(i != 0) {
            node->setFlowStyle(true);
        }

        node->write("type", "Transform");
        write(node, "translation", Vector3(0.0, y * (1 - i * 2), 0.0));

        if(!elementsNode1->empty()) {
            node->insert("elements", elementsNode1);
        }

        elementsNode->append(node);
    }

    double depth = tread * steps;
    ListingPtr elementsNode2 = new Listing;
    writeStepShape(elementsNode2);

    for(int i = 0; i < steps; ++i) {
        MappingPtr node = new Mapping;
        if(i != 0) {
            node->setFlowStyle(true);
        }

        node->write("type", "Transform");
        write(node, "translation", Vector3(-depth / 2.0 + tread * i, 0.0, riser * (i + 1)));

        if(!elementsNode2->empty()) {
            node->insert("elements", elementsNode2);
        }

        elementsNode->append(node);
    }

}


void StairsCreatorWidget::writeStringerShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double tread = doubleSpinBoxes[TREAD]->value();
    double riser = doubleSpinBoxes[RISER]->value();
    double stringer = doubleSpinBoxes[STRINGER]->value();
    double steps = stepsSpinBox->value();

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("cross_section");

    int n = 10;
    double depth = tread * steps;
    double height = riser * steps;
    double hw = stringer / 2.0;

    crossSectionList.append(  -depth / 2.0, 2, n);
    crossSectionList.append(           0.0, 2, n);
    crossSectionList.append(   depth / 2.0, 2, n);
    crossSectionList.append(        height, 2, n);
    crossSectionList.append(   depth / 2.0, 2, n);
    crossSectionList.append(height + riser, 2, n);
    crossSectionList.append(  -depth / 2.0, 2, n);
    crossSectionList.append(         riser, 2, n);
    crossSectionList.append(  -depth / 2.0, 2, n);
    crossSectionList.append(           0.0, 2, n);

    VectorXd spine(6);
    spine << 0.0, -hw, 0.0, 0.0, hw, 0.0;
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
}


void StairsCreatorWidget::writeStepShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double tread = doubleSpinBoxes[TREAD]->value();
    double width = doubleSpinBoxes[WIDTH]->value();
    double thickness = doubleSpinBoxes[THICKNESS]->value();

    node->write("type", "Shape");
    write(node, "translation", Vector3(tread / 2.0, 0.0, -thickness / 2.0));

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Box");
    write(geometryNode, "size", Vector3(tread, width, thickness));

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    Listing& diffuseColorList = *materialNode->createFlowStyleListing("diffuse");
    Vector3 c = colorButton->color();
    for(int i = 0; i < 3; ++i) {
        diffuseColorList.append(c[i], 3, 3);
    }

    elementsNode->append(node);
}