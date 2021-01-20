/**
   \file
   \author Kenta Suzuki
*/

#include "BoxTerrainBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/SpinBox>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;

BoxTerrainBuilderDialog* stepFieldBuilderDialog = nullptr;

namespace cnoid {

class BoxTerrainBuilderDialogImpl
{
public:
    BoxTerrainBuilderDialogImpl(BoxTerrainBuilderDialog* self);
    BoxTerrainBuilderDialog* self;

    DoubleSpinBox* scaleSpin;
    LineEdit* inputFileLine;
    LineEdit* outputFileLine;

    void onAccepted();
    void onRejected();
    void onSaveButtonClicked();
    void onLoadButtonClicked();
    void onClearButtonClicked();
    void onExportBody();
};


class CellManager
{
public:
    CellManager();
    ~CellManager();

    bool read(const string fileName);
    const int getXsize() const;
    const int getYsize() const;
    const double getPointax(int x, int y, int index) const;
    const double getPointay(int x, int y, int index) const;
    const double getPointaz(int x, int y, int index) const;
    const double getPointbx(int x, int y, int index) const;
    const double getPointby(int x, int y, int index) const;
    const double getPointbz(int x, int y, int index) const;
    const int getId(int x, int y, int index, int sindex) const;

private:
    class CellManagerImpl;
    CellManagerImpl *impl;
};

class CellManager::CellManagerImpl
{
public:
    bool read(const string fileName);

    double height[512][512];
    double cella[512][512][4];
    double cellb[512][512][4];
    double pointa[512][512][4][3];
    double pointb[512][512][4][3];

    double xsize;
    double ysize;
    int id;
};


CellManager::CellManager()
{
    impl = new CellManagerImpl;
}


CellManager::~CellManager()
{
    delete impl;
}


bool CellManager::read(const string fileName)
{
    impl->read(fileName);
    return true;
}


bool CellManager::CellManagerImpl::read(const string fileName)
{
    //set height
    ifstream ifs(fileName.c_str());
    if(!ifs) {
        return false;
    }

    string str;
    int row = 0;
    int clm = 0;

    while(getline(ifs, str)) {
        string token;
        istringstream stream(str);
        row = 0;
        while(getline(stream, token, ',')) {
            height[clm][row] = atof(token.c_str());
            if(height[clm][row] < 0.0) {
                height[clm][row] = 0.0;
            }
            row++;
        }
        clm++;
    }
    xsize = row;
    ysize = clm;

    id = 0;

    //set cella
    for(int k = 0; k < ysize; k++) {
        for(int j = 0; j < xsize; j++) {
            for(int i = 0; i < 4; i++) {
                cella[k][j][i] = id++;
            }
        }
    }

    //set cellb
    for(int k = 0; k < ysize; k++) {
        for(int j = 0; j < xsize; j++) {
            for(int i = 0; i < 4; i++) {
                cellb[k][j][i] = id++;
            }
        }
    }

    //set pointa, pointb
    double scale = 0.1;
    if(stepFieldBuilderDialog) {
        scale *= stepFieldBuilderDialog->scale();
    }
    for(int j = 0; j < ysize; j++) {
        for(int i = 0; i < xsize; i++) {
            pointa[j][i][0][0] = scale * (i - xsize / 2);
            pointa[j][i][0][1] = scale * (j - ysize / 2) * -1;
            pointa[j][i][0][2] = scale * height[j][i];
            pointa[j][i][1][0] = scale * (i - xsize / 2);
            pointa[j][i][1][1] = scale * (j + 1 - ysize / 2) * -1;
            pointa[j][i][1][2] = scale * height[j][i];
            pointa[j][i][2][0] = scale * (i + 1 - xsize / 2);
            pointa[j][i][2][1] = scale * (j + 1 - ysize / 2) * -1;
            pointa[j][i][2][2] = scale * height[j][i];
            pointa[j][i][3][0] = scale * (i + 1 - xsize / 2);
            pointa[j][i][3][1] = scale * (j - ysize / 2) * -1;
            pointa[j][i][3][2] = scale * height[j][i];

            pointb[j][i][0][0] = scale * (i - xsize / 2);
            pointb[j][i][0][1] = scale * (j - ysize / 2) * -1;
            pointb[j][i][0][2] = scale * 0.0;
            pointb[j][i][1][0] = scale * (i - xsize / 2);
            pointb[j][i][1][1] = scale * (j + 1 - ysize / 2) * -1;
            pointb[j][i][1][2] = scale * 0.0;
            pointb[j][i][2][0] = scale * (i + 1 - xsize / 2);
            pointb[j][i][2][1] = scale * (j + 1 - ysize / 2) * -1;
            pointb[j][i][2][2] = scale * 0.0;
            pointb[j][i][3][0] = scale * (i + 1 - xsize / 2);
            pointb[j][i][3][1] = scale * (j - ysize / 2) * -1;
            pointb[j][i][3][2] = scale * 0.0;
        }
    }

    return true;
}


const int CellManager::getXsize() const
{
    return impl->xsize;
}


const int CellManager::getYsize() const
{
    return impl->ysize;
}


const double CellManager::getPointax(int x, int y, int index) const
{
    return impl->pointa[y][x][index][0];
}


const double CellManager::getPointay(int x, int y, int index) const
{
    return impl->pointa[y][x][index][1];
}


const double CellManager::getPointaz(int x, int y, int index) const
{
    return impl->pointa[y][x][index][2];
}


const double CellManager::getPointbx(int x, int y, int index) const
{
    return impl->pointb[y][x][index][0];
}


const double CellManager::getPointby(int x, int y, int index) const
{
    return impl->pointb[y][x][index][1];
}


const double CellManager::getPointbz(int x, int y, int index) const
{
    return impl->pointb[y][x][index][2];
}


const int CellManager::getId(int x, int y, int index, int sindex) const
{
    if(sindex == 0) {
        return impl->cella[y][x][index];
    }
    else {
        return impl->cellb[y][x][index];
    }
}

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
    PushButton* clearButton = new PushButton(_("Clear"));
    gbox->addWidget(new QLabel(_("scale[0.1-10.0]")), index, 0);
    gbox->addWidget(scaleSpin, index, 1);
    gbox->addWidget(clearButton, index++, 2);

