/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/Buttons>
#include <cnoid/ComboBox>
#include <cnoid/EigenTypes>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/MessageView>
#include <cnoid/SpinBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTabWidget>
#include "BodyCreatorDialog.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

struct DoubleSpinInfo {
    int page;
    QDoubleSpinBox* spin;
};

DoubleSpinInfo doubleSpinInfo[] = {
    { 0, nullptr }, { 0, nullptr }, { 0, nullptr }, { 0, nullptr },
    { 1, nullptr }, { 1, nullptr },
    { 2, nullptr }, { 2, nullptr }, { 2, nullptr },
    { 3, nullptr }, { 3, nullptr }, { 3, nullptr }
};

class CalculatorWidget : public QWidget
{
public:
    CalculatorWidget(QWidget* parent = nullptr);

private:
    void calc();

    enum {
        BOX_MAS, BOX_X, BOX_Y, BOX_Z,
        SPR_MAS, SPR_RAD,
        CLD_MAS, CLD_RAD, CLD_HGT,
        CON_MAS, CON_RAD, CON_HGT,
        NumDoubleSpinBoxes
    };
    enum { Page_Box, Page_Sphere, Page_Cylinder, Page_Cone, NumPages };
    enum Axis { XAxis, YAxis, ZAxis };

    MessageView* mv;

    QComboBox* axisComboBoxes[2];
    QDoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    QTabWidget* tabWidget;
};

}


void InertiaCalculator::initializeClass(ExtensionManager* ext)
{
    static CalculatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new CalculatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Inertia Calculator"), widget);
    }
}


InertiaCalculator::InertiaCalculator()
{

}


InertiaCalculator::~InertiaCalculator()
{

}


CalculatorWidget::CalculatorWidget(QWidget* parent)
    : QWidget(parent),
      mv(new MessageView)
{
    mv->clear();

    tabWidget = new QTabWidget;

    const QStringList texts = { _("Box"), _("Sphere"), _("Cylinder"), _("Cone") };
    const QStringList texts2 = { "X", "Y", "Z" };

    for(int i = 0; i < 2; ++i) {
        axisComboBoxes[i] = new QComboBox;
        axisComboBoxes[i]->addItems(texts2);
    }

    QFormLayout* formLayout[NumPages];
    for(int i = 0; i < NumPages; ++i) {
        auto page = new QWidget;
        formLayout[i] = new QFormLayout;
        auto layout = new QVBoxLayout;
        layout->addLayout(formLayout[i]);
        layout->addStretch();
        page->setLayout(layout);
        tabWidget->addTab(page, texts.at(i));
    }

    const QStringList list = {
        _("mass [kg]"), _("x [m]"), _("y [m]"), _("z [m]"),
        _("mass [kg]"), _("radius [m]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]"),
        _("mass [kg]"), _("radius [m]"), _("height [m]")
    };

    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        info.spin = doubleSpinBoxes[i] = new QDoubleSpinBox;
        info.spin->setDecimals(7);
        info.spin->setSingleStep(0.01);
        info.spin->setRange(0.0, 9999.999);
        formLayout[info.page]->addRow(list[i], info.spin);
    }

    formLayout[Page_Cylinder]->addRow(_("axis [-]"), axisComboBoxes[0]);
    formLayout[Page_Cone]->addRow(_("axis [-]"), axisComboBoxes[1]);

    const QIcon calcIcon = QIcon::fromTheme("accessories-calculator");
    auto calcButton = new QPushButton(calcIcon, _("&Calc"), this);
    connect(calcButton, &QPushButton::clicked, [&](){ calc(); });

    const QIcon clearIcon = QIcon::fromTheme("edit-clear");
    auto clearButton = new QPushButton(clearIcon, _("C&lear"), this);
    connect(clearButton, &QPushButton::clicked, [&](){ mv->clear(); });

    auto layout = new QHBoxLayout;
    layout->addWidget(calcButton);
    layout->addWidget(clearButton);
    layout->addStretch();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(mv);
    // mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("Inertia Calculator"));
}


void CalculatorWidget::calc()
{
    double ix, iy, iz = 0.0;
    int index = tabWidget->currentIndex();
    if(index == Page_Box) {
        double m = doubleSpinBoxes[BOX_MAS]->value();
        double x = doubleSpinBoxes[BOX_X]->value();
        double y = doubleSpinBoxes[BOX_Y]->value();
        double z = doubleSpinBoxes[BOX_Z]->value();

        ix = m / 12.0 * (y * y + z * z);
        iy = m / 12.0 * (z * z + x * x);
        iz = m / 12.0 * (x * x + y * y);

        mv->putln(formatR(_("shape: Box, mass: {0} [kg], x: {1} [m], y: {2} [m], z: {3} [m]"),
                                    m, x, y, z));
    } else if(index == Page_Sphere) {
        double m = doubleSpinBoxes[SPR_MAS]->value();
        double r = doubleSpinBoxes[SPR_RAD]->value();

        ix = iy = iz = m * r * r / 5.0 * 2.0;

        mv->putln(formatR(_("shape: Sphere, mass: {0} [kg], radius: {1} [m]"),
                                    m, r));
    } else if(index == Page_Cylinder) {
        double m = doubleSpinBoxes[CLD_MAS]->value();
        double r = doubleSpinBoxes[CLD_RAD]->value();
        double h = doubleSpinBoxes[CLD_HGT]->value();
        int index = axisComboBoxes[0]->currentIndex();

        double main_inertia = m * r * r / 2.0;
        double sub_inertia = m * (3.0 * r * r + h * h) / 12.0;

        if(index == XAxis) {
            ix = main_inertia;
            iy = iz = sub_inertia;
        } else if(index == YAxis) {
            iy = main_inertia;
            iz = ix = sub_inertia;
        } else if(index == ZAxis) {
            iz = main_inertia;
            ix = iy = sub_inertia;
        }

        mv->putln(formatR(_("shape: Cylinder, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                    m, r, h, axisComboBoxes[0]->currentText().toStdString()));
    } else if(index == Page_Cone) {
        double m = doubleSpinBoxes[CON_MAS]->value();
        double r = doubleSpinBoxes[CON_RAD]->value();
        double h = doubleSpinBoxes[CON_HGT]->value();
        int index = axisComboBoxes[1]->currentIndex();

        double main_inertia = m * r * r * 3.0 / 10.0;
        double sub_inertia = m * 3.0 / 80.0 * (4.0 * r * r + h * h);

        if(index == XAxis) {
            ix = main_inertia;
            iy = iz = sub_inertia;
        } else if(index == YAxis) {
            iy = main_inertia;
            iz = ix = sub_inertia;
        } else if(index == ZAxis) {
            iz = main_inertia;
            ix = iy = sub_inertia;
        }

        mv->putln(formatR(_("shape: Cone, mass: {0} [kg], radius: {1} [m], height: {2} [m], axis: {3} [-]"),
                                    m, r, h, axisComboBoxes[1]->currentText().toStdString()));
    }

    Vector3 inertia(ix, iy, iz);
    mv->putln(formatR(_("inertia: [ {0}, 0, 0, 0, {1}, 0, 0, 0, {2} ]\n"),
                          inertia[0], inertia[1], inertia[2]));
}