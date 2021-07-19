/**
   \file
   \author Kenta Suzuki
*/

#include "InertiaCalculatorDialog.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
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

InertiaCalculatorDialog* dialog = nullptr;

namespace {

struct DialogButtonInfo {
    QDialogButtonBox::ButtonRole role;
    char* label;
};


DialogButtonInfo dialogButtonInfo[] = {
    { QDialogButtonBox::ActionRole, _("&Calc") },
    { QDialogButtonBox::AcceptRole,  _ ("&Ok") }
};

}


namespace cnoid {

class InertiaCalculatorDialogImpl
{
public:
    InertiaCalculatorDialogImpl(InertiaCalculatorDialog* self);

    InertiaCalculatorDialog* self;
    ComboBox* shapeCombo;
    QStackedLayout* stbox;
    MessageView* messageView;
    ComboBox* cylinderAxisCombo;
    ComboBox* coneAxisCombo;
    DoubleSpinBox* dspinBox[12];

    enum Shape {
        BOX,
        SPHERE,
        CYLINDER,
        CONE,
        NUM_SHAPE
    };

    enum Axis {
        X,
        Y,
        Z,
        NUM_AXIS
    };

    enum DialogButtonId { CALC, OK, NUM_DBUTTONS };

    PushButton* dialogButtons[NUM_DBUTTONS];

    void onAccepted();
    void onRejected();
    void calcBoxInertia();
    void calcSphereInertia();
    void calcCylinderInertia();
    void calcConeInertia();
    void printIntertia(const Vector3& inertia);
};

}


InertiaCalculatorDialog::InertiaCalculatorDialog()
{
    impl = new InertiaCalculatorDialogImpl(this);
}


InertiaCalculatorDialogImpl::InertiaCalculatorDialogImpl(InertiaCalculatorDialog* self)
    : self(self)
{
    self->setWindowTitle(_("InertiaCalculator"));

    shapeCombo = new ComboBox();
    QStringList items = { _("Box"), _("Sphere"), _("Cylinder"), _("Cone") };
    shapeCombo->addItems(items);
    QHBoxLayout* sbox = new QHBoxLayout();
    sbox->addWidget(new QLabel(_("Shape")));
    sbox->addWidget(shapeCombo);
    sbox->addStretch();

    stbox = new QStackedLayout();
    messageView = new MessageView();

    for(int i = 0; i < 12; i++) {
        dspinBox[i] = new DoubleSpinBox();
        dspinBox[i]->setDecimals(4);
        dspinBox[i]->setSingleStep(0.01);
        dspinBox[i]->setRange(0.0000, 1000.0);
    }
    cylinderAxisCombo = new ComboBox();
    coneAxisCombo = new ComboBox();
    QStringList axisList = { "x", "y", "z" };
    cylinderAxisCombo->addItems(axisList);
    coneAxisCombo->addItems(axisList);

    QHBoxLayout* bhbox = new QHBoxLayout();
    bhbox->addWidget(new QLabel(_("mass [kg]")));
    bhbox->addWidget(dspinBox[0]);
    bhbox->addWidget(new QLabel(_("x [m]")));
    bhbox->addWidget(dspinBox[1]);
    bhbox->addWidget(new QLabel(_("y [m]")));
    bhbox->addWidget(dspinBox[2]);
    bhbox->addWidget(new QLabel(_("z [m]")));
    bhbox->addWidget(dspinBox[3]);
    bhbox->addStretch();
    Widget* boxWidget = new Widget();
    boxWidget->setLayout(bhbox);

    QHBoxLayout* shbox = new QHBoxLayout();
    shbox->addWidget(new QLabel(_("mass [kg]")));
    shbox->addWidget(dspinBox[4]);
    shbox->addWidget(new QLabel(_("radius [m]")));
    shbox->addWidget(dspinBox[5]);
    shbox->addStretch();
    Widget* sphereWidget = new Widget();
    sphereWidget->setLayout(shbox);

    QHBoxLayout* cyhbox = new QHBoxLayout();
    cyhbox->addWidget(new QLabel(_("mass [kg]")));
    cyhbox->addWidget(dspinBox[6]);
    cyhbox->addWidget(new QLabel(_("radius [m]")));
    cyhbox->addWidget(dspinBox[7]);
    cyhbox->addWidget(new QLabel(_("height [m]")));
    cyhbox->addWidget(dspinBox[8]);
    cyhbox->addWidget(new QLabel(_("axis [-]")));
    cyhbox->addWidget(cylinderAxisCombo);
    cyhbox->addStretch();
    Widget* cylinderWidget = new Widget();
    cylinderWidget->setLayout(cyhbox);

    QHBoxLayout* cnhbox = new QHBoxLayout();
    cnhbox->addWidget(new QLabel(_("mass [kg]")));
    cnhbox->addWidget(dspinBox[9]);
    cnhbox->addWidget(new QLabel(_("radius [m]")));
    cnhbox->addWidget(dspinBox[10]);
    cnhbox->addWidget(new QLabel(_("height [m]")));
    cnhbox->addWidget(dspinBox[11]);
    cnhbox->addWidget(new QLabel(_("axis [-]")));
    cnhbox->addWidget(coneAxisCombo);
    cnhbox->addStretch();
    Widget* coneWidget = new Widget();
    coneWidget->setLayout(cnhbox);

    stbox->addWidget(boxWidget);
    stbox->addWidget(sphereWidget);
    stbox->addWidget(cylinderWidget);
    stbox->addWidget(coneWidget);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    for(int i = 0; i < NUM_DBUTTONS; ++i) {
        DialogButtonInfo info = dialogButtonInfo[i];
        dialogButtons[i] = new PushButton(info.label);
        PushButton* dialogButton = dialogButtons[i];
        buttonBox->addButton(dialogButton, info.role);
        if(i == OK) {
            dialogButton->setDefault(true);
        }
    }
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(sbox);
    vbox->addLayout(stbox);
    vbox->addWidget(messageView);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    dialogButtons[CALC]->sigClicked().connect([&](){
        int index = shapeCombo->currentIndex();
        switch (index) {
        case Shape::BOX:
            calcBoxInertia();
            break;
        case Shape::SPHERE:
            calcSphereInertia();
            break;
        case Shape::CYLINDER:
            calcCylinderInertia();
            break;
        case Shape::CONE:
            calcConeInertia();
            break;
        default:
            break;
        }
    });

    shapeCombo->sigCurrentIndexChanged().connect([&](int index){ stbox->setCurrentIndex(index); });
}


