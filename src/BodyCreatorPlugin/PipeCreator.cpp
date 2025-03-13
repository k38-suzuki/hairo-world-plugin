/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/Action>
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/Menu>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QContextMenuEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QtMath>
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
    { 0, 3, 0.01, 1000.0, 0.01, 3, 1.00,         "length", nullptr },
    { 1, 1, 0.01, 1000.0, 0.01, 3, 0.03, "inner_diameter", nullptr },
    { 1, 3, 0.01, 1000.0, 0.01, 3, 0.05, "outer_diameter", nullptr }
};

SpinInfo spinInfo[] = {
    { 3, 3, 0, 359, 1,  0,      "angle", nullptr },
    { 2, 1, 1, 120, 1, 30, "inter_step", nullptr },
    { 2, 3, 1, 120, 1, 30, "outer_step", nullptr }
};

class SquarePipeCreatorDialog : public QDialog
{
public:
    SquarePipeCreatorDialog(QWidget* parent = nullptr);

    void setWidth(double width) { widthSpinBox->setValue(width); }
    double width() const { return widthSpinBox->value(); }

    void setRadius(double radius) { radiusSpinBox->setValue(radius); }
    double  radius() const { return radiusSpinBox->value(); }

    void setLength(double length) { lengthSpinBox->setValue(length); }
    double length() const { return lengthSpinBox->value(); }

private:
    DoubleSpinBox* widthSpinBox;
    DoubleSpinBox* heightSpinBox;
    DoubleSpinBox* radiusSpinBox;
    DoubleSpinBox* lengthSpinBox;
    QDialogButtonBox* buttonBox;
};

class PipeCreatorWidget : public QWidget
{
public:
    PipeCreatorWidget(QWidget* parent = nullptr);

private:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;

    void reset();
    void configure();
    bool save(const string& filename);

    void onInnerDiameterChanged(const double& diameter);
    void onOuterDiameterChanged(const double& diameter);
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    VectorXd calcInertia();

    enum { MASS, LENGTH, IN_DIA, OUT_DIA, NumDoubleSpinBoxes };
    enum { ANGLE, IN_STEP, OUT_STEP, NumSpinBoxes };

    DoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    SpinBox* spinBoxes[NumSpinBoxes];
    ColorButton* colorButton;
    YAMLWriter yamlWriter;
    Action* configureAct;
};


}


void PipeCreator::initializeClass(ExtensionManager* ext)
{
    static PipeCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new PipeCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Pipe"), widget);
    }
}


PipeCreator::PipeCreator()
{

}


PipeCreator::~PipeCreator()
{

}


