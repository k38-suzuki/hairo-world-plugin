/**
   \file
   \author Kenta Suzuki
*/

#include "BoxTerrainBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "gettext.h"
#include "CellManager.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

BoxTerrainBuilderDialog* dialog = nullptr;

namespace {

struct DialogButtonInfo {
    QDialogButtonBox::ButtonRole role;
};


DialogButtonInfo dialogButtonInfo[] = {
    { QDialogButtonBox::ResetRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::AcceptRole }
};

}

namespace cnoid {

class BoxTerrainBuilderDialogImpl
{
public:
    BoxTerrainBuilderDialogImpl(BoxTerrainBuilderDialog* self);
    BoxTerrainBuilderDialog* self;

    DoubleSpinBox* scaleSpin;
    LineEdit* inputFileLine;
    LineEdit* outputFileLine;

    enum DialogButtonId { RESET, SAVE, SAVEAS, LOAD, OK, NUM_DBUTTONS };
    PushButton* dialogButtons[NUM_DBUTTONS];

    void onAccepted();
    void onRejected();
    void onSaveAsButtonClicked();
    void onLoadButtonClicked();
    void onResetButtonClicked();
    void onSaveButtonClicked();
};

}


BoxTerrainBuilderDialog::BoxTerrainBuilderDialog()
{
    impl = new BoxTerrainBuilderDialogImpl(this);
}


BoxTerrainBuilderDialogImpl::BoxTerrainBuilderDialogImpl(BoxTerrainBuilderDialog* self)
    : self(self)
{
    self->setWindowTitle(_("BoxTerrainBuilder"));
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();
    int index = 0;

    scaleSpin = new DoubleSpinBox();
    scaleSpin->setDecimals(1);
    scaleSpin->setSingleStep(0.1);
    scaleSpin->setValue(1.0);
    scaleSpin->setRange(0.1, 10.0);
    gbox->addWidget(new QLabel(_("scale[0.1-10.0]")), index, 0);
    gbox->addWidget(scaleSpin, index++, 1);

    inputFileLine = new LineEdit();
    inputFileLine->setEnabled(false);
    gbox->addWidget(new QLabel(_("Input File (.csv)")), index, 0);
    gbox->addWidget(inputFileLine, index++, 1);

    outputFileLine = new LineEdit();
    outputFileLine->setEnabled(false);
    gbox->addWidget(new QLabel(_("Output File (.body)")), index, 0);
    gbox->addWidget(outputFileLine, index++, 1);

    vbox->addLayout(gbox);

    const char* labels[] = {
        _("&Reset"), _("&Save"), _("&Save As..."),
        _("&Load"), _("&Ok")
    };

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
    vbox->addWidget(buttonBox);

    self->setLayout(vbox);
    dialogButtons[RESET]->sigClicked().connect([&](){ onResetButtonClicked(); });
    dialogButtons[SAVE]->sigClicked().connect([&](){ onSaveButtonClicked(); });
    dialogButtons[SAVEAS]->sigClicked().connect([&](){ onSaveAsButtonClicked(); });
    dialogButtons[LOAD]->sigClicked().connect([&](){ onLoadButtonClicked(); });
}


BoxTerrainBuilderDialog::~BoxTerrainBuilderDialog()
{
    delete impl;
}


void BoxTerrainBuilderDialog::initialzeClass(ExtensionManager* ext)
{
    if(!dialog) {
        dialog = ext->manage(new BoxTerrainBuilderDialog());
    }

    MenuManager& menuManager = ext->menuManager();
    menuManager.setPath("/Tools");
    menuManager.addItem(_("BoxTerrainBuilder"))
            ->sigTriggered().connect([](){ dialog->show(); });
}


BoxTerrainBuilderDialog* BoxTerrainBuilderDialog::instance()
{
    return dialog;
}


double BoxTerrainBuilderDialog::scale() const
{
    return impl->scaleSpin->value();
}


void BoxTerrainBuilderDialog::onAccepted()
{
    impl->onAccepted();
}


void BoxTerrainBuilderDialogImpl::onAccepted()
{

}


void BoxTerrainBuilderDialog::onRejected()
{
    impl->onRejected();
}


void BoxTerrainBuilderDialogImpl::onRejected()
{

}


void BoxTerrainBuilderDialogImpl::onSaveAsButtonClicked()
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
        string filename = dialog.selectedFiles().front().toStdString();
        if(!filename.empty()){
            filesystem::path path(filename);
            string ext = path.extension().string();
            if(ext != ".body"){
                filename += ".body";
            }
            outputFileLine->setText(filename.c_str());
            onSaveButtonClicked();
        }
    }
}


void BoxTerrainBuilderDialogImpl::onLoadButtonClicked()
{
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Open a CSV file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("CSV files (*.csv)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    if(dialog.exec()) {
        string filename = dialog.selectedFiles().front().toStdString();
        inputFileLine->setText(filename);
    }
}


