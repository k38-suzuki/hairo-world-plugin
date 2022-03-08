/**
   \file
   \author Kenta Suzuki
*/

#include "KIOSKView.h"
#include <cnoid/AppConfig>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class KIOSKViewImpl
{
public:
    KIOSKViewImpl(KIOSKView* self);
    KIOSKView* self;
    virtual ~KIOSKViewImpl();

    QScrollArea scrollArea;
    QTabWidget* tabWidget;
    BookmarkWidget* bookmarkWidget;
    LogWidget* logWidget;

    void store(Mapping& archive);
    void restore(const Mapping& archive);
};

}


KIOSKView::KIOSKView()
{
    impl = new KIOSKViewImpl(this);
}


KIOSKViewImpl::KIOSKViewImpl(KIOSKView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::CenterArea);
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

    bookmarkWidget = new BookmarkWidget;
    logWidget = new LogWidget;
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(false);
    tabWidget->addTab(bookmarkWidget, _("Bookmark"));
    tabWidget->addTab(logWidget, _("Log"));
    topVBox->addWidget(tabWidget);

    Mapping& config = *AppConfig::archive()->openMapping("kiosk");
    if(config.isValid()) {
        restore(config);
    }
}


KIOSKView::~KIOSKView()
{
    delete impl;
}


KIOSKViewImpl::~KIOSKViewImpl()
{
    store(*AppConfig::archive()->openMapping("kiosk"));
}


void KIOSKView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<KIOSKView>(
                "KIOSKView", N_("KIOSK"), ViewManager::SINGLE_OPTIONAL);
}


KIOSKView* KIOSKView::instance()
{
    static KIOSKView* instance_ = ViewManager::findView<KIOSKView>();
    return instance_;
}


BookmarkWidget* KIOSKView::bookmarkWidget()
{
    return impl->bookmarkWidget;
}


LogWidget* KIOSKView::logWidget()
{
    return impl->logWidget;
}


void KIOSKViewImpl::store(Mapping& archive)
{
    bookmarkWidget->store(archive);
    logWidget->store(archive);
}


void KIOSKViewImpl::restore(const Mapping& archive)
{
    bookmarkWidget->restore(archive);
    logWidget->restore(archive);
}
