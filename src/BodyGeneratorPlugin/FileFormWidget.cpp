/**
   \file
   \author Kenta Suzuki
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
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

FileFormWidget* formWidget = nullptr;

}

namespace cnoid {

class FileFormWidgetImpl
{
public:
    FileFormWidgetImpl(FileFormWidget* self);
    FileFormWidget* self;

    enum DialogButtonId { RELOAD, SAVE, SAVEAS, NUM_BUTTONS };

    LineEdit* fileLine;
    PushButton* buttons[NUM_BUTTONS];

    Signal<void(string)> sigClicked_;
    SignalProxy<void(string)> sigClicked() {
        return sigClicked_;
    }

    void openSaveDialog();
    void save(const bool& overwrite);
    void onReloadButtonClicked();
};

}


FileFormWidget::FileFormWidget()
{
    impl = new FileFormWidgetImpl(this);
}


FileFormWidgetImpl::FileFormWidgetImpl(FileFormWidget* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout;

    fileLine = new LineEdit;
    fileLine->setEnabled(false);
    QHBoxLayout* fhbox = new QHBoxLayout;
    fhbox->addWidget(new QLabel(_("File")));
    fhbox->addWidget(fileLine);

    static const char* labels[] = { _("&Reload"), _("&Save"), _("Save &As...") };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton(labels[i]);
        buttonBox->addButton(buttons[i], QDialogButtonBox::ActionRole);
    }

    vbox->addLayout(fhbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    buttons[RELOAD]->sigClicked().connect([&](){ onReloadButtonClicked(); });
    buttons[SAVE]->sigClicked().connect([&](){ save(true); });
    buttons[SAVEAS]->sigClicked().connect([&](){ save(false); });
}


FileFormWidget::~FileFormWidget()
{
    delete impl;
}


SignalProxy<void(string)> FileFormWidget::sigClicked()
{
    return impl->sigClicked();
}


void FileFormWidgetImpl::openSaveDialog()
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


void FileFormWidgetImpl::save(const bool &overwrite)
{
    string filename = fileLine->text().toStdString();

    if(!overwrite || filename.empty()) {
        openSaveDialog();
        filename = fileLine->text().toStdString();
    }

    if(!filename.empty()) {
        filesystem::path path(filename);
        string extension = path.extension().string();
        if(extension.empty()) {
           filename += ".body";
           fileLine->setText(QString::fromStdString(filename));
        }

        sigClicked_(filename);
    }
}


void FileFormWidgetImpl::onReloadButtonClicked()
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