void BoxTerrainBuilderDialogImpl::onResetButtonClicked()
{
    inputFileLine->clear();
    outputFileLine->clear();
    scaleSpin->setValue(1.0);
}


void BoxTerrainBuilderDialogImpl::onSaveButtonClicked()
{
    QString inputFileName = inputFileLine->text();
    QString outputFileName = outputFileLine->text();
    if(!inputFileName.isEmpty()) {
        if(outputFileName.isEmpty()) {
            onSaveAsButtonClicked();
        } else {
            FILE* fp = fopen(outputFileName.toStdString().c_str(), "w");
            if(fp == NULL) {
                qCritical().noquote() << "cannot open body file." << endl;
            } else {
                CellManager cm;
                if(!cm.read(inputFileName.toStdString())) {
                    qCritical().noquote() << "cannot csv body file." << endl;
                } else {
                    filesystem::path path(outputFileName.toStdString());
                    string bodyName = path.stem().string();

                    fprintf(fp, "format: ChoreonoidBody\n");
                    fprintf(fp, "formatVersion: 1.0\n");
                    fprintf(fp, "name: %s\n", bodyName.c_str());
                    fprintf(fp, "links:\n");
                    fprintf(fp, "  -\n");
                    fprintf(fp, "    name: STEPFIELD\n");
                    fprintf(fp, "#    parent: \n");
                    fprintf(fp, "    translation: [ 0, 0, 0 ]\n");
                    fprintf(fp, "    rotation: [ [ 1, 0, 0, 0 ], [ 0, 1, 0, 0 ], [ 0, 0, 1, 0 ] ]\n");
                    fprintf(fp, "    jointType: fixed\n");
                    fprintf(fp, "    material: Ground\n");
                    fprintf(fp, "    elements:\n");
                    fprintf(fp, "      Shape:\n");
                    fprintf(fp, "        geometry:\n");
                    fprintf(fp, "          type: IndexedFaceSet\n");
                    fprintf(fp, "          coordinate: [\n");


                    for(int k = 0; k < cm.ysize(); k++) {
                        for(int j = 0; j < cm.xsize(); j++) {
                            for(int i = 0; i < 4; i++) {
                                fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", cm.pointax(j, k, i), cm.pointay(j, k, i), cm.pointaz(j, k, i));
                            }
                        }
                    }

                    for(int k = 0; k < cm.ysize(); k++) {
                        for(int j = 0; j < cm.xsize(); j++) {
                            for(int i = 0; i < 4; i++) {
                                fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", cm.pointbx(j, k, i), cm.pointby(j, k, i), cm.pointbz(j, k, i));
                            }
                        }
                    }

                    fprintf(fp, "          ]\n");
                    fprintf(fp, "          coordIndex: [\n");

                    for(int j = 0; j < cm.ysize(); j++) {
                        for(int i = 0; i < cm.xsize(); i++) {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 0, 0), cm.id(i, j, 1, 0), cm.id(i, j, 2, 0), cm.id(i, j, 3, 0));
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 0, 1), cm.id(i, j, 3, 1), cm.id(i, j, 2, 1), cm.id(i, j, 1, 1));
                            if(i != 0) {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i - 1, j, 3, 0), cm.id(i - 1, j, 2, 0), cm.id(i, j, 1, 0), cm.id(i, j, 0, 0));
                            } else {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 1, 1), cm.id(i, j, 1, 0), cm.id(i, j, 0, 0), cm.id(i, j, 0, 1));
                            }

                            if(j != 0) {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j - 1, 1, 0), cm.id(i, j, 0, 0), cm.id(i, j, 3, 0), cm.id(i, j - 1, 2, 0));
                            } else {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 0, 1), cm.id(i, j, 0, 0), cm.id(i, j, 3, 0), cm.id(i, j, 3, 1));
                            }

                            if(j == cm.ysize() - 1) {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 1, 0), cm.id(i, j, 1, 1), cm.id(i, j, 2, 1), cm.id(i, j, 2, 0));
                            }

                            if(i == cm.xsize() - 1) {
                                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.id(i, j, 2, 1), cm.id(i, j, 3, 1), cm.id(i, j, 3, 0), cm.id(i, j, 2, 0));
                            }
                        }
                    }

                    fprintf(fp, "          ]\n");
                    fprintf(fp, "        appearance:\n");
                    fprintf(fp, "          material:\n");
                    fprintf(fp, "            diffuseColor: [ 1, 1, 1 ]\n");
                    fprintf(fp, "#          texture:\n");
                    fprintf(fp, "#            url: \"texture/oak.png\"\n");
                }
            }
            fclose(fp);
        }
    }
}
