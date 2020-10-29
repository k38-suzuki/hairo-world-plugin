/**
   \file
   \author Kenta Suzuki
*/

#include "VisualEffectDialog.h"
#include <cnoid/Button>
#include <cnoid/Separator>
#include <cnoid/Slider>
#include <cnoid/SpinBox>
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
    vector<DoubleSpinBox*> dspins;

    void onAccepted();
    void onRejected();
    void onClearButtonClicked();
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
    QVBoxLayout* vbox = new QVBoxLayout();

    HSeparatorBox* hsbox = new HSeparatorBox(new QLabel(_("Visual Effects")));
    vbox->addLayout(hsbox);

    QStringList labels = { _("Hue"), _("Saturation"), _("Value"),
                         _("Red"), _("Green"), _("Blue"),
                         _("CoefB"), _("CoefD"), _("Std_dev"),
                         _("Salt"), _("Pepper") };
    double ranges[11][2] = {
        { 0.0, 1.0 },
        { -1.0, 1.0 },
        { -1.0, 1.0 },
        { -1.0, 1.0 },
        { -1.0, 1.0 },
        { -1.0, 1.0 },
        { -1.0, 0.0 },
        { 1.0, 32.0 },
        { 0.0, 1.0 },
        { 0.0, 1.0 },
        { 0.0, 1.0 }
    };
    for(int i = 0; i < 11; i++) {
        QHBoxLayout* hbox = new QHBoxLayout();
        hbox->addWidget(new QLabel(_(labels[i].toStdString().c_str())));
        hbox->addStretch();
        DoubleSpinBox* dspinbox = new DoubleSpinBox();
        dspinbox->setRange(ranges[i][0], ranges[i][1]);
        dspinbox->setSingleStep(0.1);
        dspinbox->setFixedWidth(70);
        if(i == 7) {
            dspinbox->setValue(1.0);
        }
        dspins.push_back(dspinbox);
        hbox->addWidget(dspinbox);
        vbox->addLayout(hbox);
    }

    vbox->addWidget(new HSeparator());
    PushButton* clearButton = new PushButton(_("Clear"));
    clearButton->sigClicked().connect([&](){ onClearButtonClicked(); });
    vbox->addWidget(clearButton);
    self->setLayout(vbox);
}


VisualEffectDialog::~VisualEffectDialog()
{
    delete impl;
}


double VisualEffectDialog::value(const int index) const
{
    return impl->dspins[index]->value();
}


void VisualEffectDialog::setVisualEffect(VisualEffect* effect)
{
    impl->dspins[0]->setValue(effect->hue());
    impl->dspins[1]->setValue(effect->saturation());
    impl->dspins[2]->setValue(effect->value());
    impl->dspins[3]->setValue(effect->red());
    impl->dspins[4]->setValue(effect->green());
    impl->dspins[5]->setValue(effect->blue());
    impl->dspins[6]->setValue(effect->coefB());
    impl->dspins[7]->setValue(effect->coefD());
    impl->dspins[8]->setValue(effect->stdDev());
    impl->dspins[9]->setValue(effect->salt());
    impl->dspins[10]->setValue(effect->pepper());
}


void VisualEffectDialogImpl::onClearButtonClicked()
{
    for(int i = 0; i < 11; i++) {
        if(i == 7) {
            dspins[i]->setValue(1.0);
        }
        else {
            dspins[i]->setValue(0.0);
        }
    }
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
