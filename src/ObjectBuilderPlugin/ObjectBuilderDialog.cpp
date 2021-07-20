/**
   \file
   \author Kenta Suzuki
*/

#include "ObjectBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/ComboBox>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/stdx/filesystem>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QVBoxLayout>
#include "gettext.h"
#include "GratingBuilderWidget.h"
#include "PipeBuilderWidget.h"
#include "SlopeBuilderWidget.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

ObjectBuilderDialog* builderDialog = nullptr;

namespace {

struct DialogButtonInfo {
    QDialogButtonBox::ButtonRole role;
};


DialogButtonInfo dialogButtonInfo[] = {
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::AcceptRole }
};

}


namespace cnoid {

class ObjectBuilderDialogImpl
{
public:
    ObjectBuilderDialogImpl(ObjectBuilderDialog* self);
    ObjectBuilderDialog* self;

    QStackedLayout* sbox;
    ComboBox* shapeCombo;
    LineEdit* fileLine;
    GratingBuilderWidget* gratingWidget;
    PipeBuilderWidget* pipeWidget;
    SlopeBuilderWidget* slopeWidget;

    enum Shape {
        PIPE,
        GRATING,
        SLOPE,
        NUM_SHAPE
    };

    enum DialogButtonId { SAVE, SAVEAS, OK, NUM_DBUTTONS };

    PushButton* dialogButtons[NUM_DBUTTONS];

    void openSaveDialog();
    void writeYaml(const bool& overwrite);
    void onCurrentIndexChanged(const int& index);
    void onSaveAsButtonClicked();
    void onSaveButtonClicked();
    void onAccepted();
    void onRejected();
};

}


ObjectBuilderDialog::ObjectBuilderDialog()
{
    impl = new ObjectBuilderDialogImpl(this);
}


ObjectBuilderDialogImpl::ObjectBuilderDialogImpl(ObjectBuilderDialog* self)
    : self(self)
{
    self->setWindowTitle(_("ObjectBuilder"));
    QVBoxLayout* vbox = new QVBoxLayout();

    QHBoxLayout* hbox = new QHBoxLayout();
    shapeCombo = new ComboBox();
    shapeCombo->setCurrentIndex(0);
    QStringList shapes = { _("Pipe"), _("Grating"), _("Slope") };
    shapeCombo->addItems(shapes);
    hbox->addWidget(new QLabel(_("Shape")));
    hbox->addWidget(shapeCombo);
    hbox->addStretch();

    sbox = new QStackedLayout();
    pipeWidget = new PipeBuilderWidget();
    sbox->addWidget(pipeWidget);
    sbox->setCurrentWidget(pipeWidget);
    gratingWidget = new GratingBuilderWidget();
    sbox->addWidget(gratingWidget);
    slopeWidget = new SlopeBuilderWidget();
    sbox->addWidget(slopeWidget);

    QHBoxLayout* fhbox = new QHBoxLayout();
    fileLine = new LineEdit();
    fileLine->setEnabled(false);
    fhbox->addWidget(new QLabel(_("File")));
    fhbox->addWidget(fileLine);

    const char* labels[] = { _("&Save"), _("&Save As..."), _ ("&Ok") };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    for(int i = 0; i < NUM_DBUTTONS; ++i) {
        DialogButtonInfo info = dialogButtonInfo[i];
        dialogButtons[i] = new PushButton(labels[i]);
        PushButton* dialogButton = dialogButtons[i];
        buttonBox->addButton(dialogButton, info.role);
        if(i == OK) {
            dialogButton->setDefault(true);
        }
    }

    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    shapeCombo->sigCurrentIndexChanged().connect([&](int index){ onCurrentIndexChanged(index); });
    dialogButtons[SAVE]->sigClicked().connect([&](){ onSaveButtonClicked(); });
    dialogButtons[SAVEAS]->sigClicked().connect([&](){ onSaveAsButtonClicked(); });

    vbox->addLayout(hbox);
    vbox->addLayout(new HSeparatorBox(new QLabel(_("Configuration"))));
    vbox->addLayout(sbox);
    vbox->addWidget(new HSeparator());
    vbox->addLayout(fhbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);
}


ObjectBuilderDialog::~ObjectBuilderDialog()
{
    delete impl;
}


void ObjectBuilderDialog::initializeClass(ExtensionManager* ext)
{
    if(!builderDialog) {
        builderDialog = ext->manage(new ObjectBuilderDialog());
    }

    MenuManager& menuManager = ext->menuManager();
    menuManager.setPath("/Tools");
    menuManager.addItem(_("ObjectBuilder"))
            ->sigTriggered().connect([](){ builderDialog->show(); });
}


void ObjectBuilderDialogImpl::openSaveDialog()
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


void ObjectBuilderDialogImpl::writeYaml(const bool &overwrite)
{
    string filename = fileLine->text().toStdString();

    if(!overwrite || filename.empty()) {
        openSaveDialog();
    }

    filename = fileLine->text().toStdString();

    if(!filename.empty()) {
        filesystem::path path(filename);
        string extension = path.extension().string();
        if(extension.empty()) {
           filename += ".body";
           fileLine->setText(QString::fromStdString(filename));
        }

        int index = shapeCombo->currentIndex();
        switch (index) {
        case PIPE:
            pipeWidget->save(filename);
            break;
        case GRATING:
            gratingWidget->save(filename);
            break;
        case SLOPE:
            slopeWidget->save(filename);
        default:
            break;
        }
    }
}


void ObjectBuilderDialogImpl::onCurrentIndexChanged(const int& index)
{
    sbox->setCurrentIndex(index);
}


void ObjectBuilderDialogImpl::onSaveAsButtonClicked()
{
    writeYaml(false);
}


void ObjectBuilderDialogImpl::onSaveButtonClicked()
{
    writeYaml(true);
}


void ObjectBuilderDialog::onAccepted()
{
    impl->onAccepted();
}


void ObjectBuilderDialogImpl::onAccepted()
{

}


void ObjectBuilderDialog::onRejected()
{
    impl->onRejected();
}


void ObjectBuilderDialogImpl::onRejected()
{

}