InertiaCalculatorDialog::~InertiaCalculatorDialog()
{
    delete impl;
}


void InertiaCalculatorDialog::initializeClass(ExtensionManager* ext)
{
    if(!dialog) {
        dialog = ext->manage(new InertiaCalculatorDialog());
    }

    MenuManager& menuManager = ext->menuManager();
    menuManager.setPath("/Tools");
    menuManager.addItem(_("InertiaCalculator"))
            ->sigTriggered().connect([](){ dialog->show(); });
}


void InertiaCalculatorDialog::onAccepted()
{
    impl->onAccepted();
}


void InertiaCalculatorDialogImpl::onAccepted()
{

}


void InertiaCalculatorDialog::onRejected()
{
    impl->onRejected();
}


void InertiaCalculatorDialogImpl::onRejected()
{

}


void InertiaCalculatorDialogImpl::calcBoxInertia()
{
    double mass = dspinBox[0]->value();
    double x = dspinBox[1]->value();
    double y = dspinBox[2]->value();
    double z = dspinBox[3]->value();
    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    messageView->putln(fmt::format(_("shape: Box, mass: {0} [kg], x: {1} [m], y: {2} [m], z: {3} [m]"),
                                   mass, x, y, z));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorDialogImpl::calcSphereInertia()
{
    double mass = dspinBox[4]->value();
    double radius = dspinBox[5]->value();
    double ix, iy, iz;
    ix = iy = iz = mass * radius * radius / 5.0 * 2.0;

    messageView->putln(fmt::format(_("shape: Sphere, mass: {0} [kg], radius: {1} [m]"),
                                   mass, radius));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorDialogImpl::calcCylinderInertia()
{
    double mass = dspinBox[6]->value();
    double radius = dspinBox[7]->value();
    double height = dspinBox[8]->value();
    int index = cylinderAxisCombo->currentIndex();
    double mainInertia = mass * radius * radius / 2.0;
    double subInertia = mass * (3.0 * radius * radius + height * height) / 12.0;
    double ix, iy, iz;

    switch (index) {
    case Axis::X:
        ix = mainInertia;
        iy = iz = subInertia;
        break;
    case Axis::Y:
        iy = mainInertia;
        iz = ix = subInertia;
        break;
    case Axis::Z:
        iz = mainInertia;
        ix = iy = subInertia;
        break;
    default:
        break;
    }

    messageView->putln(fmt::format(_("shape: Cylinder, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   mass, radius, height, cylinderAxisCombo->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorDialogImpl::calcConeInertia()
{
    double mass = dspinBox[9]->value();
    double radius = dspinBox[10]->value();
    double height = dspinBox[11]->value();
    int index = coneAxisCombo->currentIndex();
    double mainInertia = mass * radius * radius * 3.0 / 10.0;
    double subInertia = mass * 3.0 / 80.0 * (4.0 * radius * radius + height * height);
    double ix, iy, iz;

    switch (index) {
    case Axis::X:
        ix = mainInertia;
        iy = iz = subInertia;
        break;
    case Axis::Y:
        iy = mainInertia;
        iz = ix = subInertia;
        break;
    case Axis::Z:
        iz = mainInertia;
        ix = iy = subInertia;
        break;
    default:
        break;
    }

    messageView->putln(fmt::format(_("shape: Cone, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   mass, radius, height, coneAxisCombo->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculatorDialogImpl::printIntertia(const Vector3& inertia)
{
    messageView->putln(fmt::format(_("inertia: [ {0}, 0, 0, 0, {1}, 0, 0, 0, {2} ]\n"), inertia[0], inertia[1], inertia[2]));
}
