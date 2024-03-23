/**
   @author Kenta Suzuki
*/

#include "FileFormWidget.h"
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/ItemFileDialog>
#include <cnoid/ItemFileIO>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/stdx/filesystem>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
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


FileFormWidget::FileFormWidget()
{
    impl = new Impl(this);
}


FileFormWidget::Impl::Impl(FileFormWidget* self)
    : self(self)
{
    fileLine = new LineEdit;
    PushButton* saveButton = new PushButton(_("&Save"));
    saveButton->sigClicked().connect([&](){ onSaveButtonClicked(); });

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel(_("File")));
    hbox->addWidget(fileLine);
    hbox->addWidget(saveButton);

    QVBoxLayout* vbox = new QVBoxLayout;
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
        filesystem::path path(filename);
        string extension = path.extension().string();
        if(extension.empty()) {
           filename += ".body";
           fileLine->setText(filename.c_str());
        }
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


static QString makeNameFilterString(const std::string& caption, const string& extensions)
{
    QString filters =
        ItemFileDialog::makeNameFilter(
            caption, ItemFileIO::separateExtensions(extensions));
    
    filters += _(";;Any files (*)");
    return filters;
}


namespace cnoid {

string getSaveFileName(const string& caption, const string& extensions)
{
    string filename;
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(caption.c_str());
    dialog.setNameFilter(makeNameFilterString(caption, extensions));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.updatePresetDirectories();
    if(dialog.exec() == QDialog::Accepted) {
        filename = dialog.selectedFiles().front().toStdString();
    }
    return filename;
}


vector<string> getSaveFileNames(const string& caption, const string& extensions)
{
    vector<string> filenames;
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(caption.c_str());
    dialog.setNameFilter(makeNameFilterString(caption, extensions));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.updatePresetDirectories();
    if(dialog.exec() == QDialog::Accepted) {
        for(auto& file : dialog.selectedFiles()) {
            filenames.push_back(file.toStdString());
        }
    }
    return filenames;
}

}

