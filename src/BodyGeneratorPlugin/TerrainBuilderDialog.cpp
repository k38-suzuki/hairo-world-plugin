/**
   \file
   \author Kenta Suzuki
*/

#include "TerrainBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "CellManager.h"
#include "FileFormWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

TerrainBuilderDialog* terrainDialog = nullptr;

namespace cnoid {

class TerrainBuilderDialogImpl
{
public:
    TerrainBuilderDialogImpl(TerrainBuilderDialog* self);
    TerrainBuilderDialog* self;

    DoubleSpinBox* scaleSpin;
    LineEdit* inputFileLine;
    PushButton* loadButton;
    FileFormWidget* formWidget;

    bool save(const string& filename);
    void onLoadButtonClicked();
    void onAccepted();
    void onRejected();
};

}


TerrainBuilderDialog::TerrainBuilderDialog()
{
    impl = new TerrainBuilderDialogImpl(this);

}


TerrainBuilderDialogImpl::TerrainBuilderDialogImpl(TerrainBuilderDialog* self)
    : self(self)
{
    self->setWindowTitle(_("BoxTerrain Builder"));
    scaleSpin = new DoubleSpinBox();
    scaleSpin->setDecimals(1);
    scaleSpin->setSingleStep(0.1);
    scaleSpin->setValue(1.0);
    scaleSpin->setRange(0.1, 10.0);

    inputFileLine = new LineEdit();
    inputFileLine->setEnabled(false);

    loadButton = new PushButton(_("&Load"));

    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();
    int index = 0;
    gbox->addWidget(new QLabel(_("Input File (.csv)")), index, 0);
    gbox->addWidget(inputFileLine, index, 1, 1, 2);
    gbox->addWidget(loadButton, index++, 3);
    gbox->addWidget(new QLabel(_("scale[0.1-10.0]")), index, 0);
    gbox->addWidget(scaleSpin, index++, 1);

    formWidget = new FileFormWidget();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    PushButton* okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);

    vbox->addLayout(gbox);
    vbox->addWidget(formWidget);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));
    loadButton->sigClicked().connect([&](){ onLoadButtonClicked(); });
    formWidget->sigClicked().connect([&](string filename){ terrainDialog->save(filename); });
}


TerrainBuilderDialog::~TerrainBuilderDialog()
{
    delete impl;
}


TerrainBuilderDialog* TerrainBuilderDialog::instance()
{
    if(!terrainDialog) {
        terrainDialog = new TerrainBuilderDialog();
    }
    return terrainDialog;
}


double TerrainBuilderDialog::scale() const
{
    return impl->scaleSpin->value();
}


bool TerrainBuilderDialog::save(const string& filename)
{
    return impl->save(filename);
}


bool TerrainBuilderDialogImpl::save(const string& filename)
{
    string inputFile = inputFileLine->text().toStdString();
    if(!filename.empty() && !inputFile.empty()) {
        CellManager cm;
        if(!cm.read(inputFile)) {
            qCritical().noquote() << "cannot csv body file." << endl;
        } else {
            FILE* fp = fopen(filename.c_str(), "w");
            if(fp == NULL) {
                qCritical().noquote() << "cannot open body file." << endl;
            } else {
                filesystem::path path(filename);
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
                fclose(fp);
            }
        }
    }
    return true;
}


void TerrainBuilderDialogImpl::onLoadButtonClicked()
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


void TerrainBuilderDialog::onAccepted()
{
    impl->onAccepted();
}


void TerrainBuilderDialogImpl::onAccepted()
{

}


void TerrainBuilderDialog::onRejected()
{
    impl->onRejected();
}


void TerrainBuilderDialogImpl::onRejected()
{

}
