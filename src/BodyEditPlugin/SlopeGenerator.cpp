/**
   \file
   \author Kenta Suzuki
*/

#include "SlopeGenerator.h"
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

SlopeGenerator* sgeneratorInstance = nullptr;

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

class SlopeGeneratorImpl : public Dialog
{
public:
    SlopeGeneratorImpl(SlopeGenerator* self);
    SlopeGenerator* self;

    enum DoubleSpinId {
        MASS, WIDTH, HEIGHT,
        LENGTH, NUM_DSPINS
    };

    DoubleSpinBox* dspins[NUM_DSPINS];
    PushButton* colorButton;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    bool save(const string& filename);
    void onColorButtonClicked();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();
};

}


SlopeGenerator::SlopeGenerator()
{
    impl = new SlopeGeneratorImpl(this);
}


SlopeGeneratorImpl::SlopeGeneratorImpl(SlopeGenerator* self)
    : self(self)
{
    setWindowTitle(_("Slope Builder"));
    yamlWriter.setKeyOrderPreservationMode(true);

    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;

    static const char* dlabels[] = {
        _("Mass [kg]"),  _("Width [m]"),
        _("Height [m]"), _("Length [m]")
    };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        dspins[i]->setRange(info.min, info.max);
        dspins[i]->setSingleStep(info.step);
        dspins[i]->setDecimals(info.decimals);
        dspins[i]->setValue(info.value);
        gbox->addWidget(new QLabel(dlabels[i]), info.row, info.column - 1);
        gbox->addWidget(dspins[i], info.row, info.column);
    }

    colorButton = new PushButton;
    gbox->addWidget(new QLabel(_("Color [-]")), 2, 0);
    gbox->addWidget(colorButton, 2, 1);

    formWidget = new FileFormWidget;

    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


SlopeGenerator::~SlopeGenerator()
{
    delete impl;
}


void SlopeGenerator::initializeClass(ExtensionManager* ext)
{
    if(!sgeneratorInstance) {
        sgeneratorInstance = ext->manage(new SlopeGenerator);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("Slope"))->sigTriggered().connect(
                    [&](){ sgeneratorInstance->impl->show(); });
    }
}


bool SlopeGeneratorImpl::save(const string& filename)
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


void SlopeGeneratorImpl::onColorButtonClicked()
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


MappingPtr SlopeGeneratorImpl::writeBody(const string& filename)
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


MappingPtr SlopeGeneratorImpl::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = dspins[MASS]->value();

    node->write("name", "Root");
    node->write("jointType", "fixed");
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


void SlopeGeneratorImpl::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double length = dspins[LENGTH]->value();
    double width = dspins[WIDTH]->value();
    double height = dspins[HEIGHT]->value();

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");

    int n = 8;
    double hl = length / 2.0;
    double hw = width / 2.0;
    double hh = height / 2.0;
    crossSectionList.append(-hl, 2, n);
    crossSectionList.append(-hh, 2, n);
    crossSectionList.append( hl, 2, n);
    crossSectionList.append(-hh, 2, n);
    crossSectionList.append( hl, 2, n);
    crossSectionList.append( hh, 2, n);
    crossSectionList.append(-hl, 2, n);
    crossSectionList.append(-hh, 2, n);

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


VectorXd SlopeGeneratorImpl::calcInertia()
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