/**
   \file
   \author Kenta Suzuki
*/

#include "TerrainGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <fstream>
#include <sstream>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

DoubleSpinBox* scaleSpin = nullptr;

}

namespace cnoid {

class TerrainData
{
public:
    TerrainData();
    virtual ~TerrainData();

    bool read(const std::string& filename);

    int xsize() const { return xsize_; }
    int ysize() const { return ysize_; }
    Vector3 p_a(const int& x, const int& y, const int& index) const { return point_a_[y][x][index]; }
    Vector3 p_b(const int& x, const int& y, const int& index) const { return point_b_[y][x][index]; }

    int id(const int& x, const int& y, const int& index, const int& sindex) const;

private:
    double height[512][512];
    double cell_a[512][512][4];
    double cell_b[512][512][4];
    Vector3 point_a_[512][512][4];
    Vector3 point_b_[512][512][4];
    int xsize_;
    int ysize_;
    int id_;
};


class TerrainConfigDialog : public Dialog
{
public:
    TerrainConfigDialog();

    LineEdit* inputFileLine;
    PushButton* loadButton;
    MessageView* mv;
    FileFormWidget* formWidget;

    bool save(const string& filename);
    void onLoadButtonClicked();
};


class TerrainGeneratorImpl
{
public:
    TerrainGeneratorImpl(TerrainGenerator* self);
    TerrainGenerator* self;

    TerrainConfigDialog* dialog;
};

}


TerrainGenerator::TerrainGenerator()
{
    impl = new TerrainGeneratorImpl(this);

}


TerrainGeneratorImpl::TerrainGeneratorImpl(TerrainGenerator* self)
    : self(self)
{
    dialog = new TerrainConfigDialog();
}


TerrainGenerator::~TerrainGenerator()
{
    delete impl;
}


void TerrainGenerator::initialize(ExtensionManager* ext)
{
    TerrainGenerator* generator = ext->manage(new TerrainGenerator);

    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("BodyGenerator"));
    mm.addItem(_("BoxTerrain"))->sigTriggered().connect([=](){ generator->show(); });
}


void TerrainGenerator::show()
{
    impl->dialog->show();
}


TerrainConfigDialog::TerrainConfigDialog()
    : mv(MessageView::instance())
{
    setWindowTitle(_("BoxTerrain Builder"));

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

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    PushButton* okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);

    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(formWidget);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));
    loadButton->sigClicked().connect([&](){ onLoadButtonClicked(); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


bool TerrainConfigDialog::save(const string& filename)
{
    string inputFile = inputFileLine->text().toStdString();
    if(filename.empty() || inputFile.empty()) {
        return false;
    }

    FILE* fp = fopen(filename.c_str(), "w");
    if(fp == NULL) {
        mv->putln(_("cannot open body file."));
        return false;
    }

    TerrainData* data = new TerrainData();
    if(!data->read(inputFile)) {
        mv->putln(_("cannot csv body file."));
        return false;
    }

    filesystem::path path(filename);
    string name = path.stem().string();

    fprintf(fp, "format: ChoreonoidBody\n");
    fprintf(fp, "formatVersion: 1.0\n");
    fprintf(fp, "name: %s\n", name.c_str());
    fprintf(fp, "links:\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: BOXTERRAIN\n");

    double unit = 0.1 * scaleSpin->value() / 2.0;
    double x = data->xsize() * unit * -1.0;
    double y = data->ysize() * unit;
    Vector3 pos(x, y, 0.0);

    fprintf(fp, "    translation: [ %4.2lf, %4.2lf, %4.2lf ]\n", pos[0], pos[1], pos[2]);
    fprintf(fp, "    jointType: fixed\n");
    fprintf(fp, "    material: Ground\n");
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      Shape:\n");
    fprintf(fp, "        geometry:\n");
    fprintf(fp, "          type: IndexedFaceSet\n");
    fprintf(fp, "          coordinate: [\n");


    for(int k = 0; k < data->ysize(); k++) {
        for(int j = 0; j < data->xsize(); j++) {
            for(int i = 0; i < 4; i++) {
                Vector3 point = data->p_a(j, k, i);
                fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", point[0], point[1], point[2]);
            }
        }
    }

    for(int k = 0; k < data->ysize(); k++) {
        for(int j = 0; j < data->xsize(); j++) {
            for(int i = 0; i < 4; i++) {
                Vector3 point = data->p_b(j, k, i);
                fprintf(fp, "            %4.2f, %4.2f, %4.2f,\n", point[0], point[1], point[2]);
            }
        }
    }

    fprintf(fp, "          ]\n");
    fprintf(fp, "          coordIndex: [\n");

    for(int j = 0; j < data->ysize(); j++) {
        for(int i = 0; i < data->xsize(); i++) {
            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 0, 0), data->id(i, j, 1, 0), data->id(i, j, 2, 0), data->id(i, j, 3, 0));
            fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 0, 1), data->id(i, j, 3, 1), data->id(i, j, 2, 1), data->id(i, j, 1, 1));
            if(i != 0) {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i - 1, j, 3, 0), data->id(i - 1, j, 2, 0), data->id(i, j, 1, 0), data->id(i, j, 0, 0));
            } else {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 1, 1), data->id(i, j, 1, 0), data->id(i, j, 0, 0), data->id(i, j, 0, 1));
            }

            if(j != 0) {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j - 1, 1, 0), data->id(i, j, 0, 0), data->id(i, j, 3, 0), data->id(i, j - 1, 2, 0));
            } else {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 0, 1), data->id(i, j, 0, 0), data->id(i, j, 3, 0), data->id(i, j, 3, 1));
            }

            if(j == data->ysize() - 1) {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 1, 0), data->id(i, j, 1, 1), data->id(i, j, 2, 1), data->id(i, j, 2, 0));
            }

            if(i == data->xsize() - 1) {
                fprintf(fp,  "            %d, %d, %d, %d, -1,\n", data->id(i, j, 2, 1), data->id(i, j, 3, 1), data->id(i, j, 3, 0), data->id(i, j, 2, 0));
            }
        }
    }

    fprintf(fp, "          ]\n");
    fprintf(fp, "        appearance:\n");
    fprintf(fp, "          material: { diffuseColor: [ 1, 1, 1 ] }\n");
    fprintf(fp, "#          texture: { url: \"path/to/image.png\" }\n");
    fclose(fp);

    return true;
}