PipeCreatorWidget::PipeCreatorWidget(QWidget* parent)
    : QWidget(parent)
{
    yamlWriter.setKeyOrderPreservationMode(true);

    auto gridLayout = new QGridLayout;

    const QStringList list = { _("Mass [kg]"), _("Length [m]"),
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

    const QStringList list2 = { _("Opening angle [deg]"), _("Inner step angle [deg]"), _("Outer step angle [deg]") };

    for(int i = 0; i < NumSpinBoxes; ++i) {
        SpinInfo& info = spinInfo[i];
        info.spin = spinBoxes[i] = new SpinBox;
        info.spin->setRange(info.min, info.max);
        info.spin->setValue(info.value);
        gridLayout->addWidget(new QLabel(list2[i]), info.row, info.column - 1);
        gridLayout->addWidget(info.spin, info.row, info.column);
    }

    colorButton = new ColorButton;
    colorButton->setColor(Vector3(0.5, 0.5, 0.5));
    gridLayout->addWidget(new QLabel(_("Color [-]")), 3, 0);
    gridLayout->addWidget(colorButton, 3, 1);

    configureAct = new Action;
    configureAct->setText(_("Advanced settings"));
    configureAct->sigTriggered().connect([this](){ configure(); });

    auto toolBar = new CreatorToolBar;
    toolBar->sigNewRequested().connect([&](){ reset(); });
    toolBar->sigSaveRequested().connect([&](const string& filename){ save(filename); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("Pipe Generator"));
}


void PipeCreatorWidget::contextMenuEvent(QContextMenuEvent* event)
{
    Menu menu(this);
    menu.addAction(configureAct);
    menu.exec(event->globalPos());
}


void PipeCreatorWidget::reset()
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


void PipeCreatorWidget::configure()
{
    SquarePipeCreatorDialog dialog;
    dialog.setWidth(doubleSpinBoxes[OUT_DIA]->value() / qSqrt(2.0));
    dialog.setRadius(doubleSpinBoxes[IN_DIA]->value() / 2.0);
    dialog.setLength(doubleSpinBoxes[LENGTH]->value());

    if(dialog.exec() == QDialog::Accepted) {
        doubleSpinBoxes[LENGTH]->setValue(dialog.length());
        doubleSpinBoxes[OUT_DIA]->setValue(dialog.width() * qSqrt(2.0));
        doubleSpinBoxes[IN_DIA]->setValue(dialog.radius() * 2.0);
        spinBoxes[OUT_STEP]->setValue(90);
        spinBoxes[IN_STEP]->setValue(90);
    }
}


bool PipeCreatorWidget::save(const string& filename)
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


void PipeCreatorWidget::onInnerDiameterChanged(const double& diameter)
{
    double d_out = doubleSpinBoxes[OUT_DIA]->value();
    if(diameter >= d_out) {
        double d_in = d_out - 0.01;
        doubleSpinBoxes[IN_DIA]->setValue(d_in);
    }
}


void PipeCreatorWidget::onOuterDiameterChanged(const double& diameter)
{
    double d_in = doubleSpinBoxes[IN_DIA]->value();
    if(diameter <= d_in) {
        double d_out = d_in + 0.01;
        doubleSpinBoxes[OUT_DIA]->setValue(d_out);
    }
}


MappingPtr PipeCreatorWidget::writeBody(const string& filename)
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


MappingPtr PipeCreatorWidget::writeLink()
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


void PipeCreatorWidget::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    double length = doubleSpinBoxes[LENGTH]->value();
    double d_in = doubleSpinBoxes[IN_DIA]->value();
    double d_out = doubleSpinBoxes[OUT_DIA]->value();
    double r_in = d_in / 2.0;
    double r_out = d_out / 2.0;

    int angle = spinBoxes[ANGLE]->value();
    int step_in = spinBoxes[IN_STEP]->value();
    int step_out = spinBoxes[OUT_STEP]->value();

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("cross_section");

    int range = 360 - angle;
    int n = ((360 - angle) / step_in + 1) + ((360 - angle) / step_out + 1) + 1;
    double sx;
    double sy;
    for(int i = 0; i <= range; i += step_out) {
        double x = r_out * cos(i * TO_RADIAN);
        double y = r_out * sin(i * TO_RADIAN);
        if(i == 0) {
            sx = x;
            sy = y;
        }
        crossSectionList.append(x, 2, n);
        crossSectionList.append(y, 2, n);
    }
    for(int i = 0; i <= range; i += step_in) {
        double x = r_in * cos((range - i) * TO_RADIAN);
        double y = r_in * sin((range - i) * TO_RADIAN);
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
    Vector3 c = colorButton->color();
    for(int i = 0; i < 3; ++i) {
        diffuseColorList.append(c[i], 3, 3);
    }

    elementsNode->append(node);
}


VectorXd PipeCreatorWidget::calcInertia()
{
    VectorXd innerInertia, outerInertia;
    innerInertia.resize(9);
    outerInertia.resize(9);

    double length = doubleSpinBoxes[LENGTH]->value();
    double d_in = doubleSpinBoxes[IN_DIA]->value();
    double d_out = doubleSpinBoxes[OUT_DIA]->value();
    double r_in = d_in / 2.0;
    double r_out = d_out / 2.0;

    double innerRate = r_in * r_in / r_out * r_out;
    double outerRate = 1.0 - innerRate;

    {
        double mass = doubleSpinBoxes[MASS]->value() * innerRate;
        double radius = r_in;
        double mainInertia = mass * radius * radius / 2.0;
        double subInertia = mass * (3.0 * radius * radius + length * length) / 12.0;
        double ix, iy, iz;
        ix = iz = subInertia;
        iy = mainInertia;
        innerInertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;
    }

    {
        double mass = doubleSpinBoxes[MASS]->value() * outerRate;
        double radius = r_out;
        double mainInertia = mass * radius * radius / 2.0;
        double subInertia = mass * (3.0 * radius * radius + length * length) / 12.0;
        double ix, iy, iz;
        ix = iz = subInertia;
        iy = mainInertia;
        outerInertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;
    }

    return outerInertia - innerInertia;
}


SquarePipeCreatorDialog::SquarePipeCreatorDialog(QWidget* parent)
    : QDialog(parent)
{
    widthSpinBox = new DoubleSpinBox;
    widthSpinBox->setRange(0.01, 1000.0);
    widthSpinBox->setSingleStep(0.01);
    widthSpinBox->setDecimals(3);
    widthSpinBox->sigValueChanged().connect([&](double value){ heightSpinBox->setValue(value); });

    heightSpinBox = new DoubleSpinBox;
    heightSpinBox->setRange(0.01, 1000.0);
    heightSpinBox->setSingleStep(0.01);
    heightSpinBox->setDecimals(3);
    heightSpinBox->setEnabled(false);

    radiusSpinBox = new DoubleSpinBox;
    radiusSpinBox->setRange(0.01, 1000.0);
    radiusSpinBox->setSingleStep(0.01);
    radiusSpinBox->setDecimals(3);

    lengthSpinBox = new DoubleSpinBox;
    lengthSpinBox->setRange(0.01, 1000.0);
    lengthSpinBox->setSingleStep(0.01);
    lengthSpinBox->setDecimals(3);

    auto formLayout = new QFormLayout;
    formLayout->addRow(_("Width [m]"), widthSpinBox);
    formLayout->addRow(_("Height [m]"), heightSpinBox);
    formLayout->addRow(_("Radius [m]"), radiusSpinBox);
    formLayout->addRow(_("Length [m]"), lengthSpinBox);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("SquarePipe Generator"));
}