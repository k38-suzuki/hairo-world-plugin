/**
   @author Kenta Suzuki
*/

#include "LayoutSwitcher.h"
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/LineEdit>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/ValueTree>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <algorithm>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

LayoutSwitcher* switcherInstance = nullptr;

class LayoutButton : public ToolButton
{
public:
    function<void(QMouseEvent* event)> callbackOnContextMenuRrequest;
    
    LayoutButton(const string& caption);
    virtual void mousePressEvent(QMouseEvent* event) override;
};

class LayoutInfo : public Referenced
{
public:
    string name;
    MappingPtr layoutData;
    LayoutButton* button;

    ~LayoutInfo();
};

typedef ref_ptr<LayoutInfo> LayoutInfoPtr;

class NewLayoutDialog : public Dialog
{
public:
    LayoutSwitcherImpl* impl;
    LineEdit nameEdit;

    NewLayoutDialog(LayoutSwitcherImpl* impl);
    virtual void onAccepted() override;
};

}

namespace cnoid {

class LayoutSwitcherImpl
{
public:
    LayoutSwitcher* self;

    LayoutSwitcherImpl(LayoutSwitcher* self);

    vector<LayoutInfoPtr> layoutInfos;
    MenuManager menuManager;
    QHBoxLayout* buttonBox;
    NewLayoutDialog* dialog;

    void onAddButtonClicked();
    void adjustSize();
    void storeCurrentLayoutAs(const string& name, MappingPtr layoutData);
    void restoreLayout(LayoutInfo* info);
    void onLayoutContextMenuRequest(LayoutInfo* info, QMouseEvent* event);
    void updateLayout(LayoutInfo* info);
    void removeLayout(LayoutInfo* info);
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


LayoutButton::LayoutButton(const string& caption)
    : ToolButton(caption.c_str())
{
    setAutoRaise(true);
}


void LayoutButton::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        callbackOnContextMenuRrequest(event);
    }
    ToolButton::mousePressEvent(event);
}


LayoutInfo::~LayoutInfo()
{
    delete button;
}


NewLayoutDialog::NewLayoutDialog(LayoutSwitcherImpl* impl)
    : impl(impl)
{
    setWindowTitle(_("Register Layout"));
    auto vbox = new QVBoxLayout;
    setLayout(vbox);
    auto hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(_("Name:")));
    nameEdit.setText(_("Layout"));
    hbox->addWidget(&nameEdit);
    vbox->addLayout(hbox);

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Register"));
    auto cancelButton = new PushButton(_("&Cancel"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [this](){ this->reject(); });
    vbox->addWidget(buttonBox);
}


LayoutSwitcher::LayoutSwitcher()
{
    impl = new LayoutSwitcherImpl(this);
    switcherInstance = this;
}


LayoutSwitcherImpl::LayoutSwitcherImpl(LayoutSwitcher* self)
    : self(self)
{
    auto hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(0);
    buttonBox = new QHBoxLayout;
    hbox->addLayout(buttonBox);
    auto addButton = new ToolButton("+");
    addButton->setAutoRaise(true);
    addButton->sigClicked().connect([&](){ onAddButtonClicked(); });
    hbox->addWidget(addButton);
    self->setLayout(hbox);

    dialog = nullptr;
}


LayoutSwitcher::~LayoutSwitcher()
{
    delete impl;
}


void LayoutSwitcherImpl::onAddButtonClicked()
{
    if(!dialog) {
        dialog = new NewLayoutDialog(this);
    }
    dialog->show();
}


void NewLayoutDialog::onAccepted()
{
    auto name = nameEdit.text().toStdString();
    if(!name.empty()) {
        auto layoutData = ProjectManager::instance()->storeCurrentLayout();
        if(layoutData) {
            impl->storeCurrentLayoutAs(name, layoutData);
        }
    }
}


void LayoutSwitcherImpl::adjustSize()
{
    auto parent = self->parentWidget();
    while(parent) {
        if(auto menuBar = dynamic_cast<QMenuBar*>(parent)) {
            menuBar->adjustSize();
            break;
        }
        parent = parent->parentWidget();
    }
}


void LayoutSwitcherImpl::storeCurrentLayoutAs(const string& name, MappingPtr layoutData)
{
    auto info = new LayoutInfo;
    info->name = name;
    info->layoutData = layoutData;
    auto button = new LayoutButton(name);
    button->sigClicked().connect(
        [this, info](){ restoreLayout(info); });
    button->callbackOnContextMenuRrequest =
        [this, info](QMouseEvent* event){ onLayoutContextMenuRequest(info, event); };
    buttonBox->addWidget(button);
    info->button = button;

    adjustSize();

    layoutInfos.push_back(info);
}


void LayoutSwitcherImpl::restoreLayout(LayoutInfo* info)
{
    ProjectManager::instance()->restoreLayout(info->layoutData);
}


void LayoutSwitcherImpl::onLayoutContextMenuRequest(LayoutInfo* info, QMouseEvent* event)
{
    menuManager.setNewPopupMenu(self);
    menuManager.addItem("Update")->sigTriggered().connect(
        [this, info](){ updateLayout(info); });
    menuManager.addItem("Remove")->sigTriggered().connect(
        [this, info](){ removeLayout(info); });
    menuManager.popupMenu()->popup(event->globalPos());
    //menuManager.popupMenu()->popup(button->mapToGlobal(QPoint(0,0)));
}


void LayoutSwitcherImpl::updateLayout(LayoutInfo* info)
{
    if(auto layoutData = ProjectManager::instance()->storeCurrentLayout()) {
        info->layoutData = layoutData;
    }
}


void LayoutSwitcherImpl::removeLayout(LayoutInfo* info)
{
    auto& infos = layoutInfos;
    infos.erase(remove(infos.begin(), infos.end(), info), infos.end());
    adjustSize();
}


bool LayoutSwitcher::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool LayoutSwitcherImpl::storeState(Archive& archive)
{
    ListingPtr layoutInfoListing = new Listing;

    for(auto& info : layoutInfos) {
        ArchivePtr subArchive = new Archive;
        subArchive->write("name", info->name);
        subArchive->write("layout_data", info->layoutData);

        layoutInfoListing->append(subArchive);
    }

    archive.insert("layouts", layoutInfoListing);

    return true;
}


bool LayoutSwitcher::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool LayoutSwitcherImpl::restoreState(const Archive& archive)
{
    int size = layoutInfos.size();
    for(int i = 0; i < size; ++i) {
        LayoutInfoPtr info = layoutInfos[size - 1 - i];
        removeLayout(info);
    }

    ListingPtr layoutInfoListing = archive.findListing("layouts");
    if(layoutInfoListing->isValid()) {
        for(int i = 0; i < layoutInfoListing->size(); ++i) {
            auto subArchive = archive.subArchive(layoutInfoListing->at(i)->toMapping());
            string name;
            subArchive->read("name", name);
            MappingPtr layoutData = subArchive->findMapping("layout_data")->toMapping();
            storeCurrentLayoutAs(name, layoutData);
        }
    }
    return true;
}
