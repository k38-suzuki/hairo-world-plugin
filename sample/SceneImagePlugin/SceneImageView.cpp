/**
   \file
   \author Kenta Suzuki
*/

#include "SceneImageView.h"
#include <cnoid/ViewManager>
#include <QVBoxLayout>
#include "SceneImageWidget.h"
#include "gettext.h"

using namespace cnoid;

namespace cnoid {

class SceneImageViewImpl
{
public:
    SceneImageViewImpl(SceneImageView* self);
    SceneImageView* self;
    SceneImageWidget* imageWidget;

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


SceneImageView::SceneImageView()
{
    impl = new SceneImageViewImpl(this);
}


SceneImageViewImpl::SceneImageViewImpl(SceneImageView* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout;
    imageWidget = new SceneImageWidget;
    vbox->addWidget(imageWidget);
    self->setLayout(vbox);
}


SceneImageView::~SceneImageView()
{
    delete impl;
}


void SceneImageView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<SceneImageView>(
                N_("SceneImageView"), N_("Scene Image"), ViewManager::MULTI_OPTIONAL);
}


bool SceneImageView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool SceneImageViewImpl::storeState(Archive& archive)
{
    imageWidget->storeState(archive);
    return true;
}


bool SceneImageView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool SceneImageViewImpl::restoreState(const Archive& archive)
{
    imageWidget->restoreState(archive);
    return true;
}
