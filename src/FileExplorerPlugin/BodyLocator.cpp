/**
    @author Kenta Suzuki
 */

#include "BodyLocator.h"
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/Process>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

BodyLocator* locatorInstance = nullptr;

}

namespace cnoid {

class BodyLocator::Impl : public Dialog
{
public:

    Impl();
    ~Impl();

    void onTranslationButtonClicked(const int& id);
    void onRotationButtonClocked(const int& id);

    DoubleSpinBox* distanceSpin;
    DoubleSpinBox* angleSpin;
    QDialogButtonBox* buttonBox;
};

}


void BodyLocator::initializeClass(ExtensionManager* ext)
{
    if(!locatorInstance) {
        locatorInstance = ext->manage(new BodyLocator);
    }

    ItemTreeView::customizeContextMenu<BodyItem>(
        [&](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("Open"));
            menuManager.addItem(_("gedit"))->sigTriggered().connect(
                [&, item](){
                    QProcess::startDetached("gedit",
                        QStringList() << item->filePath().c_str()); });
            menuManager.addItem(_("Nautilus"))->sigTriggered().connect(
                [&, item](){
                    filesystem::path path(fromUTF8(item->filePath()));
                    QProcess::startDetached("nautilus",
                        QStringList() << path.parent_path().string().c_str()); });
            menuManager.setPath("/");
            menuManager.addItem(_("Body Locator"))->sigTriggered().connect(
                [&, item](){ locatorInstance->impl->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


BodyLocator* BodyLocator::instance()
{
    return locatorInstance;
}


BodyLocator::BodyLocator()
{
    impl = new Impl;
}


BodyLocator::Impl::Impl()
    : Dialog()
{
    auto layout = new QHBoxLayout;

    distanceSpin = new DoubleSpinBox;
    distanceSpin->setRange(0.0, 9999.0);
    distanceSpin->setValue(1.0);
    layout->addWidget(distanceSpin);
    layout->addWidget(new QLabel("[m]"));
    layout->addStretch();

    const QStringList list = { "x+", "x-", "y+", "y-", "z+", "z-" };
    for(int i = 0; i < 6; ++i) {
        ToolButton* button = new ToolButton;
        button->setText(list.at(i));
        button->sigClicked().connect([this, i](){ onTranslationButtonClicked(i); });
        layout->addWidget(button);
    }

    auto layout2 = new QHBoxLayout;

    angleSpin = new DoubleSpinBox;
    angleSpin->setRange(0.0, 9999.0);
    angleSpin->setValue(10.0);
    layout2->addWidget(angleSpin);
    layout2->addWidget(new QLabel("[deg]"));
    layout2->addStretch();

    const QStringList list2 = { "r+", "r-", "p+", "p-", "y+", "y-" };
    for(int i = 0; i < 6; ++i) {
        ToolButton* button = new ToolButton;
        button->setText(list2.at(i));
        button->sigClicked().connect([this, i](){ onRotationButtonClocked(i); });
        layout2->addWidget(button);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(layout2);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_("Body Locator"));
}


BodyLocator::~BodyLocator()
{
    delete impl;
}


BodyLocator::Impl::~Impl()
{

}


void BodyLocator::Impl::onTranslationButtonClicked(const int& id)
{
    auto rootItem = RootItem::instance();
    int index1 = id / 2;
    int index2 = id % 2;
    double pos = distanceSpin->value();
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


void BodyLocator::Impl::onRotationButtonClocked(const int& id)
{
    auto rootItem = RootItem::instance();
    int index1 = id / 2;
    int index2 = id % 2;
    double angle = angleSpin->value();
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