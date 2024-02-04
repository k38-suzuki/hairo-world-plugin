/**
   @author Kenta Suzuki
*/

#include "BeepView.h"
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QScrollArea>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

BeepView* instance_ = nullptr;

}

namespace cnoid {

class BeepView::Impl
{
public:
    BeepView* self;

    Impl(BeepView* self);

    QScrollArea scrollArea;

    BeepWidget* beepWidget;

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


void BeepView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<BeepView>(
                N_("BeepView"), N_("Beep"), ViewManager::SINGLE_OPTIONAL);
}


BeepView* BeepView::instance()
{
    if(!instance_) {
        instance_ = ViewManager::findView<BeepView>();
    }
    return instance_;
}


BeepView::BeepView()
{
    impl = new Impl(this);
}


BeepView::Impl::Impl(BeepView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::TopCenterArea);

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

    beepWidget = new BeepWidget;
    topVBox->addWidget(beepWidget);
}


BeepView::~BeepView()
{
    delete impl;
}


BeepWidget* BeepView::beepWidget()
{
    return impl->beepWidget;
}


bool BeepView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool BeepView::Impl::storeState(Archive& archive)
{
    beepWidget->storeState(archive);
    return true;
}


bool BeepView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool BeepView::Impl::restoreState(const Archive& archive)
{
    beepWidget->restoreState(archive);
    return true;
}
