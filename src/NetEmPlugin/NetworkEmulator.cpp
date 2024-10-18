/**
   @author Kenta Suzuki
*/

#include "NetworkEmulator.h"
#include <cnoid/Action>
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include "gettext.h"
#include "NetEm.h"

using namespace cnoid;

namespace {

class EmulatorDialog : public QDialog
{
public:
    EmulatorDialog(QWidget* parent = nullptr);

private:
    void on_startButton_toggled(bool checked);

    enum { In, Out, NumInterfaces };

    NetEmPtr emulator;
    QComboBox* interfaceComboBox;
    QComboBox* ifbdeviceComboBox;
    QDoubleSpinBox* delaySpinBoxes[NumInterfaces];
    QDoubleSpinBox* rateSpinBoxes[NumInterfaces];
    QDoubleSpinBox* lossSpinBoxes[NumInterfaces];
    QPushButton* startButton;
    QDialogButtonBox* buttonBox;
};

}


void NetworkEmulator::initializeClass(ExtensionManager* ext)
{
    static EmulatorDialog* dialog = nullptr;

    if(!dialog) {
        dialog = ext->manage(new EmulatorDialog);

        MainMenu::instance()->add_Tools_Item(
            _("Network Emulator"), [](){ dialog->show(); });
    }
}


NetworkEmulator::NetworkEmulator()
{

}


NetworkEmulator::~NetworkEmulator()
{

}


EmulatorDialog::EmulatorDialog(QWidget* parent)
    : QDialog(parent)
{
    emulator = new NetEm;

    interfaceComboBox = new QComboBox;
    for(int i = 0; i < emulator->interfaces().size(); ++i) {
        interfaceComboBox->addItem(emulator->interfaces()[i].c_str());
    }

    ifbdeviceComboBox = new QComboBox;
    ifbdeviceComboBox->addItems(QStringList() << "ifb0" << "ifb1");
    ifbdeviceComboBox->setCurrentIndex(1);

    for(int i = 0; i < NumInterfaces; ++i) {
        delaySpinBoxes[i] = new QDoubleSpinBox;
        delaySpinBoxes[i]->setRange(0.0, 100000.0);
        rateSpinBoxes[i] = new QDoubleSpinBox;
        rateSpinBoxes[i]->setRange(0.0, 11000000.0);
        lossSpinBoxes[i] = new QDoubleSpinBox;
        lossSpinBoxes[i]->setRange(0.0, 100.0);
    }

    auto formLayout = new QFormLayout;
    formLayout->addRow(_("Interface"), interfaceComboBox);
    formLayout->addRow(_("IFB Device"), ifbdeviceComboBox);
    formLayout->addRow(_("Inbound Delay [ms]"), delaySpinBoxes[In]);
    formLayout->addRow(_("Inbound Rate [kbit/s]"), rateSpinBoxes[In]);
    formLayout->addRow(_("Inbound Loss [%]"), lossSpinBoxes[In]);
    formLayout->addRow(_("Outbound Delay [ms]"), delaySpinBoxes[Out]);
    formLayout->addRow(_("Outbound Rate [kbit/s]"), rateSpinBoxes[Out]);
    formLayout->addRow(_("Outbound Loss [%]"), lossSpinBoxes[Out]);

    buttonBox = new QDialogButtonBox(this);
    startButton = new QPushButton(_("&Start"));
    startButton->setCheckable(true);
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    connect(startButton, &QPushButton::toggled, [&](bool checked){ on_startButton_toggled(checked); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Network Emulator"));
}


void EmulatorDialog::on_startButton_toggled(bool checked)
{
    if(checked) {
        startButton->setText(_("&Stop"));
        emulator->start(interfaceComboBox->currentIndex(), ifbdeviceComboBox->currentIndex());
        for(int i = 0; i < NumInterfaces; ++i) {
            emulator->setDelay(i, delaySpinBoxes[i]->value());
            emulator->setRate(i, rateSpinBoxes[i]->value());
            emulator->setLoss(i, lossSpinBoxes[i]->value());
        }
        emulator->update();
    } else {
        startButton->setText(_("&Start"));
        emulator->stop();
    }
}