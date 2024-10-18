/**
    @author Kenta Suzuki
*/

#include "BodyLocator.h"
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include "gettext.h"

using namespace cnoid;

namespace {

class LocatorDialog : public QDialog
{
public:
    LocatorDialog(QWidget* parent = nullptr);

private:
    void on_xyzButton_clicked(const int& id);
    void on_rpyButton_clicked(const int& id);

    QDoubleSpinBox* distanceSpinBox;
    QDoubleSpinBox* angleSpinBox;
    QDialogButtonBox* buttonBox;
};

}


void BodyLocator::initializeClass(ExtensionManager* ext)
{
    static LocatorDialog* dialog = nullptr;

    if(!dialog) {
        dialog = ext->manage(new LocatorDialog);

        MainMenu::instance()->add_Tools_Item(
            _("Body Locator"), [](){ dialog->show(); });
    }
}


BodyLocator::BodyLocator()
{

}


BodyLocator::~BodyLocator()
{

}


LocatorDialog::LocatorDialog(QWidget* parent)
    : QDialog(parent)
{
    auto layout = new QHBoxLayout;

    distanceSpinBox = new QDoubleSpinBox;
    distanceSpinBox->setRange(0.0, 9999.0);
    distanceSpinBox->setValue(1.0);
    layout->addWidget(distanceSpinBox);
    layout->addWidget(new QLabel("[m]"));
    layout->addStretch();

    const QStringList list = { "x+", "x-", "y+", "y-", "z+", "z-" };
    for(int i = 0; i < 6; ++i) {
        auto xyzButton = new QToolButton;
        xyzButton->setText(list.at(i));
        connect(xyzButton, &QToolButton::clicked, [&, i](){ on_xyzButton_clicked(i); });
        layout->addWidget(xyzButton);
    }

    auto layout2 = new QHBoxLayout;

    angleSpinBox = new QDoubleSpinBox;
    angleSpinBox->setRange(0.0, 9999.0);
    angleSpinBox->setValue(10.0);
    layout2->addWidget(angleSpinBox);
    layout2->addWidget(new QLabel("[deg]"));
    layout2->addStretch();

    const QStringList list2 = { "r+", "r-", "p+", "p-", "y+", "y-" };
    for(int i = 0; i < 6; ++i) {
        auto rpyButton = new QToolButton;
        rpyButton->setText(list2.at(i));
        connect(rpyButton, &QToolButton::clicked, [&, i](){ on_rpyButton_clicked(i); });
        layout2->addWidget(rpyButton);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(layout2);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Body Locator"));
}


void LocatorDialog::on_xyzButton_clicked(const int& id)
{
    auto rootItem = RootItem::instance();
    int index1 = id / 2;
    int index2 = id % 2;
    double pos = distanceSpinBox->value();
    pos = index2 == 0 ? pos : pos * -1.0;

    ItemList<BodyItem> bodyItems = rootItem->selectedItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        auto body = bodyItem->body();

        Link* rootLink = body->rootLink();
        Vector3 p = rootLink->translation();
        p[index1] += pos;
        rootLink->setTranslation(p);
        bodyItem->notifyKinematicStateChange(true);
        bodyItem->storeInitialState();
    }
}


void LocatorDialog::on_rpyButton_clicked(const int& id)
{
    auto rootItem = RootItem::instance();
    int index1 = id / 2;
    int index2 = id % 2;
    double angle = angleSpinBox->value();
    angle = index2 == 0 ? angle : angle * -1.0;

    ItemList<BodyItem> bodyItems = rootItem->selectedItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        auto body = bodyItem->body();

        Link* rootLink = body->rootLink();
        Vector3 rpy = rpyFromRot(rootLink->R());
        rpy[index1] += radian(angle);
        rootLink->setRotation(rotFromRpy(rpy));
        bodyItem->notifyKinematicStateChange();
        bodyItem->storeInitialState();
    }
}