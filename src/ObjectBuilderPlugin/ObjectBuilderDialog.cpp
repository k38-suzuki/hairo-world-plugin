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

    void openSaveDialog();
    void writeYaml(const bool& overwrite);
    void onCurrentIndexChanged(const int& index);
    void onGenerateButtonClicked();
    void onOverwriteButtonClicked();
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
    PushButton* generateButton = new PushButton(_("Generate"));
    PushButton* overwriteButton = new PushButton(_("Overwrite"));
    fileLine->setEnabled(false);
    fhbox->addWidget(new QLabel(_("File")));
    fhbox->addWidget(fileLine);
    fhbox->addWidget(generateButton);
    fhbox->addWidget(overwriteButton);

    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    shapeCombo->sigCurrentIndexChanged().connect([&](int index){ onCurrentIndexChanged(index); });
    generateButton->sigClicked().connect([&](){ onGenerateButtonClicked(); });
    overwriteButton->sigClicked().connect([&](){ onOverwriteButtonClicked(); });

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
    string currentProjectName = path.stem();
    if(!dialog.selectFilePath(currentProjectFile)) {
        dialog.selectFile(currentProjectName);
    }

    if(dialog.exec() == QDialog::Accepted) {
        QString fileName = dialog.selectedFiles().front();
        fileLine->setText(fileName);
    }
}


void ObjectBuilderDialogImpl::writeYaml(const bool &overwrite)
{
    string filename = fileLine->text().toStdString();
    if(!overwrite || filename.empty()) {
        openSaveDialog();

    }

    filename = fileLine->text().toStdString();
    filesystem::path path(filename);
    string extension = path.extension();
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


void ObjectBuilderDialogImpl::onCurrentIndexChanged(const int& index)
{
    sbox->setCurrentIndex(index);
}


void ObjectBuilderDialogImpl::onGenerateButtonClicked()
{
    writeYaml(false);
}


void ObjectBuilderDialogImpl::onOverwriteButtonClicked()
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
