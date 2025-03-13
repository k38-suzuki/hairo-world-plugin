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
    { 0, 1, 0.01, 1000.0, 0.01, 3, 1.00,           "mass", nullptr },
    { 0, 3, 0.01, 1000.0, 0.01, 3, 0.50,    "bent_radius", nullptr },
    { 1, 1, 0.01, 1000.0, 0.01, 3, 0.03, "inner_diameter", nullptr },
    { 1, 3, 0.01, 1000.0, 0.01, 3, 0.05, "outer_diameter", nullptr }
};

SpinInfo spinInfo[] = {
    { 2, 1, 1, 360, 1, 90,      "bent_angle", nullptr },
    { 2, 3, 1, 120, 1, 30, "bent_step_angle", nullptr },
    { 3, 3, 1, 120, 1, 30,      "step_angle", nullptr }
};

class BentPipeCreatorWidget : public QWidget
{
public:
    BentPipeCreatorWidget(QWidget* parent = nullptr);

private:
    void reset();
    bool save(const string& filename);

    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    void onBentAngleChanged(double value);
    void onInnerDiameterChanged(double diameter);
    void onOuterDiameterChanged(double diameter);

    enum { MASS, BENT_RAD, IN_DIA, OUT_DIA, NumDoubleSpinBoxes };
    enum { BENT_ANGLE, BENT_STEP, STEP, NumSpinBoxes };

    DoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    SpinBox* spinBoxes[NumSpinBoxes];
    ColorButton* colorButton;
    YAMLWriter yamlWriter;
};

}


void BentPipeCreator::initializeClass(ExtensionManager* ext)
{
    static BentPipeCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new BentPipeCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("BentPipe"), widget);
    }
}


BentPipeCreator::BentPipeCreator()
{

}


BentPipeCreator::~BentPipeCreator()
{

}


BentPipeCreatorWidget::BentPipeCreatorWidget(QWidget* parent)
    : QWidget(parent)
{
    yamlWriter.setKeyOrderPreservationMode(true);

    auto gridLayout = new QGridLayout;

    const QStringList list = {
        _("Mass [kg]"), _("Bent radius [m]"),
        _("Inner diameter [m]"), _("Outer diameter [m]")
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
    doubleSpinBoxes[IN_DIA]->sigValueChanged().connect([&](double value){ onInnerDiameterChanged(value); });
    doubleSpinBoxes[OUT_DIA]->sigValueChanged().connect([&](double value){ onOuterDiameterChanged(value); });

    const QStringList list2 = {
        _("Bent angle [deg]"), _("Bent step angle [deg]"),
         _("Step angle [deg]")
    };

    for(int i = 0; i < NumSpinBoxes; ++i) {
        SpinInfo& info = spinInfo[i];
        info.spin = spinBoxes[i] = new SpinBox;
        info.spin->setRange(info.min, info.max);
        info.spin->setValue(info.value);
        gridLayout->addWidget(new QLabel(list2[i]), info.row, info.column - 1);
        gridLayout->addWidget(info.spin, info.row, info.column);
    }
    spinBoxes[BENT_ANGLE]->sigValueChanged().connect([&](double value){ onBentAngleChanged(value); });

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

    setWindowTitle(_("BentPipe Generator"));
}


void BentPipeCreatorWidget::reset()
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


bool BentPipeCreatorWidget::save(const string& filename)
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


MappingPtr BentPipeCreatorWidget::writeBody(const string& filename)
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


MappingPtr BentPipeCreatorWidget::writeLink()
{
    MappingPtr node = new Mapping;

    double mass = doubleSpinBoxes[MASS]->value();

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


void BentPipeCreatorWidget::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double r_bent = doubleSpinBoxes[BENT_RAD]->value();
    double d_in = doubleSpinBoxes[IN_DIA]->value();
    double d_out = doubleSpinBoxes[OUT_DIA]->value();
    double r_in = d_in / 2.0;
    double r_out = d_out / 2.0;

    int bent_angle = spinBoxes[BENT_ANGLE]->value();
    int bent_step = spinBoxes[BENT_STEP]->value();
    int step = spinBoxes[STEP]->value();

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


void BentPipeCreatorWidget::onBentAngleChanged(double value)
{
    double bent_step = spinBoxes[BENT_STEP]->value();
    if(value < bent_step) {
        spinBoxes[BENT_ANGLE]->setValue(bent_step);
    }
}


void BentPipeCreatorWidget::onInnerDiameterChanged(double diameter)
{
    double d_out = doubleSpinBoxes[OUT_DIA]->value();
    if(diameter >= d_out) {
        double d_in = d_out - 0.01;
        doubleSpinBoxes[IN_DIA]->setValue(d_in);
    }
}


void BentPipeCreatorWidget::onOuterDiameterChanged(double diameter)
{
    double d_in = doubleSpinBoxes[IN_DIA]->value();
    if(diameter <= d_in) {
        double d_out = d_in + 0.01;
        doubleSpinBoxes[OUT_DIA]->setValue(d_out);
    }
}