/**
   \file
   \author Kenta Suzuki
*/

#include "StairsGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
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

StairsGenerator* instance_ = nullptr;

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
    { 0, 1, 0.001, 1000.0, 0.01, 3, 0.15 },
    { 0, 3, 0.001, 1000.0, 0.01, 3, 0.75 },
    { 1, 1, 0.001, 1000.0, 0.01, 3, 0.23 },
    { 1, 3, 0.001, 1000.0, 0.01, 3, 0.05 },
    { 2, 1, 0.001, 1000.0, 0.01, 3, 0.02 },
};

}

namespace cnoid {

class StairsGeneratorImpl : public Dialog
{
public:
    StairsGeneratorImpl(StairsGenerator* self);
    StairsGenerator* self;

    enum DoubleSpinId {
        TREAD, WIDTH, RISER,
        STRINGER, THICKNESS,
        NUM_DSPINS
    };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* stepsSpin;
    PushButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    bool save(const string& filename);
    void onColorButtonClicked();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    void writeStringerShape(Listing* elementsNode);
    void writeStepShape(Listing* elementsNode);
};

}


StairsGenerator::StairsGenerator()
{
    impl = new StairsGeneratorImpl(this);
}


StairsGeneratorImpl::StairsGeneratorImpl(StairsGenerator* self)
    : self(self)
{
    setWindowTitle(_("Stairs Generator"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QGridLayout* gbox = new QGridLayout;

    static const char* label[] = {
        _("Tread [m]"),  _("Stair width [m]"),
        _("Riser [m]"), _("Width of stringer [m]"),
        _("Tread thickness [m]")
    };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        dspins[i]->setRange(info.min, info.max);
        dspins[i]->setSingleStep(info.step);
        dspins[i]->setDecimals(info.decimals);
        dspins[i]->setValue(info.value);
        gbox->addWidget(new QLabel(label[i]), info.row, info.column - 1);
        gbox->addWidget(dspins[i], info.row, info.column);
    }

    stepsSpin = new SpinBox;
    stepsSpin->setRange(1, 9999);
    stepsSpin->setValue(10);
    gbox->addWidget(new QLabel(_("Number of steps [-]")), 2, 2);
    gbox->addWidget(stepsSpin, 2, 3); 

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
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


StairsGenerator::~StairsGenerator()
{
    delete impl;
}


void StairsGenerator::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new StairsGenerator);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("Stairs"))->sigTriggered().connect(
                    [&](){ instance_->impl->show(); });
    }
}


bool StairsGeneratorImpl::save(const string& filename)
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


void StairsGeneratorImpl::onColorButtonClicked()
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


MappingPtr StairsGeneratorImpl::writeBody(const string& filename)
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


MappingPtr StairsGeneratorImpl::writeLink()
{
    MappingPtr node = new Mapping;

    node->write("name", "Root");
    write(node, "translation", Vector3(0.0, 0.0, 0.0));
    node->write("jointType", "fixed");
    node->write("material", "Ground");
    node->write("AMOR", true);

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void StairsGeneratorImpl::writeLinkShape(Listing* elementsNode)
{
    double tread = dspins[TREAD]->value();
    double width = dspins[WIDTH]->value();
    double riser = dspins[RISER]->value();
    double stringer = dspins[STRINGER]->value();
    double steps = stepsSpin->value();

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


void StairsGeneratorImpl::writeStringerShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double tread = dspins[TREAD]->value();
    double riser = dspins[RISER]->value();
    double stringer = dspins[STRINGER]->value();
    double steps = stepsSpin->value();

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");

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
}


void StairsGeneratorImpl::writeStepShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double tread = dspins[TREAD]->value();
    double width = dspins[WIDTH]->value();
    double thickness = dspins[THICKNESS]->value();

    node->write("type", "Shape");
    write(node, "translation", Vector3(tread / 2.0, 0.0, -thickness / 2.0));

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Box");
    write(geometryNode, "size", Vector3(tread, width, thickness));

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
}