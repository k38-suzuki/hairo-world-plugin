/**
   \file
   \author Kenta Suzuki
*/

#include "InertiaCalculator.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/EigenTypes>
#include <fmt/format.h>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

InertiaCalculator* calculatorInstance = nullptr;

struct DoubleSpinInfo {
    int row;
    int column;
    int page;
};

DoubleSpinInfo dspinInfo[] = {
    { 0, 1, 0 }, { 0, 3, 0 }, { 0, 5, 0 }, { 0, 7, 0 },
    { 0, 1, 1 }, { 0, 3, 1 },
    { 0, 1, 2 }, { 0, 3, 2 }, { 0, 5, 2 },
    { 0, 1, 3 }, { 0, 3, 3 }, { 0, 5, 3 }
};

struct LabelInfo {
    int row;
    int column;
    int page;
};

LabelInfo labelInfo[] = {
    { 0, 0, 0 }, { 0, 2, 0 }, { 0, 4, 0 }, { 0, 6, 0 },
    { 0, 0, 1 }, { 0, 2, 1 },
    { 0, 0, 2 }, { 0, 2, 2 }, { 0, 4, 2 }, { 0, 6, 2 },
    { 0, 0, 3 }, { 0, 2, 3 }, { 0, 4, 3 }, { 0, 6, 3 }
};

}

namespace cnoid {

class InertiaCalculatorImpl : public Dialog
{
public:
    InertiaCalculatorImpl(InertiaCalculator* self);
    InertiaCalculator* self;

    enum DoubleSpinID {
        BOX_MAS, BOX_X, BOX_Y, BOX_Z,
        SPR_MAS, SPR_RAD,
        CLD_MAS, CLD_RAD, CLD_HGT,
        CON_MAS, CON_RAD, CON_HGT,
        NUM_DSPINS
    };
    enum PageID { BOX, SPHERE, CYLINDER, CONE, NUM_PAGES };
    enum AxisID { X, Y, Z, NUM_AXES };

    ComboBox* shapeCombo;
    QStackedLayout* stbox;
    MessageView* mv;
    ComboBox* cylinderAxisCombo;
    ComboBox* coneAxisCombo;
    DoubleSpinBox* dspins[NUM_DSPINS];

    void calcBoxInertia();
    void calcSphereInertia();
    void calcCylinderInertia();
    void calcConeInertia();
    void printIntertia(const Vector3& inertia);
};

}


InertiaCalculator::InertiaCalculator()
{
    impl = new InertiaCalculatorImpl(this);
}


