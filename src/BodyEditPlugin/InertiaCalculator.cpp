/**
   @author Kenta Suzuki
*/

#include "InertiaCalculator.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/EigenTypes>
#include <fmt/format.h>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QStackedWidget>
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
    { 0, 1, 0 }, { 1, 1, 0 }, { 2, 1, 0 }, { 3, 1, 0 },
    { 0, 1, 1 }, { 1, 1, 1 },
    { 0, 1, 2 }, { 1, 1, 2 }, { 2, 1, 2 },
    { 0, 1, 3 }, { 1, 1, 3 }, { 2, 1, 3 }
};

struct LabelInfo {
    int row;
    int column;
    int page;
};

LabelInfo labelInfo[] = {
    { 0, 0, 0 }, { 1, 0, 0 }, { 2, 0, 0 }, { 3, 0, 0 },
    { 0, 0, 1 }, { 1, 0, 1 },
    { 0, 0, 2 }, { 1, 0, 2 }, { 2, 0, 2 }, { 3, 0, 2 },
    { 0, 0, 3 }, { 1, 0, 3 }, { 2, 0, 3 }, { 3, 0, 3 }
};

}

namespace cnoid {

class InertiaCalculator::Impl : public Dialog
{
public:

    enum DoubleSpinID {
        BOX_MAS, BOX_X, BOX_Y, BOX_Z,
        SPR_MAS, SPR_RAD,
        CLD_MAS, CLD_RAD, CLD_HGT,
        CON_MAS, CON_RAD, CON_HGT,
        NUM_DSPINS
    };
    enum ComboID { SHAPE, CLD_AXIS, CON_AXIS, NUM_COMBOS };
    enum PageID { BOX, SPHERE, CYLINDER, CONE, NUM_PAGES };
    enum AxisID { X, Y, Z, NUM_AXES };

    QStackedWidget* topWidget;
    MessageView* mv;
    DoubleSpinBox* dspins[NUM_DSPINS];
    ComboBox* combos[NUM_COMBOS];

    Impl();

    void onCalcButtonClicked();
    void calcBoxInertia();
    void calcSphereInertia();
    void calcCylinderInertia();
    void calcConeInertia();
    void printIntertia(const Vector3& inertia);
};

}


InertiaCalculator::InertiaCalculator()
{
    impl = new Impl;
}