    inputFileLine = new LineEdit();
    inputFileLine->setEnabled(false);
    PushButton* loadButton = new PushButton(_("Load"));
    gbox->addWidget(new QLabel(_("Input File (.csv)")), index, 0);
    gbox->addWidget(inputFileLine, index, 1);
    gbox->addWidget(loadButton, index++, 2);

    outputFileLine = new LineEdit();
    outputFileLine->setEnabled(false);
    PushButton* saveButton = new PushButton(_("Save"));
    gbox->addWidget(new QLabel(_("Output File (.body)")), index, 0);
    gbox->addWidget(outputFileLine, index, 1);
    gbox->addWidget(saveButton, index++, 2);

    PushButton* overwriteButton = new PushButton(_("Overwrite"));
    gbox->addWidget(overwriteButton, index, 2);
    vbox->addLayout(gbox);

    self->setLayout(vbox);

    clearButton->sigClicked().connect([&](){ onClearButtonClicked(); });
    loadButton->sigClicked().connect([&](){ onLoadButtonClicked(); });
    saveButton->sigClicked().connect([&](){ onSaveButtonClicked(); });
    overwriteButton->sigClicked().connect([&](){ onExportBody(); });
}


BoxTerrainBuilderDialog::~BoxTerrainBuilderDialog()
{
    delete impl;
}