InertiaCalculatorImpl::InertiaCalculatorImpl(InertiaCalculator* self)
    : self(self),
      mv(new MessageView)
{
    setWindowTitle(_("InertiaCalculator"));

    shapeCombo = new ComboBox;
    const QStringList shapeList = { _("Box"), _("Sphere"), _("Cylinder"), _("Cone") };
    shapeCombo->addItems(shapeList);
    QHBoxLayout* shbox = new QHBoxLayout;
    shbox->addWidget(new QLabel(_("Shape")));
    shbox->addWidget(shapeCombo);
    shbox->addStretch();

    stbox = new QStackedLayout;
    QGridLayout* gbox[NUM_PAGES];
    for(int i = 0; i < NUM_PAGES; ++i) {
        Widget* pageWidget = new Widget;
        gbox[i] = new QGridLayout;
        pageWidget->setLayout(gbox[i]);
        stbox->addWidget(pageWidget);
    }

    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new DoubleSpinBox;
        DoubleSpinInfo info = dspinInfo[i];
        dspins[i]->setDecimals(4);
        dspins[i]->setSingleStep(0.01);
        dspins[i]->setRange(0.0000, 1000.0);
        gbox[info.page]->addWidget(dspins[i], info.row, info.column);
    }

    static const char* labels[] = {
        _("mass [kg]"), _("x [m]"), _("y [m]"), _("z [m]"),
        _("mass [kg]"), _("radius [m]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]"), _("axis [-]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]"), _("axis [-]")
    };

    for(int i = 0; i < 14; ++i) {
        LabelInfo info = labelInfo[i];
        gbox[info.page]->addWidget(new QLabel(labels[i]), info.row, info.column);
    }

    cylinderAxisCombo = new ComboBox;
    gbox[CYLINDER]->addWidget(cylinderAxisCombo, 0, 7);
    coneAxisCombo = new ComboBox;
    gbox[CONE]->addWidget(coneAxisCombo, 0, 7);
    const QStringList axisList = { "x", "y", "z" };
    cylinderAxisCombo->addItems(axisList);
    coneAxisCombo->addItems(axisList);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    PushButton* calcButton = new PushButton(_("&Calc"));
    buttonBox->addButton(calcButton, QDialogButtonBox::ActionRole);

    calcButton->sigClicked().connect([&](){
        int index = shapeCombo->currentIndex();
        switch (index) {
        case BOX:
            calcBoxInertia();
            break;
        case SPHERE:
            calcSphereInertia();
            break;
        case CYLINDER:
            calcCylinderInertia();
            break;
        case CONE:
            calcConeInertia();
            break;
        default:
            break;
        }
    });

    shapeCombo->sigCurrentIndexChanged().connect([&](int index){ stbox->setCurrentIndex(index); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(shbox);
    vbox->addLayout(stbox);
    vbox->addWidget(mv);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


InertiaCalculator::~InertiaCalculator()
{
    delete impl;
}


void InertiaCalculator::initializeClass(ExtensionManager* ext)
{
    if(!calculatorInstance) {
        calculatorInstance = ext->manage(new InertiaCalculator);
    }

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools"));
    mm.addItem(_("InertiaCalculator"))->sigTriggered().connect([&](){
        calculatorInstance->impl->mv->clear();
        calculatorInstance->impl->show();
    });
}


void InertiaCalculatorImpl::calcBoxInertia()
{
    double mass = dspins[0]->value();
    double x = dspins[1]->value();
    double y = dspins[2]->value();
    double z = dspins[3]->value();
    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    mv->putln(fmt::format(_("shape: Box, mass: {0} [kg], x: {1} [m], y: {2} [m], z: {3} [m]"),
                                   mass, x, y, z));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorImpl::calcSphereInertia()
{
    double mass = dspins[4]->value();
    double radius = dspins[5]->value();
    double ix, iy, iz;
    ix = iy = iz = mass * radius * radius / 5.0 * 2.0;

    mv->putln(fmt::format(_("shape: Sphere, mass: {0} [kg], radius: {1} [m]"),
                                   mass, radius));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorImpl::calcCylinderInertia()
{
    double mass = dspins[6]->value();
    double radius = dspins[7]->value();
    double height = dspins[8]->value();
    int index = cylinderAxisCombo->currentIndex();
    double mainInertia = mass * radius * radius / 2.0;
    double subInertia = mass * (3.0 * radius * radius + height * height) / 12.0;
    double ix, iy, iz;

    switch (index) {
    case X:
        ix = mainInertia;
        iy = iz = subInertia;
        break;
    case Y:
        iy = mainInertia;
        iz = ix = subInertia;
        break;
    case Z:
        iz = mainInertia;
        ix = iy = subInertia;
        break;
    default:
        break;
    }

    mv->putln(fmt::format(_("shape: Cylinder, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   mass, radius, height, cylinderAxisCombo->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorImpl::calcConeInertia()
{
    double mass = dspins[9]->value();
    double radius = dspins[10]->value();
    double height = dspins[11]->value();
    int index = coneAxisCombo->currentIndex();
    double mainInertia = mass * radius * radius * 3.0 / 10.0;
    double subInertia = mass * 3.0 / 80.0 * (4.0 * radius * radius + height * height);
    double ix, iy, iz;

    switch (index) {
    case X:
        ix = mainInertia;
        iy = iz = subInertia;
        break;
    case Y:
        iy = mainInertia;
        iz = ix = subInertia;
        break;
    case Z:
        iz = mainInertia;
        ix = iy = subInertia;
        break;
    default:
        break;
    }

    mv->putln(fmt::format(_("shape: Cone, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   mass, radius, height, coneAxisCombo->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorImpl::printIntertia(const Vector3& inertia)
{
    mv->putln(fmt::format(_("inertia: [ {0}, 0, 0, 0, {1}, 0, 0, 0, {2} ]\n"),
                          inertia[0], inertia[1], inertia[2]));
}
