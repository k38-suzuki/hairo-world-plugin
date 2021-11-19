/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffector.h"
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <math.h>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class EffectConfigDialog : public Dialog
{
public:
    EffectConfigDialog();

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
    CheckBox* flipCheck;
    ComboBox* filterCombo;

    void onResetButtonClicked();
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};


class VisualEffectorImpl
{
public:
    VisualEffectorImpl(VisualEffector* self);
    VisualEffector* self;

    EffectConfigDialog* dialog;
};

}


VisualEffector::VisualEffector()
{
    impl = new VisualEffectorImpl(this);
}


VisualEffectorImpl::VisualEffectorImpl(VisualEffector* self)
    : self(self)
{
    dialog = new EffectConfigDialog();
}


VisualEffector::~VisualEffector()
{
    delete impl;
}


void VisualEffector::show()
{
    impl->dialog->show();
}


double VisualEffector::hue() const
{
    return impl->dialog->hueSpin->value();
}


double VisualEffector::saturation() const
{
    return impl->dialog->saturationSpin->value();
}


double VisualEffector::value() const
{
    return impl->dialog->valueSpin->value();
}


double VisualEffector::red() const
{
    return impl->dialog->redSpin->value();
}


double VisualEffector::green() const
{
    return impl->dialog->greenSpin->value();
}


double VisualEffector::blue() const
{
    return impl->dialog->blueSpin->value();
}


double VisualEffector::coefB() const
{
    return impl->dialog->coefBSpin->value();
}


double VisualEffector::coefD() const
{
    return impl->dialog->coefDSpin->value();
}


double VisualEffector::stdDev() const
{
    return impl->dialog->stdDevSpin->value();
}


double VisualEffector::salt() const
{
    return impl->dialog->saltSpin->value();
}


double VisualEffector::pepper() const
{
    return impl->dialog->pepperSpin->value();
}


bool VisualEffector::flip() const
{
    return impl->dialog->flipCheck->isChecked();
}


int VisualEffector::filter() const
{
    return impl->dialog->filterCombo->currentIndex();
}


EffectConfigDialog::EffectConfigDialog()
{
    setWindowTitle(_("Effect Config"));

    const double ranges[11][2] = {
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
    flipCheck = new CheckBox();
    filterCombo = new ComboBox();

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
    flipCheck->setText(_("Flip"));
    flipCheck->setChecked(false);
    QStringList filters = { _("No filter"), _("Gaussian 3x3"), _("Gaussian 5x5"), _("Sobel"), _("Prewitt") };
    filterCombo->addItems(filters);
    filterCombo->setCurrentIndex(0);

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
    gbox->addWidget(flipCheck, index, 0);
    gbox->addWidget(new QLabel(_("Filter")), index, 2);
    gbox->addWidget(filterCombo, index++, 3);

    PushButton* resetButton = new PushButton(_("&Reset"));
    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    resetButton->sigClicked().connect([&](){ onResetButtonClicked(); });
}


void EffectConfigDialog::onResetButtonClicked()
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
    flipCheck->setChecked(false);
    filterCombo->setCurrentIndex(0);
}


bool VisualEffector::store(Archive& archive)
{
    return impl->dialog->store(archive);
}


bool EffectConfigDialog::store(Archive& archive)
{
    archive.write("hue", hueSpin->value());
    archive.write("saturation", saturationSpin->value());
    archive.write("value", valueSpin->value());
    archive.write("red", redSpin->value());
    archive.write("green", greenSpin->value());
    archive.write("blue", blueSpin->value());
    archive.write("coef_b", coefBSpin->value());
    archive.write("coef_d", coefDSpin->value());
    archive.write("std_dev", stdDevSpin->value());
    archive.write("salt", saltSpin->value());
    archive.write("pepper", pepperSpin->value());
    archive.write("flip", flipCheck->isChecked());
    archive.write("filter", filterCombo->currentIndex());
    return true;
}


bool VisualEffector::restore(const Archive& archive)
{
    return impl->dialog->restore(archive);
}


bool EffectConfigDialog::restore(const Archive& archive)
{
    double value;
    archive.read("hue", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    hueSpin->setValue(value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    archive.read("saturation", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    saturationSpin->setValue(value);
    archive.read("value", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    valueSpin->setValue(value);
    archive.read("red", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    redSpin->setValue(value);
    archive.read("green", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    greenSpin->setValue(value);
    archive.read("blue", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    blueSpin->setValue(value);
    archive.read("coef_b", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    coefBSpin->setValue(value);
    archive.read("coef_d", value);
    if(fabs(value) < 1.0) {
        value = 1.0;
    }
    coefDSpin->setValue(value);
    archive.read("std_dev", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    stdDevSpin->setValue(value);
    archive.read("salt", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    saltSpin->setValue(value);
    archive.read("pepper", value);
    if(fabs(value) < 0.01) {
        value = 0.0;
    }
    pepperSpin->setValue(value);
    bool flip;
    archive.read("flip", flip);
    flipCheck->setChecked(flip);
    int filter = 0;
    archive.read("filter", filter);
    filterCombo->setCurrentIndex(filter);
    return true;
}
