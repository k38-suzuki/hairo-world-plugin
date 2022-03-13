/**
   \file
   \author Kenta Suzuki
*/

#include "KIOSKView.h"
#include <cnoid/AppConfig>
#include <cnoid/MainWindow>
#include <cnoid/ViewManager>
#include <cnoid/Widget>
#include <QKeyEvent>
#include <QMenuBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;

namespace {

KIOSKView* instance_ = nullptr;

}

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
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
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
    if(!instance_) {
        instance_ = ViewManager::findView<KIOSKView>();
    }
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


bool KIOSKView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool KIOSKViewImpl::storeState(Archive& archive)
{
    bookmarkWidget->storeState(archive);
    return true;
}


bool KIOSKView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool KIOSKViewImpl::restoreState(const Archive& archive)
{
    bookmarkWidget->restoreState(archive);
    return true;
}


void KIOSKView::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape) {
        MainWindow* mw = MainWindow::instance();
        QMenuBar* menuBar = mw->menuBar();
        menuBar->setVisible(!menuBar->isVisible());
    }
}