void TerrainConfigDialog::onLoadButtonClicked()
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


TerrainData::TerrainData()
{
    xsize_ = 0;
    ysize_ = 0;
    id_ = 0;
}


TerrainData::~TerrainData()
{

}


bool TerrainData::read(const string& filename)
{
    //set height
    ifstream ifs(filename.c_str());
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
    xsize_ = row;
    ysize_ = clm;

    //set cell_a
    for(int k = 0; k < ysize_; k++) {
        for(int j = 0; j < xsize_; j++) {
            for(int i = 0; i < 4; i++) {
                cell_a[k][j][i] = id_++;
            }
        }
    }

    //set cell_b
    for(int k = 0; k < ysize_; k++) {
        for(int j = 0; j < xsize_; j++) {
            for(int i = 0; i < 4; i++) {
                cell_b[k][j][i] = id_++;
            }
        }
    }

    //set point_a, point_b
    double scale = 0.1 * scaleSpin->value();

    for(int j = 0; j < ysize_; j++) {
        for(int i = 0; i < xsize_; i++) {
            double eq0 = scale * i;
            double eq1 = scale * j * -1;
            double eq2 = scale * height[j][i];
            double eq3 = scale * (j + 1) * -1;
            double eq4 = scale * (i + 1);
            double eq5 = scale * 0;

            point_a_[j][i][0] = Vector3(eq0, eq1, eq2);
            point_a_[j][i][1] = Vector3(eq0, eq3, eq2);
            point_a_[j][i][2] = Vector3(eq4, eq3, eq2);
            point_a_[j][i][3] = Vector3(eq4, eq1, eq2);
            point_b_[j][i][0] = Vector3(eq0, eq1, eq5);
            point_b_[j][i][1] = Vector3(eq0, eq3, eq5);
            point_b_[j][i][2] = Vector3(eq4, eq3, eq5);
            point_b_[j][i][3] = Vector3(eq4, eq1, eq5);
        }
    }

    return true;
}


int TerrainData::id(const int& x, const int& y, const int& index, const int& sindex) const
{
    if(sindex == 0) {
        return cell_a[y][x][index];
    } else {
        return cell_b[y][x][index];
    }
}
