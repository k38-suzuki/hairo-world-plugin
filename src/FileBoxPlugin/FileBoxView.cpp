/**
   \file
   \author Kenta Suzuki
*/

#include "FileBoxView.h"
#include <cnoid/MainWindow>
#include <cnoid/ViewArea>
#include <cnoid/ViewManager>
#include <QVBoxLayout>
#include "FileBoxWidget.h"
#include "gettext.h"

using namespace cnoid;

FileBoxView* fileBoxView = 0;

namespace cnoid {

class FileBoxViewImpl
{
public:
    FileBoxViewImpl(FileBoxView* self);

    FileBoxView* self;
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
    void onActivated();
    void onDeactivated();
};

}


FileBoxView::FileBoxView()
{
    impl = new FileBoxViewImpl(this);
}


FileBoxViewImpl::FileBoxViewImpl(FileBoxView* self)
    : self(self)
{
    self->setDefaultLayoutArea(View::RIGHT);
    QVBoxLayout* vbox = new QVBoxLayout();
    FileBoxWidget* widget = new FileBoxWidget();
    vbox->addWidget(widget);
    self->setLayout(vbox);
}


FileBoxView::~FileBoxView()
{
    delete impl;
}


void FileBoxView::initializeClass(ExtensionManager* ext)
{
    fileBoxView = ext->viewManager().registerClass<FileBoxView>(
        "FileBoxView", N_("File Box"), ViewManager::SINGLE_OPTIONAL);
}


FileBoxView* FileBoxView::instance()
{
    return fileBoxView;
}


bool FileBoxView::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool FileBoxViewImpl::storeState(Archive& archive)
{
    return true;
}


bool FileBoxView::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool FileBoxViewImpl::restoreState(const Archive& archive)
{
    return true;
}


void FileBoxView::onActivated()
{
    impl->onActivated();
}


void FileBoxViewImpl::onActivated()
{

}


void FileBoxView::onDeactivated()
{
    impl->onDeactivated();
}


void FileBoxViewImpl::onDeactivated()
{

}
