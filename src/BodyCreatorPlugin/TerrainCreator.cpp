/**
   @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/MenuManager>
#include <cnoid/UTF8>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include "BodyCreatorDialog.h"
#include "CreatorToolBar.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

double scale = 1.0;

class TerrainData
{
public:
    TerrainData();

    bool read(const string& filename);

    double height(const int& x, const int& y) const { return height_[y][x]; }
    int xsize() const { return xsize_; }
    int ysize() const { return ysize_; }
    Vector3 p_a(const int& x, const int& y, const int& index) const { return point_a_[y][x][index]; }
    Vector3 p_b(const int& x, const int& y, const int& index) const { return point_b_[y][x][index]; }

    int id(const int& x, const int& y, const int& index, const int& sindex) const;

private:
    double height_[BUFSIZ][BUFSIZ];
    double cell_a_[BUFSIZ][BUFSIZ][4];
    double cell_b_[BUFSIZ][BUFSIZ][4];
    Vector3 point_a_[BUFSIZ][BUFSIZ][4];
    Vector3 point_b_[BUFSIZ][BUFSIZ][4];
    int xsize_;
    int ysize_;
    int id_;
};

class TerrainCreatorWidget : public QWidget
{
public:
    TerrainCreatorWidget(QWidget* parent = nullptr);

private:
    void reset();
    bool save(const string& filename);
    void load();

    MappingPtr writeBody(const string& filename);
    MappingPtr writeLink();
    void writeLinkShape(Listing* elementsNode);
    void writeLinkShape2(Listing* elementsNode);

    QString fileName;
    QDoubleSpinBox* scaleSpinBox;

    TerrainData* data;
    YAMLWriter yamlWriter;
};

}


void TerrainCreator::initializeClass(ExtensionManager* ext)
{
    static TerrainCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new TerrainCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("BoxTerrain"), widget);
    }
}


TerrainCreator::TerrainCreator()
{

}


TerrainCreator::~TerrainCreator()
{

}


TerrainCreatorWidget::TerrainCreatorWidget(QWidget* parent)
    : QWidget(parent),
      fileName("")
{
    yamlWriter.setKeyOrderPreservationMode(true);

    data = nullptr;

    const QIcon openIcon = QIcon::fromTheme("document-open");
    QPushButton* openButton = new QPushButton(openIcon, _("&Open"), this);
    connect(openButton, &QPushButton::clicked, [&](){ load(); });

    scaleSpinBox = new QDoubleSpinBox;
    scaleSpinBox->setDecimals(1);
    scaleSpinBox->setSingleStep(0.1);
    scaleSpinBox->setValue(1.0);
    scaleSpinBox->setRange(0.1, 10.0);
    connect(scaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [&](double d){ scale = d; });

    auto formLayout = new QFormLayout;
    formLayout->addRow(_("CSV File"), openButton);
    formLayout->addRow(_("scale[0.1-10.0]"), scaleSpinBox);

    auto toolBar = new CreatorToolBar;
    toolBar->sigNewRequested().connect([&](){ reset(); });
    toolBar->sigSaveRequested().connect([&](const string& filename){ save(filename); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setWindowTitle(_("BoxTerrain Generator"));
}


void TerrainCreatorWidget::reset()
{
    fileName.clear();

    scaleSpinBox->setValue(1.0);
}


bool TerrainCreatorWidget::save(const string& filename)
{
    string inputFile = fileName.toStdString();
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


void TerrainCreatorWidget::load()
{
    string filename = getOpenFileName(_("CSV File"), "csv");
    if(!filename.empty()) {
        fileName = filename.c_str();
    }
}


MappingPtr TerrainCreatorWidget::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(fromUTF8(filename));
    string name = path.stem().string();

    node->write("format", "ChoreonoidBody");
    node->write("format_version", "2.0");
    node->write("angle_unit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;
    linksNode->append(writeLink());
    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
}


MappingPtr TerrainCreatorWidget::writeLink()
{
    MappingPtr node = new Mapping;

    node->write("name", "Root");
    node->write("joint_type", "fixed");
    node->write("material", "Ground");
    node->write("AMOR", true);

    ListingPtr elementsNode = new Listing;
    writeLinkShape(elementsNode);
    // writeLinkShape2(elementsNode);
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


void TerrainCreatorWidget::writeLinkShape(Listing* elementsNode)
{
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    double unit = 0.1 * scale;
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
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuse", Vector3(1.0, 1.0, 1.0));

    elementsNode->append(node);
}


void TerrainCreatorWidget::writeLinkShape2(Listing* elementsNode)
{
    int xsize = data->xsize();
    int ysize = data->ysize();
    double unit = 0.1 * scale;

    for(int j = 0; j < ysize; ++j) {
        for(int i = 0; i < xsize; ++i) {
            MappingPtr node = new Mapping;

            node->write("type", "Shape");

            double x = (-xsize / 2.0 + i + 0.5) * unit;
            double y = (ysize / 2.0 - j - 0.5) * unit;
            double z = data->height(i, j) * unit ;
            write(node, "translation", Vector3(x, y, z / 2.0));

            MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
            geometryNode->write("type", "Box");
            write(geometryNode, "size", Vector3(unit, unit, z));

            MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
            MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
            write(materialNode, "diffuse", Vector3(1.0, 1.0, 1.0));

            elementsNode->append(node);
        }
    }
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
            height_[clm][row] = atof(token.c_str());
            if(height_[clm][row] < 0.0) {
                height_[clm][row] = 0.0;
            }
            row++;
        }
        clm++;
    }
    xsize_ = row;
    ysize_ = clm;

    //set cell_a_
    for(int k = 0; k < ysize_; ++k) {
        for(int j = 0; j < xsize_; ++j) {
            for(int i = 0; i < 4; ++i) {
                cell_a_[k][j][i] = id_++;
            }
        }
    }

    //set cell_b_
    for(int k = 0; k < ysize_; ++k) {
        for(int j = 0; j < xsize_; ++j) {
            for(int i = 0; i < 4; ++i) {
                cell_b_[k][j][i] = id_++;
            }
        }
    }

    //set point_a, point_b
    double unit = 0.1 * scale;

    for(int j = 0; j < ysize_; ++j) {
        for(int i = 0; i < xsize_; ++i) {
            double eq0 = unit * i;
            double eq1 = unit * j * -1;
            double eq2 = unit * height_[j][i];
            double eq3 = unit * (j + 1) * -1;
            double eq4 = unit * (i + 1);
            double eq5 = unit * 0;

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
        return cell_a_[y][x][index];
    } else {
        return cell_b_[y][x][index];
    }
}