/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffectDialog.h"
#include <cnoid/Button>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class VisualEffectDialogImpl
{
public:
    VisualEffectDialogImpl(VisualEffectDialog* self);
    VisualEffectDialog* self;

    DoubleSpinBox* hueSpin;
    DoubleSpinBox* saturationSpin;
    DoubleSpinBox* valueSpin;
    DoubleSpinBox* redSpin;
    DoubleSpinBox* greenSpin;
    DoubleSpinBox* blueSpin;
    DoubleSpinBox* coefBSpin;
    DoubleSpinBox* coefDSpin;
    DoubleSpinBox* stdDevSpin;
    DoubleSpinBox* saltSpin;
    DoubleSpinBox* pepperSpin;

    void onAccepted();
    void onRejected();
    void onResetButtonClicked();
};

}


VisualEffectDialog::VisualEffectDialog()
{
    impl = new VisualEffectDialogImpl(this);
}


VisualEffectDialogImpl::VisualEffectDialogImpl(VisualEffectDialog* self)
    : self(self)
{
    self->setWindowTitle(_("Configuration"));

    double ranges[11][2] = {
        {  0.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
        { -1.0, 1.0 }, { -1.0,  1.0 }, { -1.0, 1.0 },
        { -1.0, 0.0 }, {  1.0, 32.0 },
        {  0.0, 1.0 }, {  0.0,  1.0 }, {  0.0, 1.0 }
    };

    hueSpin = new DoubleSpinBox();
    saturationSpin = new DoubleSpinBox();
    valueSpin = new DoubleSpinBox();
    redSpin = new DoubleSpinBox();
    greenSpin = new DoubleSpinBox();
    blueSpin = new DoubleSpinBox();
    coefBSpin = new DoubleSpinBox();
    coefDSpin = new DoubleSpinBox();
    stdDevSpin = new DoubleSpinBox();
    saltSpin = new DoubleSpinBox();
    pepperSpin = new DoubleSpinBox();

    vector<DoubleSpinBox*> dspins;
    dspins.push_back(hueSpin);
    dspins.push_back(saturationSpin);
    dspins.push_back(valueSpin);
    dspins.push_back(redSpin);
    dspins.push_back(greenSpin);
    dspins.push_back(blueSpin);
    dspins.push_back(coefBSpin);
    dspins.push_back(coefDSpin);
    dspins.push_back(stdDevSpin);
    dspins.push_back(saltSpin);
    dspins.push_back(pepperSpin);

    for(size_t i = 0; i < dspins.size(); ++i) {
        DoubleSpinBox* dspin = dspins[i];
        dspin->setRange(ranges[i][0], ranges[i][1]);
        dspin->setSingleStep(0.1);
    }
    coefDSpin->setValue(1.0);

    QGridLayout* gbox = new QGridLayout();
    int index = 0;
    gbox->addWidget(new QLabel(_("Hue")), index, 0);
    gbox->addWidget(hueSpin, index, 1);
    gbox->addWidget(new QLabel(_("Saturation")), index, 2);
    gbox->addWidget(saturationSpin, index, 3);
    gbox->addWidget(new QLabel(_("Value")), index, 4);
    gbox->addWidget(valueSpin, index++, 5);
    gbox->addWidget(new QLabel(_("Red")), index, 0);
    gbox->addWidget(redSpin, index, 1);
    gbox->addWidget(new QLabel(_("Green")), index, 2);
    gbox->addWidget(greenSpin, index, 3);
    gbox->addWidget(new QLabel(_("Blue")), index, 4);
    gbox->addWidget(blueSpin, index++, 5);
    gbox->addWidget(new QLabel(_("CoefB")), index, 0);
    gbox->addWidget(coefBSpin, index, 1);
    gbox->addWidget(new QLabel(_("CoefD")), index, 2);
    gbox->addWidget(coefDSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Std_dev")), index, 0);
    gbox->addWidget(stdDevSpin, index, 1);
    gbox->addWidget(new QLabel(_("Salt")), index, 2);
    gbox->addWidget(saltSpin, index, 3);
    gbox->addWidget(new QLabel(_("Pepper")), index, 4);
    gbox->addWidget(pepperSpin, index++, 5);

    PushButton* resetButton = new PushButton(_("&Reset"));
    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    QVBoxLayout* vbox = new QVBoxLayout();
    HSeparatorBox* hsbox = new HSeparatorBox(new QLabel(_("Visual Effects")));
    vbox->addLayout(hsbox);
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    resetButton->sigClicked().connect([&](){ onResetButtonClicked(); });
}


VisualEffectDialog::~VisualEffectDialog()
{
    delete impl;
}


void VisualEffectDialog::setVisualEffect(VisualEffect* effect)
{
    impl->hueSpin->setValue(effect->hue());
    impl->saturationSpin->setValue(effect->saturation());
    impl->valueSpin->setValue(effect->value());
    impl->redSpin->setValue(effect->red());
    impl->greenSpin->setValue(effect->green());
    impl->blueSpin->setValue(effect->blue());
    impl->coefBSpin->setValue(effect->coefB());
    impl->coefDSpin->setValue(effect->coefD());
    impl->stdDevSpin->setValue(effect->stdDev());
    impl->saltSpin->setValue(effect->salt());
    impl->pepperSpin->setValue(effect->pepper());
}


double VisualEffectDialog::hue() const
{
    return impl->hueSpin->value();
}


double VisualEffectDialog::saturation() const
{
    return impl->saturationSpin->value();
}


double VisualEffectDialog::value() const
{
    return impl->valueSpin->value();
}


double VisualEffectDialog::red() const
{
    return impl->redSpin->value();
}


double VisualEffectDialog::green() const
{
    return impl->greenSpin->value();
}


double VisualEffectDialog::blue() const
{
    return impl->blueSpin->value();
}


double VisualEffectDialog::coefB() const
{
    return impl->coefBSpin->value();
}


double VisualEffectDialog::coefD() const
{
    return impl->coefDSpin->value();
}


double VisualEffectDialog::stdDev() const
{
    return impl->stdDevSpin->value();
}


double VisualEffectDialog::salt() const
{
    return impl->saltSpin->value();
}


double VisualEffectDialog::pepper() const
{
    return impl->pepperSpin->value();
}


void VisualEffectDialogImpl::onResetButtonClicked()
{
    hueSpin->setValue(0.0);
    saturationSpin->setValue(0.0);
    valueSpin->setValue(0.0);
    redSpin->setValue(0.0);
    greenSpin->setValue(0.0);
    blueSpin->setValue(0.0);
    coefBSpin->setValue(0.0);
    coefDSpin->setValue(1.0);
    stdDevSpin->setValue(0.0);
    saltSpin->setValue(0.0);
    pepperSpin->setValue(0.0);
}


void VisualEffectDialog::onAccepted()
{
    impl->onAccepted();
}


void VisualEffectDialogImpl::onAccepted()
{

}


void VisualEffectDialog::onRejected()
{
    impl->onRejected();
}


void VisualEffectDialogImpl::onRejected()
{

}
