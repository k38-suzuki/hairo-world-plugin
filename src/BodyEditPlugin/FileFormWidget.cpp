/**
   @author Kenta Suzuki
*/

#include "FileFormWidget.h"
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/LineEdit>
#include <cnoid/RootItem>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <cnoid/HamburgerMenu>
#include <QBoxLayout>
#include <QLabel>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class FileFormWidget::Impl
{
public:
    FileFormWidget* self;

    Impl(FileFormWidget* self);

    LineEdit* fileLine;

    Signal<void(string)> sigClicked_;
    SignalProxy<void(string)> sigClicked() {
        return sigClicked_;
    }

    void onSaveButtonClicked();
    void onReloadButtonClicked();
};

}


FileFormWidget::FileFormWidget(QWidget* parent)
    : Widget(parent)
{
    impl = new Impl(this);
}


FileFormWidget::Impl::Impl(FileFormWidget* self)
    : self(self)
{
    fileLine = new LineEdit;
    PushButton* saveButton = new PushButton(_("&Save"));
    saveButton->sigClicked().connect([&](){ onSaveButtonClicked(); });

    auto hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(_("File")));
    hbox->addWidget(fileLine);
    hbox->addWidget(saveButton);

    auto vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    self->setLayout(vbox);
}


FileFormWidget::~FileFormWidget()
{
    delete impl;
}


SignalProxy<void(string)> FileFormWidget::sigClicked()
{
    return impl->sigClicked();
}


void FileFormWidget::Impl::onSaveButtonClicked()
{
    string filename = fileLine->text().toStdString();

    if(filename.empty()) {
        filename = getSaveFileName("Save a body", "body");
        fileLine->setText(filename.c_str());
    }

    if(!filename.empty()) {
        filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".body") {
           filename += ".body";
        }
        fileLine->setText(filename.c_str());
        sigClicked_(filename);
    }

    onReloadButtonClicked();
}


void FileFormWidget::Impl::onReloadButtonClicked()
{
    RootItem* rootItem = RootItem::instance();
    ItemList<BodyItem> bodyItems = rootItem->checkedItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        string filename0 = fileLine->text().toStdString();
        string filename1 = bodyItem->filePath().c_str();
        if(filename0 == filename1) {
            bodyItem->reload();
        }
    }
}