InertiaCalculator::Impl::Impl()
    : mv(new MessageView)
{
    setWindowTitle(_("InertiaCalculator"));
    setFixedWidth(500);

    for(int i = 0; i < NUM_COMBOS; ++i) {
        combos[i] = new ComboBox;
    }

    const QStringList shapeList = { _("Box"), _("Sphere"), _("Cylinder"), _("Cone") };
    combos[SHAPE]->addItems(shapeList);
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(_("Shape")));
    hbox->addWidget(combos[SHAPE]);
    hbox->addStretch();

    topWidget = new QStackedWidget;
    QGridLayout* gbox[NUM_PAGES];
    for(int i = 0; i < NUM_PAGES; ++i) {
        Widget* pageWidget = new Widget;
        gbox[i] = new QGridLayout;
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addLayout(gbox[i]);
        vbox->addStretch();
        pageWidget->setLayout(vbox);
        topWidget->addWidget(pageWidget);
    }

    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        DoubleSpinInfo info = dspinInfo[i];
        dspin->setDecimals(7);
        dspin->setSingleStep(0.01);
        dspin->setRange(0.000, 9999.999);
        gbox[info.page]->addWidget(dspin, info.row, info.column);
    }

    static const char* label[] = {
        _("mass [kg]"), _("x [m]"), _("y [m]"), _("z [m]"),
        _("mass [kg]"), _("radius [m]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]"), _("axis [-]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]"), _("axis [-]")
    };

    for(int i = 0; i < 14; ++i) {
        LabelInfo info = labelInfo[i];
        gbox[info.page]->addWidget(new QLabel(label[i]), info.row, info.column);
    }

    const QStringList axisList = { "x", "y", "z" };
    combos[CLD_AXIS]->addItems(axisList);
    combos[CON_AXIS]->addItems(axisList);
    gbox[CYLINDER]->addWidget(combos[CLD_AXIS], 3, 1);
    gbox[CONE]->addWidget(combos[CON_AXIS], 3, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    PushButton* calcButton = new PushButton(_("&Calc"));
    buttonBox->addButton(calcButton, QDialogButtonBox::ActionRole);

    calcButton->sigClicked().connect([&](){ onCalcButtonClicked(); });
    combos[SHAPE]->sigCurrentIndexChanged().connect(
                [&](int index){ topWidget->setCurrentIndex(index); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(topWidget);
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
    mm.addItem(_("Calculate Inertia"))->sigTriggered().connect([&](){
        calculatorInstance->impl->mv->clear();
        calculatorInstance->impl->show();
    });
}


void InertiaCalculator::Impl::onCalcButtonClicked()
{
    int index = combos[SHAPE]->currentIndex();
    if(index == BOX) {
        calcBoxInertia();
    } else if(index == SPHERE) {
        calcSphereInertia();
    } else if(index == CYLINDER) {
        calcCylinderInertia();
    } else if(index == CONE) {
        calcConeInertia();
    }
}


void InertiaCalculator::Impl::calcBoxInertia()
{
    double m = dspins[BOX_MAS]->value();
    double x = dspins[BOX_X]->value();
    double y = dspins[BOX_Y]->value();
    double z = dspins[BOX_Z]->value();
    double ix = m / 12.0 * (y * y + z * z);
    double iy = m / 12.0 * (z * z + x * x);
    double iz = m / 12.0 * (x * x + y * y);

    mv->putln(fmt::format(_("shape: Box, mass: {0} [kg], x: {1} [m], y: {2} [m], z: {3} [m]"),
                                   m, x, y, z));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculator::Impl::calcSphereInertia()
{
    double m = dspins[SPR_MAS]->value();
    double r = dspins[SPR_RAD]->value();
    double ix, iy, iz;
    ix = iy = iz = m * r * r / 5.0 * 2.0;

    mv->putln(fmt::format(_("shape: Sphere, mass: {0} [kg], radius: {1} [m]"),
                                   m, r));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculator::Impl::calcCylinderInertia()
{
    double m = dspins[CLD_MAS]->value();
    double r = dspins[CLD_RAD]->value();
    double h = dspins[CLD_HGT]->value();
    int index = combos[CLD_AXIS]->currentIndex();
    double mainInertia = m * r * r / 2.0;
    double subInertia = m * (3.0 * r * r + h * h) / 12.0;
    double ix, iy, iz;

    if(index == X) {
        ix = mainInertia;
        iy = iz = subInertia;
    } else if(index == Y) {
        iy = mainInertia;
        iz = ix = subInertia;
    } else if(index == Z) {
        iz = mainInertia;
        ix = iy = subInertia;
    }

    mv->putln(fmt::format(_("shape: Cylinder, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   m, r, h, combos[CLD_AXIS]->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculator::Impl::calcConeInertia()
{
    double m = dspins[CON_MAS]->value();
    double r = dspins[CON_RAD]->value();
    double h = dspins[CON_HGT]->value();
    int index = combos[CON_AXIS]->currentIndex();
    double mainInertia = m * r * r * 3.0 / 10.0;
    double subInertia = m * 3.0 / 80.0 * (4.0 * r * r + h * h);
    double ix, iy, iz;

    if(index == X) {
        ix = mainInertia;
        iy = iz = subInertia;
    } else if(index == Y) {
        iy = mainInertia;
        iz = ix = subInertia;
    } else if(index == Z) {
        iz = mainInertia;
        ix = iy = subInertia;
    }

    mv->putln(fmt::format(_("shape: Cone, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                   m, r, h, combos[CON_AXIS]->currentText().toStdString()));
    printIntertia(Vector3(ix, iy, iz));
}


void InertiaCalculator::Impl::printIntertia(const Vector3& inertia)
{
    mv->putln(fmt::format(_("inertia: [ {0}, 0, 0, 0, {1}, 0, 0, 0, {2} ]\n"),
                          inertia[0], inertia[1], inertia[2]));
}
