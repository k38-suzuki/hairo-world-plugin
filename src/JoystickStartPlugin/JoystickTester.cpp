/**
   @author Kenta Suzuki
*/

#include "JoystickTester.h"
#include <cnoid/Buttons>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/JoystickCapture>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QStackedLayout>
#include <fcntl.h>
#include <linux/joystick.h>
#include <map>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = stdx::filesystem;

namespace {

const map<string, int> modelIdMap = {
    {                  "Sony Computer Entertainment Wireless Controller", 0},
    {               "Sony Interactive Entertainment Wireless Controller", 0},
    {                                              "Wireless Controller", 0},
    { "Sony Interactive Entertainment DUALSHOCK®4 USB Wireless Adaptor", 0},
    {                                  "Sony PLAYSTATION(R)3 Controller", 0},
    {     "Sony Interactive Entertainment DualSense Wireless Controller", 0},
    {                                    "DualSense Wireless Controller", 0},
    {"Sony Interactive Entertainment DualSense Edge Wireless Controller", 0},
    {                               "DualSense Edge Wireless Controller", 0},
    {                                          "Microsoft X-Box 360 pad", 0},
    {                                          "Microsoft X-Box One pad", 0},
    {                             "Microsoft Xbox Series S|X Controller", 0},
    {                                  "Microsoft X-Box One Elite 2 pad", 0},
    {                                         "Xbox Wireless Controller", 0},
    {                                            "Logitech Gamepad F310", 0},
    {                                            "Logitech Gamepad F710", 0},
    {                                    "Logicool Logicool Dual Action", 0}
};

class TesterWidget : public QWidget
{
public:
    TesterWidget(const QString& device, QWidget* parent = nullptr);

private: // slots
    void onAxis(int id, const double& position);
    void onButton(int id, bool isPressed);

private:
    QString device;

    JoystickCapture joystick;
    vector<QProgressBar*> bars;
    vector<QPushButton*> buttons;
};

class TesterDialog : public QDialog
{
public:
    TesterDialog(QWidget* parent = nullptr);

private: // slots
    void on_deviceComboBox_currentIndexChanged(int index);

private:
    void createTesterWidget(const QString& device);

    QComboBox* deviceComboBox;
    QStackedLayout* stackedLayout;
    QStringList models;
    QDialogButtonBox* buttonBox;
};

} // namespace

void JoystickTester::initializeClass(ExtensionManager* ext)
{
    static TesterDialog* dialog = nullptr;

    if(!dialog) {
        dialog = ext->manage(new TesterDialog);

        MainMenu::instance()->add_Tools_Item(_("Joystick Tester"), []() { dialog->show(); });
    }
}

JoystickTester::JoystickTester()
{
}

JoystickTester::~JoystickTester()
{
}

TesterWidget::TesterWidget(const QString& device, QWidget* parent)
    : QWidget(parent),
      device(device)
{
    joystick.setDevice(device.toStdString().c_str());

    joystick.sigAxis().connect([&](int id, double position) { onAxis(id, position); });

    joystick.sigButton().connect([&](int id, bool isPressed) { onButton(id, isPressed); });

    auto groupBox = new QGroupBox(_("Axes"));
    auto vbox = new QVBoxLayout;
    auto gridLayout = new QGridLayout;
    for(int i = 0; i < joystick.numAxes(); ++i) {
        QProgressBar* bar = new QProgressBar;
        bar->setValue(0);
        bar->setRange(-100, 100);
        bar->setFormat(formatC("{0:.3}%", 0.0).c_str());
        bars.push_back(bar);
        const string label = "Axis " + to_string(i) + ":";
        gridLayout->addWidget(new QLabel(label.c_str()), i, 0);
        gridLayout->addWidget(bar, i, 1);
    }
    vbox->addLayout(gridLayout);
    vbox->addStretch();
    groupBox->setLayout(vbox);

    auto groupBox2 = new QGroupBox(_("Buttons"));
    vbox = new QVBoxLayout;
    for(int i = 0; i < joystick.numButtons(); ++i) {
        const string label = "Button " + to_string(i);
        PushButton* button = new PushButton(label.c_str());
        buttons.push_back(button);
        vbox->addWidget(button);
    }
    vbox->addStretch();
    groupBox2->setLayout(vbox);

    auto layout = new QHBoxLayout;
    layout->addWidget(groupBox);
    layout->addWidget(groupBox2);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    setLayout(mainLayout);
}

void TesterWidget::onAxis(int id, const double& position)
{
    QProgressBar* bar = bars[id];
    double value = 100.0 * position;
    bar->setValue(value);
    bar->setFormat(formatC("{0:.3}%", value).c_str());
}

void TesterWidget::onButton(int id, bool isPressed)
{
    QPalette palette;
    if(isPressed) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
    }
    buttons[id]->setPalette(palette);
}

TesterDialog::TesterDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(_("Joystick Tester"));

    deviceComboBox = new QComboBox(this);
    connect(deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        on_deviceComboBox_currentIndexChanged(index);
    });

    stackedLayout = new QStackedLayout;

    int id = 0;
    while(true) {
        string file = formatC("/dev/input/js{}", id);
        if(!filesystem::exists(filesystem::path(file))) {
            break;
        }
        int tmpfd = ::open(file.c_str(), O_RDONLY | O_NONBLOCK);
        if(tmpfd < 0) {
            break;
        }

        // check the josytick model name
        char buf[1024];
        if(ioctl(tmpfd, JSIOCGNAME(sizeof(buf)), buf) < 0) {
            ::close(tmpfd);
            break;
        }
        string identifier(buf);
        models << identifier.c_str();
        createTesterWidget(file.c_str());

        ++id;
        // break;
    }

    auto layout = new QHBoxLayout;
    layout->addWidget(new QLabel(_("Device")));
    layout->addWidget(deviceComboBox);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&]() { this->accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(stackedLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

void TesterDialog::on_deviceComboBox_currentIndexChanged(int index)
{
    stackedLayout->setCurrentIndex(index);
    string model_name = models.at(index).toStdString();

    int ret = -1;
    auto iter = modelIdMap.find(model_name);
    if(iter != modelIdMap.end()) {
        ret = iter->second;
    }
    model_name += ret < 0 ? " (Unsupported)" : " (Supported)";
    setWindowTitle(model_name.c_str());
}

void TesterDialog::createTesterWidget(const QString& device)
{
    deviceComboBox->addItem(device);
    stackedLayout->addWidget(new TesterWidget(device.toStdString().c_str()));
}