void BoxTerrainBuilderDialog::initialzeClass(ExtensionManager* ext)
{
    if(!stepFieldBuilderDialog) {
        stepFieldBuilderDialog = ext->manage(new BoxTerrainBuilderDialog());
    }

    MenuManager& menuManager = ext->menuManager();
    menuManager.setPath("/Tools");
    menuManager.addItem(_("BoxTerrainBuilder"))
            ->sigTriggered().connect([](){ stepFieldBuilderDialog->show(); });
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


void BoxTerrainBuilderDialogImpl::onSaveButtonClicked()
{
    QString currentDirectory = QString::fromStdString(ProjectManager::instance()->currentProjectDirectory());
    QString inputFileName = inputFileLine->text();
    if(!inputFileName.isEmpty()) {
        QFileInfo inputInfo(inputFileName);
        currentDirectory = inputInfo.absolutePath();
        QString outputFileName = QFileDialog::getSaveFileName(nullptr, _("output body file"), currentDirectory,
                                                        _("BODY files (*.body)"));
        if(!outputFileName.isEmpty()) {
            QFileInfo outputInfo(outputFileName);
            QString ext = outputInfo.suffix();
            if(ext.isEmpty()) {
                outputFileName += ".body";
            }
            outputFileLine->setText(outputFileName);
            onExportBody();
        }
    }
}


void BoxTerrainBuilderDialogImpl::onLoadButtonClicked()
{
    QString currentDirectory = QString::fromStdString(ProjectManager::instance()->currentProjectDirectory());
    QString inputFileName = inputFileLine->text();
    if(!inputFileName.isEmpty()) {
        QFileInfo info(inputFileName);
        currentDirectory = info.absolutePath();
    }
    inputFileName = QFileDialog::getOpenFileName(nullptr, _("input csv file"), currentDirectory,
                                                   _("CSV files (*.csv)"));
    inputFileLine->setText(inputFileName);
}


void BoxTerrainBuilderDialogImpl::onClearButtonClicked()
{
    inputFileLine->clear();
    outputFileLine->clear();
    scaleSpin->setValue(1.0);
}


void BoxTerrainBuilderDialogImpl::onExportBody()
{
    QString inputFileName = inputFileLine->text();
    QString outputFileName = outputFileLine->text();
    if(!inputFileName.isEmpty() && !outputFileName.isEmpty()) {
        FILE* fp = fopen(outputFileName.toStdString().c_str(), "w");
        if(fp == NULL) {
            qCritical().noquote() << "cannot open body file." << endl;
        }
        else {
            CellManager cm;
            if(!cm.read(inputFileName.toStdString())) {
                qCritical().noquote() << "cannot csv body file." << endl;
            }
            else {

                QFileInfo info(outputFileName);
                QString bodyName = info.baseName();

                fprintf(fp, "format: ChoreonoidBody\n");
                fprintf(fp, "formatVersion: 1.0\n");
                fprintf(fp, "name: %s\n", bodyName.toStdString().c_str());
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


                for(int k = 0; k < cm.getYsize(); k++) {
                    for(int j = 0; j < cm.getXsize(); j++) {
                        for(int i = 0; i < 4; i++) {
                            fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", cm.getPointax(j, k, i), cm.getPointay(j, k, i), cm.getPointaz(j, k, i));
                        }
                    }
                }

                for(int k = 0; k < cm.getYsize(); k++) {
                    for(int j = 0; j < cm.getXsize(); j++) {
                        for(int i = 0; i < 4; i++) {
                            fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", cm.getPointbx(j, k, i), cm.getPointby(j, k, i), cm.getPointbz(j, k, i));
                        }
                    }
                }

                fprintf(fp, "          ]\n");
                fprintf(fp, "          coordIndex: [\n");

                for(int j = 0; j < cm.getYsize(); j++) {
                    for(int i = 0; i < cm.getXsize(); i++) {
                        fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 0, 0), cm.getId(i, j, 1, 0), cm.getId(i, j, 2, 0), cm.getId(i, j, 3, 0));
                        fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 0, 1), cm.getId(i, j, 3, 1), cm.getId(i, j, 2, 1), cm.getId(i, j, 1, 1));
                        if(i != 0) {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i - 1, j, 3, 0), cm.getId(i - 1, j, 2, 0), cm.getId(i, j, 1, 0), cm.getId(i, j, 0, 0));
                        }
                        else {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 1, 1), cm.getId(i, j, 1, 0), cm.getId(i, j, 0, 0), cm.getId(i, j, 0, 1));
                        }

                        if(j != 0) {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j - 1, 1, 0), cm.getId(i, j, 0, 0), cm.getId(i, j, 3, 0), cm.getId(i, j - 1, 2, 0));
                        }
                        else {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 0, 1), cm.getId(i, j, 0, 0), cm.getId(i, j, 3, 0), cm.getId(i, j, 3, 1));
                        }

                        if(j == cm.getYsize() - 1) {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 1, 0), cm.getId(i, j, 1, 1), cm.getId(i, j, 2, 1), cm.getId(i, j, 2, 0));
                        }

                        if(i == cm.getXsize() - 1) {
                            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", cm.getId(i, j, 2, 1), cm.getId(i, j, 3, 1), cm.getId(i, j, 3, 0), cm.getId(i, j, 2, 0));
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
