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
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

struct ButtonInfo {
    QDialogButtonBox::ButtonRole role;
};


ButtonInfo buttonInfo[] = {
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::AcceptRole }
};

}


namespace cnoid {

class ConfigDialog : public Dialog
{
public:
    ConfigDialog();

    enum ShapeID { BOX, SPHERE, CYLINDER, CONE, NUM_SHAPE };
    enum AxisID { X, Y, Z, NUM_AXIS };
    enum DialogButtonID { CALC, OK, NUM_DBUTTONS };

    ComboBox* shapeCombo;
    QStackedLayout* stbox;
    MessageView* mv;
    ComboBox* cylinderAxisCombo;
    ComboBox* coneAxisCombo;
    DoubleSpinBox* dspins[12];
    PushButton* dialogButtons[NUM_DBUTTONS];

    void calcBoxInertia();
    void calcSphereInertia();
    void calcCylinderInertia();
    void calcConeInertia();
    void printIntertia(const Vector3& inertia);
};


class InertiaCalculatorImpl
{
public:
    InertiaCalculatorImpl(InertiaCalculator* self, ExtensionManager* ext);
    InertiaCalculator* self;

    ConfigDialog* dialog;
};

}


InertiaCalculator::InertiaCalculator(ExtensionManager* ext)
{
    impl = new InertiaCalculatorImpl(this, ext);
}


InertiaCalculatorImpl::InertiaCalculatorImpl(InertiaCalculator* self, ExtensionManager* ext)
    : self(self)
{
    dialog = new ConfigDialog();

    MenuManager& mm = ext->menuManager().setPath("/Tools");
    mm.addItem(_("InertiaCalculator"))->sigTriggered().connect([&](){ dialog->show(); });
}


InertiaCalculator::~InertiaCalculator()
{
    delete impl;
}


void InertiaCalculator::initialize(ExtensionManager* ext)
{
    ext->manage(new InertiaCalculator(ext));
}


ConfigDialog::ConfigDialog()
    : mv(new MessageView())
{
    setWindowTitle(_("InertiaCalculator"));

    shapeCombo = new ComboBox();
    QStringList items = { _("Box"), _("Sphere"), _("Cylinder"), _("Cone") };
    shapeCombo->addItems(items);
    QHBoxLayout* sbox = new QHBoxLayout();
    sbox->addWidget(new QLabel(_("Shape")));
    sbox->addWidget(shapeCombo);
    sbox->addStretch();

    stbox = new QStackedLayout();

    for(int i = 0; i < 12; i++) {
        dspins[i] = new DoubleSpinBox();
        dspins[i]->setDecimals(4);
        dspins[i]->setSingleStep(0.01);
        dspins[i]->setRange(0.0000, 1000.0);
    }
    cylinderAxisCombo = new ComboBox();
    coneAxisCombo = new ComboBox();
    QStringList axisList = { "x", "y", "z" };
    cylinderAxisCombo->addItems(axisList);
    coneAxisCombo->addItems(axisList);

    QHBoxLayout* bhbox = new QHBoxLayout();
    bhbox->addWidget(new QLabel(_("mass [kg]")));
    bhbox->addWidget(dspins[0]);
    bhbox->addWidget(new QLabel(_("x [m]")));
    bhbox->addWidget(dspins[1]);
    bhbox->addWidget(new QLabel(_("y [m]")));
    bhbox->addWidget(dspins[2]);
    bhbox->addWidget(new QLabel(_("z [m]")));
    bhbox->addWidget(dspins[3]);
    bhbox->addStretch();
    Widget* boxWidget = new Widget();
    boxWidget->setLayout(bhbox);

    QHBoxLayout* shbox = new QHBoxLayout();
    shbox->addWidget(new QLabel(_("mass [kg]")));
    shbox->addWidget(dspins[4]);
    shbox->addWidget(new QLabel(_("radius [m]")));
    shbox->addWidget(dspins[5]);
    shbox->addStretch();
    Widget* sphereWidget = new Widget();
    sphereWidget->setLayout(shbox);

    QHBoxLayout* cyhbox = new QHBoxLayout();
    cyhbox->addWidget(new QLabel(_("mass [kg]")));
    cyhbox->addWidget(dspins[6]);
    cyhbox->addWidget(new QLabel(_("radius [m]")));
    cyhbox->addWidget(dspins[7]);
    cyhbox->addWidget(new QLabel(_("height [m]")));
    cyhbox->addWidget(dspins[8]);
    cyhbox->addWidget(new QLabel(_("axis [-]")));
    cyhbox->addWidget(cylinderAxisCombo);
    cyhbox->addStretch();
    Widget* cylinderWidget = new Widget();
    cylinderWidget->setLayout(cyhbox);

    QHBoxLayout* cnhbox = new QHBoxLayout();
    cnhbox->addWidget(new QLabel(_("mass [kg]")));
    cnhbox->addWidget(dspins[9]);
    cnhbox->addWidget(new QLabel(_("radius [m]")));
    cnhbox->addWidget(dspins[10]);
    cnhbox->addWidget(new QLabel(_("height [m]")));
    cnhbox->addWidget(dspins[11]);
    cnhbox->addWidget(new QLabel(_("axis [-]")));
    cnhbox->addWidget(coneAxisCombo);
    cnhbox->addStretch();
    Widget* coneWidget = new Widget();
    coneWidget->setLayout(cnhbox);

    stbox->addWidget(boxWidget);
    stbox->addWidget(sphereWidget);
    stbox->addWidget(cylinderWidget);
    stbox->addWidget(coneWidget);

    const char* labels[] = { _("&Calc"), _ ("&Ok") };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    for(int i = 0; i < NUM_DBUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        dialogButtons[i] = new PushButton(labels[i]);
        PushButton* dialogButton = dialogButtons[i];
        buttonBox->addButton(dialogButton, info.role);
        if(i == OK) {
            dialogButton->setDefault(true);
        }
    }
    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(sbox);
    vbox->addLayout(stbox);
    vbox->addWidget(mv);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    dialogButtons[CALC]->sigClicked().connect([&](){
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
}


void ConfigDialog::calcBoxInertia()
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


void ConfigDialog::calcSphereInertia()
{
    double mass = dspins[4]->value();
    double radius = dspins[5]->value();
    double ix, iy, iz;
    ix = iy = iz = mass * radius * radius / 5.0 * 2.0;

    mv->putln(fmt::format(_("shape: Sphere, mass: {0} [kg], radius: {1} [m]"),
                                   mass, radius));
    printIntertia(Vector3(ix, iy, iz));
}


void ConfigDialog::calcCylinderInertia()
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


void ConfigDialog::calcConeInertia()
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


void ConfigDialog::printIntertia(const Vector3& inertia)
{
    mv->putln(fmt::format(_("inertia: [ {0}, 0, 0, 0, {1}, 0, 0, 0, {2} ]\n"), inertia[0], inertia[1], inertia[2]));
}
