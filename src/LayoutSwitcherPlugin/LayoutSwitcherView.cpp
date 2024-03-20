/**
   @author Kenta Suzuki
*/

#include "LayoutSwitcherView.h"
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QScrollArea>
#include <QVBoxLayout>
#include "LayoutSwitcher.h"
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class LayoutSwitcherView::Impl
{
public:
    LayoutSwitcherView* self;

    Impl(LayoutSwitcherView* self);

    QScrollArea scrollArea;

    LayoutSwitcher* layoutSwicher;

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

class LayoutSwitcherBar::Impl
{
public:
    LayoutSwitcherBar* self;

    Impl(LayoutSwitcherBar* self);

    LayoutSwitcher* layoutSwitcher;

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


LayoutSwitcherView* LayoutSwitcherView::instance()
{
    static LayoutSwitcherView* instance_ = ViewManager::findView<LayoutSwitcherView>();
    return instance_;
}


LayoutSwitcherView::LayoutSwitcherView()
{
    impl = new Impl(this);
}


LayoutSwitcherView::Impl::Impl(LayoutSwitcherView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::BottomCenterArea);

    QWidget* topWidget = new QWidget;
    topWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    QVBoxLayout* topVBox = new QVBoxLayout;
    //topVBox->setContentsMargins(4);
    topWidget->setLayout(topVBox);

    scrollArea.setStyleSheet("QScrollArea {background: transparent;}");
    scrollArea.setFrameShape(QFrame::NoFrame);
    scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea.setWidget(topWidget);
    topWidget->setAutoFillBackground(false);
    QVBoxLayout* baseLayout = new QVBoxLayout;
    scrollArea.setWidgetResizable(true);
    baseLayout->addWidget(&scrollArea);
    self->setLayout(baseLayout);

    layoutSwicher = new LayoutSwitcher;
    topVBox->addWidget(layoutSwicher);
}


LayoutSwitcherView::~LayoutSwitcherView()
{
    delete impl;
}


void LayoutSwitcherView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<LayoutSwitcherView>(
                N_("LayoutSwitcherView"), N_("Layout Switcher"), ViewManager::SINGLE_OPTIONAL);
}


bool LayoutSwitcherView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool LayoutSwitcherView::Impl::storeState(Archive& archive)
{
    layoutSwicher->storeState(archive);
    return true;
}


bool LayoutSwitcherView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool LayoutSwitcherView::Impl::restoreState(const Archive& archive)
{
    layoutSwicher->restoreState(archive);
    return true;
}


void LayoutSwitcherBar::initialize(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        ext->addToolBar(instance());
        initialized  = true;
    }
}


LayoutSwitcherBar* LayoutSwitcherBar::instance()
{
    static LayoutSwitcherBar* switcherBar = new LayoutSwitcherBar;
    return switcherBar;
}


LayoutSwitcherBar::LayoutSwitcherBar()
    : ToolBar(N_("LayoutSwitcherBar"))
{
    impl = new Impl(this);
}


LayoutSwitcherBar::Impl::Impl(LayoutSwitcherBar* self)
    : self(self)
{
    layoutSwitcher = new LayoutSwitcher;
    self->addWidget(layoutSwitcher);
}


LayoutSwitcherBar::~LayoutSwitcherBar()
{
    delete impl;
}


bool LayoutSwitcherBar::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool LayoutSwitcherBar::Impl::storeState(Archive& archive)
{
    layoutSwitcher->storeState(archive);
    return true;
}


bool LayoutSwitcherBar::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool LayoutSwitcherBar::Impl::restoreState(const Archive& archive)
{
    layoutSwitcher->restoreState(archive);
    return true;
}
