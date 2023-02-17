/**
   \file
   \author Kenta Suzuki
*/

#include "TerrainGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/FileDialog>
#include <cnoid/LineEdit>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/YAMLWriter>
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

TerrainGenerator* instance_ = nullptr;
DoubleSpinBox* scaleSpin = nullptr;

}

namespace cnoid {

class TerrainData
{
public:
    TerrainData();

    bool read(const string& filename);

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


class TerrainGeneratorImpl : public Dialog
{
public:
    TerrainGeneratorImpl(TerrainGenerator* self);
    TerrainGenerator* self;

    LineEdit* inputFileLine;
    TerrainData* data;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;

    bool save(const string& filename);
    void onLoadButtonClicked();
    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
};

}


TerrainGenerator::TerrainGenerator()
{
    impl = new TerrainGeneratorImpl(this);

}


TerrainGeneratorImpl::TerrainGeneratorImpl(TerrainGenerator* self)
    : self(self)
{
    setWindowTitle(_("BoxTerrain Builder"));
    yamlWriter.setKeyOrderPreservationMode(true);

    scaleSpin = new DoubleSpinBox;
    scaleSpin->setDecimals(1);
    scaleSpin->setSingleStep(0.1);
    scaleSpin->setValue(1.0);
    scaleSpin->setRange(0.1, 10.0);

    inputFileLine = new LineEdit;
    inputFileLine->setEnabled(false);
    data = nullptr;
    PushButton* loadButton = new PushButton;
    QIcon openIcon = QIcon::fromTheme("document-open");
    if(openIcon.isNull()) {
        loadButton->setText(_("&Load"));
    } else {
        loadButton->setIcon(openIcon);
    }

    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;
    int index = 0;
    gbox->addWidget(new QLabel(_("Input File (.csv)")), index, 0);
    gbox->addWidget(inputFileLine, index, 1, 1, 2);
    gbox->addWidget(loadButton, index++, 3);
    gbox->addWidget(new QLabel(_("scale[0.1-10.0]")), index, 0);
    gbox->addWidget(scaleSpin, index++, 1);

    formWidget = new FileFormWidget;

    vbox->addLayout(gbox);
    vbox->addStretch();
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    loadButton->sigClicked().connect([&](){ onLoadButtonClicked(); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


TerrainGenerator::~TerrainGenerator()
{
    delete impl;
}


void TerrainGenerator::initializeClass(ExtensionManager* ext)
{
    if(!instance_) {
        instance_ = ext->manage(new TerrainGenerator);

        MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("Make Body File"));
        mm.addItem(_("BoxTerrain"))->sigTriggered().connect(
                    [&](){ instance_->impl->show(); });
    }
}


bool TerrainGeneratorImpl::save(const string& filename)
{
    string inputFile = inputFileLine->text().toStdString();
    if(inputFile.empty()) {
        return false;
    }
    
    data = new TerrainData;
    if(!data->read(inputFile)) {
        return false;
    }

    if(!filename.empty()) {
        auto topNode = writeBody(filename);
        if(yamlWriter.openFile(filename)) {
            yamlWriter.putNode(topNode);
            yamlWriter.closeFile();
        }
    }

    return true;
}


void TerrainGeneratorImpl::onLoadButtonClicked()
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


MappingPtr TerrainGeneratorImpl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(filename);
    string name = path.stem().string();

    node->write("format", "ChoreonoidBody");
    node->write("formatVersion", "1.0");
    node->write("angleUnit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;
    linksNode->append(writeLink());
    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
}


MappingPtr TerrainGeneratorImpl::writeLink()
{
    MappingPtr node = new Mapping;

    node->write("name", "Root");
    node->write("jointType", "fixed");
    node->write("material", "Ground");

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void TerrainGeneratorImpl::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    double unit = 0.1 * scaleSpin->value();
    double x = data->xsize() * unit / 2.0;
    double y = data->ysize() * unit / 2.0;
    write(node, "translation", Vector3(-x, y, 0.0));

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "IndexedFaceSet");
    Listing& coordinateList = *geometryNode->createFlowStyleListing("coordinate");

    int n = data->xsize() * data->ysize() * 4 * 2 * 3;

    for(int k = 0; k < data->ysize(); ++k) {
        for(int j = 0; j < data->xsize(); ++j) {
            for(int i = 0; i < 4; ++i) {
                Vector3 p = data->p_a(j, k, i);
                coordinateList.append(p[0], 3, n);
                coordinateList.append(p[1], 3, n);
                coordinateList.append(p[2], 3, n);
            }
        }
    }

    for(int k = 0; k < data->ysize(); ++k) {
        for(int j = 0; j < data->xsize(); ++j) {
            for(int i = 0; i < 4; ++i) {
                Vector3 p = data->p_b(j, k, i);
                coordinateList.append(p[0], 3, n);
                coordinateList.append(p[1], 3, n);
                coordinateList.append(p[2], 3, n);
            }
        }
    }

    Listing& coordIndexList = *geometryNode->createFlowStyleListing("coordIndex");

    int n1 = (data->xsize() * data->ysize() * 4 + data->xsize() + data->ysize()) * 5;

    for(int j = 0; j < data->ysize(); ++j) {
        for(int i = 0; i < data->xsize(); ++i) {
            coordIndexList.append(data->id(i, j, 0, 0), 5, n1);
            coordIndexList.append(data->id(i, j, 1, 0), 5, n1);
            coordIndexList.append(data->id(i, j, 2, 0), 5, n1);
            coordIndexList.append(data->id(i, j, 3, 0), 5, n1);
            coordIndexList.append(                  -1, 5, n1);

            coordIndexList.append(data->id(i, j, 0, 1), 5, n1);
            coordIndexList.append(data->id(i, j, 3, 1), 5, n1);
            coordIndexList.append(data->id(i, j, 2, 1), 5, n1);
            coordIndexList.append(data->id(i, j, 1, 1), 5, n1);
            coordIndexList.append(                  -1, 5, n1);

            if(i != 0) {
                coordIndexList.append(data->id(i - 1, j, 3, 0), 5, n1);
                coordIndexList.append(data->id(i - 1, j, 2, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 1, 0),     5, n1);
                coordIndexList.append(data->id(i, j, 0, 0),     5, n1);
                coordIndexList.append(                  -1,     5, n1);
            } else {
                coordIndexList.append(data->id(i, j, 1, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 1, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 0, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 0, 1), 5, n1);
                coordIndexList.append(                  -1, 5, n1);
            }

            if(j != 0) {
                coordIndexList.append(data->id(i, j - 1, 1, 0), 5, n1);
                coordIndexList.append(    data->id(i, j, 0, 0), 5, n1);
                coordIndexList.append(    data->id(i, j, 3, 0), 5, n1);
                coordIndexList.append(data->id(i, j - 1, 2, 0), 5, n1);
                coordIndexList.append(                      -1, 5, n1);
            } else {
                coordIndexList.append(data->id(i, j, 0, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 0, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 3, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 3, 1), 5, n1);
                coordIndexList.append(                  -1, 5, n1);
            }

            if(j == data->ysize() - 1) {
                coordIndexList.append(data->id(i, j, 1, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 1, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 2, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 2, 0), 5, n1);
                coordIndexList.append(                  -1, 5, n1);
            }

            if(i == data->xsize() - 1) {
                coordIndexList.append(data->id(i, j, 2, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 3, 1), 5, n1);
                coordIndexList.append(data->id(i, j, 3, 0), 5, n1);
                coordIndexList.append(data->id(i, j, 2, 0), 5, n1);
                coordIndexList.append(                  -1, 5, n1);
            }
        }
    }

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", Vector3(1.0, 1.0, 1.0));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
}


TerrainData::TerrainData()
{
    xsize_ = 0;
    ysize_ = 0;
    id_ = 0;
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
    for(int k = 0; k < ysize_; ++k) {
        for(int j = 0; j < xsize_; ++j) {
            for(int i = 0; i < 4; ++i) {
                cell_a[k][j][i] = id_++;
            }
        }
    }

    //set cell_b
    for(int k = 0; k < ysize_; ++k) {
        for(int j = 0; j < xsize_; ++j) {
            for(int i = 0; i < 4; ++i) {
                cell_b[k][j][i] = id_++;
            }
        }
    }

    //set point_a, point_b
    double scale = 0.1 * scaleSpin->value();

    for(int j = 0; j < ysize_; ++j) {
        for(int i = 0; i < xsize_; ++i) {
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