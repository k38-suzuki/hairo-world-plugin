/**
   @author Kenta Suzuki
*/

#include "FileFormWidget.h"
#include <cnoid/BodyItem>
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/stdx/filesystem>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
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

    void openSaveDialog();
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


void FileFormWidget::Impl::openSaveDialog()
{
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Save a Body file"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.setOption(QFileDialog::DontConfirmOverwrite);

    QStringList filters;
    filters << _("Body files (*.body)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    ProjectManager* pm = ProjectManager::instance();
    string currentProjectFile = pm->currentProjectFile();
    filesystem::path path(currentProjectFile);
    string currentProjectName = path.stem().string();
    if(!dialog.selectFilePath(currentProjectFile)) {
        dialog.selectFile(currentProjectName);
    }

    if(dialog.exec() == QDialog::Accepted) {
        QString filename = dialog.selectedFiles().front();
        fileLine->setText(filename);
    }
}


void FileFormWidget::Impl::onSaveButtonClicked()
{
    string filename = fileLine->text().toStdString();

    if(filename.empty()) {
        openSaveDialog();
        filename = fileLine->text().toStdString();
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
    for(size_t i = 0; i < bodyItems.size(); ++i) {
        BodyItem* bodyItem = bodyItems[i];
        string filename0 = fileLine->text().toStdString();
        string filename1 = bodyItem->filePath().c_str();
        if(filename0 == filename1) {
            bodyItem->reload();
        }
    }
}
