/**
   \file
   \author Kenta Suzuki
*/

#include "CrossSectionItem.h"
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/Buttons>
#include <cnoid/CheckBox>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExecutablePath>
#include <cnoid/ItemList>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/SceneDrawables>
#include <cnoid/Selection>
#include <cnoid/Separator>
#include <cnoid/Slider>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/ViewManager>
#include <cnoid/stdx/filesystem>
#include <fmt/format.h>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>
#include <cstdlib>
#include <iomanip>
#include "gettext.h"
#include "ColorScale.h"
#include "GammaCamera.h"
#include "PHITSRunner.h"
#include "PHITSWriter.h"
#include "QADWriter.h"
#include "gettext.h"

#define MAX_PARAMETER 4
#define MAX_PROCESS 99

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

void copyQADLIB(const string& filename)
{
    filesystem::path path(filename);
    string qadDirPath = toUTF8((path.parent_path() / "LIB").string());
    filesystem::path phitsDir(qadDirPath);
    if(!filesystem::exists(phitsDir)) {
        filesystem::create_directories(phitsDir);
    }

    string libDirPath = toUTF8((shareDirPath() / "default" / "LIB").string());
    filesystem::path libDir(libDirPath);
    QDir libQDir(libDirPath.c_str());

    for(QString file : libQDir.entryList(QDir::Files)) {
        QString filename0 = toUTF8((libDir / file.toStdString().c_str()).string()).c_str();
        QString filename1 = toUTF8((phitsDir / file.toStdString().c_str()).string()).c_str();
        QFile::copy(filename0, filename1);
    }
}

bool writeTextFile(const string& filename, const string& text)
{
    if(!text.empty()) {
        QFile file(filename.c_str());
        if(!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        QTextStream qts(&file);
        qts << text.c_str();
        file.close();
    }
    return true;
}

SgMesh* generateSquare(const double xside, const double yside)
{
    SgMesh* mesh = new SgMesh;

    SgVertexArray& vertices = *mesh->setVertices(new SgVertexArray);
    vertices.resize(4);

    double hxside = xside / 2.0;
    double hyside = yside / 2.0;

    vertices[0] <<  hxside,  hyside, 0.0;
    vertices[1] <<  hxside, -hyside, 0.0;
    vertices[2] << -hxside, -hyside, 0.0;
    vertices[3] << -hxside,  hyside, 0.0;

    SgTexCoordArray& texCoords = *mesh->setTexCoords(new SgTexCoordArray);
    texCoords.resize(4);
    texCoords[0] << 1.0, 0.0;
    texCoords[1] << 1.0, 1.0;
    texCoords[2] << 0.0, 1.0;
    texCoords[3] << 0.0, 0.0;

    mesh->setNumTriangles(2);
    mesh->setTriangle(0, 0, 2, 1);
    mesh->setTriangle(1, 0, 3, 2);
    mesh->texCoordIndices() = mesh->triangleVertices();
    return mesh;
}

}

namespace cnoid {

class DoseConfigDialog : public Dialog
{
public:
    DoseConfigDialog();

    SpinBox* maxCasSpin;
    SpinBox* maxBchSpin;
    SpinBox nSpin[MAX_PARAMETER];
    DoubleSpinBox minSpin[MAX_PARAMETER];
    DoubleSpinBox maxSpin[MAX_PARAMETER];
    ComboBox* codeCombo;
    CheckBox* messageCheck;

    PHITSRunner phitsRunner;
    PHITSRunner qadRunners[MAX_PROCESS];
    GammaData::CalcInfo calcInfo;
    int countQAD;
    GammaData qadTotDoseDataFile;
    string gammaDataFile;
    string defaultNuclideTableFile;
    string defaultElementTableFile;

    enum CodeID { PHITS, QAD, NUM_CODES };

    SignalProxy<void(double value)> sigValueChanged() { return sigValueChanged_; }
    SignalProxy<void(string filename)> sigReadPHITSData() { return sigReadPHITSData_; }

    ComboBox* plainCombo;
    DoubleSpinBox* dspin;
    Slider* slider;
    Signal<void(double value)> sigValueChanged_;
    Signal<void(string filename)> sigReadPHITSData_;

    bool readPHITSData(const string& filename);
    void onItemTriggered(const bool& on);
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};


class CrossSectionItemImpl
{
public:
    CrossSectionItemImpl(CrossSectionItem* self);
    CrossSectionItemImpl(CrossSectionItem* self, const CrossSectionItemImpl& org);
    CrossSectionItem* self;

    enum PlainID { XY, YZ, ZX, NUM_PLAINS };

    SgPosTransformPtr scene;

    Isometry3 position;
    OrthoNodeDataPtr nodeData;
    GammaData gammaData;
    DoseConfigDialog* config;
    string gammaDataFile;
    Signal<void()> sigGammaDataLoaded_;
    Selection colorScale;

    enum ColorScaleID { LOG_SCALE, LINER_SCALE, NUM_SCALES };

    void initialize();
    void setDefaultNuclideTableFile(const string& filename);
    void setDefaultElementTableFile(const string& filename);
    bool onColorScalePropertyChanged(const int& index);
    void createScene();
    void updateScenePosition();
    void onValueChanged();
    void onReadPHITSData(const string& filename);
    bool load(const string& filename);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


CrossSectionItem::CrossSectionItem()
{
    impl = new CrossSectionItemImpl(this);
}


CrossSectionItemImpl::CrossSectionItemImpl(CrossSectionItem* self)
    : self(self),
      config(new DoseConfigDialog)
{
    position.setIdentity();
    nodeData = nullptr;
    gammaDataFile.clear();
    colorScale.setSymbol(LOG_SCALE, N_("Log"));
    colorScale.setSymbol(LINER_SCALE, N_("Liner"));
    initialize();
}


CrossSectionItem::CrossSectionItem(const CrossSectionItem& org)
    : Item(org),
      impl(new CrossSectionItemImpl(this, *org.impl))
{

}


CrossSectionItemImpl::CrossSectionItemImpl(CrossSectionItem* self, const CrossSectionItemImpl& org)
    : self(self),
      config(new DoseConfigDialog)
{
    position = org.position;
    nodeData = org.nodeData;
    gammaData = org.gammaData;
    gammaDataFile = org.gammaDataFile;
    colorScale = org.colorScale;
    initialize();
}


CrossSectionItem::~CrossSectionItem()
{
    delete impl;
}


void CrossSectionItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<CrossSectionItem>(N_("CrossSectionItem"))
            .addCreationPanel<CrossSectionItem>()

            .addLoader<CrossSectionItem>(
                _("Dose Distribution"), "GAMMA-DATA", "gbin",
                [](CrossSectionItem* item, const string& filename, ostream& os, Item*){ return item->impl->load(filename); });

    ItemTreeView::instance()->customizeContextMenu<CrossSectionItem>(
        [](CrossSectionItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.setPath("/").setPath(_("PHITS"));
            menuManager.addItem(_("Start"))->sigTriggered().connect(
                        [item](){ item->impl->config->onItemTriggered(true); });
            menuManager.addItem(_("Stop"))->sigTriggered().connect(
                        [item](){ item->impl->config->onItemTriggered(false); });
            menuManager.setPath("/");
            menuManager.addItem(_("Configuration of PHITS"))->sigTriggered().connect(
                [item](){ item->impl->config->show(); });
            menuManager.setPath("/");
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
    });
}


SgNode* CrossSectionItem::getScene()
{
    if(!impl->scene) {
        impl->createScene();
    }
    return impl->scene;
}


void CrossSectionItemImpl::initialize()
{
    config->sigValueChanged().connect([&](double value){ onValueChanged(); });
    config->plainCombo->sigCurrentIndexChanged().connect([&](int index){ onValueChanged(); });
    config->sigReadPHITSData().connect([&](string filename){ onReadPHITSData(filename); });
}


void CrossSectionItemImpl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform;
        updateScenePosition();
    } else {
        scene->clearChildren();
    }

    SgGroup* group = new SgGroup;
    scene->addChild(group);

    if(nodeData) {
        int id = config->plainCombo->currentIndex();
        static const int coordID[][3] = { { 0, 1, 2 }, { 1, 2, 0 }, { 2, 0, 1 } };

        const vector<double>& xcoord = nodeData->coordinates(coordID[id][0]);
        const vector<double>& ycoord = nodeData->coordinates(coordID[id][1]);
        const vector<double>& zcoord = nodeData->coordinates(coordID[id][2]);
        double xrange = xcoord[xcoord.size() - 1] - xcoord[0];
        double yrange = ycoord[ycoord.size() - 1] - ycoord[0];
        size_t nx = nodeData->size(coordID[id][0]);
        size_t ny = nodeData->size(coordID[id][1]);
        size_t nz = nodeData->size(coordID[id][2]);
        int width = (int)nx;
        int height = (int)ny;

        int index = -1;
        config->dspin->setRange(zcoord[0], zcoord[zcoord.size() - 1]);
        double z = config->dspin->value();
        for(int i = 0; i < nz; ++i) {
            if((z >= zcoord[i]) && (z < zcoord[i + 1])) {
                index = i;
            }
        }

        for(int j = 0; j < height; ++j) {
            for(int i = 0; i < width; ++i) {
                double xside = xrange / (double)nx;
                double yside = yrange / (double)ny;
                double xcenter = xcoord[0] + xrange / 2.0;
                double ycenter = ycoord[0] + yrange / 2.0;

                ColorScale scale;
                double min = nodeData->min();
                double max = nodeData->max();
                int exp = 1 + (int)floor(log10(fabs(max)));
                min = 1.0 * pow(10, exp - 4);
//                max = 1.0 * pow(10, exp + 1);

                const int positionID[][3] = {
                    { (int)i, (int)j, index},
                    { index, (int)i, (int)j },
                    { (int)j, index, (int)i }
                };

                scale.setRange(min, max);
                Vector3 color;
                if(index != -1) {
                    if(colorScale.is(LOG_SCALE)) {
                        color = scale.logColor(nodeData->value(positionID[id][0], positionID[id][1], positionID[id][2]));
                    } else if(colorScale.is(LINER_SCALE)){
                        color = scale.linerColor(nodeData->value(positionID[id][0], positionID[id][1], positionID[id][2]));
                    }
                } else {
                    if(colorScale.is(LOG_SCALE)) {
                        color = scale.logColor(0.0);
                    } else if(colorScale.is(LINER_SCALE)){
                        color = scale.linerColor(0.0);
                    }
                }

                SgPosTransform* pos = new SgPosTransform;
                SgShape* shape = new SgShape;
                shape->setMesh(generateSquare(xside, yside));
                SgMaterial* material = new SgMaterial;
                material->setDiffuseColor(color);
                material->setTransparency(0.5);
                shape->setMaterial(material);
                pos->addChild(shape);
                double x = -xside / 2.0 * (double)(width - 1) + (double)i * xside;
                double y = -yside / 2.0 * (double)(height - 1) + (double)j * yside;
                x = x + xcenter;
                y = y + ycenter;
                double z = config->dspin->value();
                pos->setTranslation(Vector3(x, y, z));
                group->addChild(pos);
            }
        }
    }
}


void CrossSectionItemImpl::updateScenePosition()
{
    if(scene) {
        int index = config->plainCombo->currentIndex();
        Vector3 rpy(0.0, 0.0, 0.0);
        if(index == XY) {
            rpy = Vector3(0.0, 0.0, 0.0);
        } else if(index == YZ) {
            rpy = Vector3(90.0, 0.0, 90.0);
        } else if(index == ZX) {
            rpy = Vector3(90.0, -90.0, 180.0);
        }
        Matrix3 rotation = rotFromRpy(radian(rpy));
        scene->setRotation(rotation);
        scene->notifyUpdate();
    }
}


void CrossSectionItemImpl::onValueChanged()
{
    if(scene) {
        createScene();
        updateScenePosition();
        self->notifyUpdate();
    }
}


GammaData& CrossSectionItem::gammaData() const
{
    return impl->gammaData;
}


OrthoNodeData* CrossSectionItem::nodeData() const
{
    return impl->nodeData;
}


SignalProxy<void()> CrossSectionItem::sigGammaDataLoaded() const
{
    return impl->sigGammaDataLoaded_;
}


void CrossSectionItemImpl::onReadPHITSData(const string& filename)
{
    this->gammaDataFile = filename;
    if(!this->gammaDataFile.empty()) {
        if(load(this->gammaDataFile)) {
            sigGammaDataLoaded_();
        }
    }
}


bool CrossSectionItemImpl::load(const string& filename)
{
    if(!filename.empty()) {
        if(gammaData.read(filename)) {
            if(gammaData.getDataHeaderInfo(gammaData.geometryInfo(0))) {
                nodeData = new OrthoNodeData(gammaData);
                onValueChanged();
            }
        }
    }
    return true;
}


void CrossSectionItemImpl::setDefaultNuclideTableFile(const string& filename)
{
    config->defaultNuclideTableFile = filename;
}


void CrossSectionItemImpl::setDefaultElementTableFile(const string& filename)
{
    config->defaultElementTableFile = filename;
}


bool CrossSectionItemImpl::onColorScalePropertyChanged(const int& index)
{
    colorScale.selectIndex(index);
    createScene();
    scene->notifyUpdate();
    return true;
}


Item* CrossSectionItem::doDuplicate() const
{
    return new CrossSectionItem(*this);
}


void CrossSectionItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void CrossSectionItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("ColorScale"), colorScale,
                [&](int index){ return onColorScalePropertyChanged(index); });
    FilePathProperty nuclideFileProperty(
                config->defaultNuclideTableFile, { _("Nuclide definition file (*.yaml)") });
    putProperty(_("Default nuclide table"), nuclideFileProperty,
                [&](const string& filename){ setDefaultNuclideTableFile(filename); return true; });
    FilePathProperty elementFileProperty(
                config->defaultElementTableFile, { _("Element definition file (*.yaml)") });
    putProperty(_("Default element table"), elementFileProperty,
                [&](const string& filename){ setDefaultElementTableFile(filename); return true; });
}


bool CrossSectionItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool CrossSectionItemImpl::store(Archive& archive)
{
    config->storeState(archive);
    archive.writeRelocatablePath("gamma_data_file", gammaDataFile);
    archive.write("color_scale", colorScale.selectedIndex());
    return true;
}


bool CrossSectionItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool CrossSectionItemImpl::restore(const Archive& archive)
{
    config->restoreState(archive);
    archive.readRelocatablePath("gamma_data_file", gammaDataFile);
    if(!gammaDataFile.empty()) {
        load(gammaDataFile);
    }
    colorScale.selectIndex(archive.get("color_scale", 0));
    return true;
}


DoseConfigDialog::DoseConfigDialog()
{
    setWindowTitle(_("Configuration of PHITS"));

    gammaDataFile.clear();
    codeCombo = new ComboBox;
    QStringList codes;
    string env = getenv("PATH");
    if(env.find("QADGP2R") != string::npos) {
        codes << "PHITS" << "QAD";
    } else {
        codes << "PHITS";
    }
    codeCombo->addItems(codes);
    codeCombo->setCurrentIndex(PHITS);

    maxCasSpin = new SpinBox;
    maxCasSpin->setRange(1, INT_MAX);
    maxCasSpin->setValue(1000);

    maxBchSpin = new SpinBox;
    maxBchSpin->setRange(1, 1000000);
    maxBchSpin->setValue(2);

    QGridLayout* rgbox = new QGridLayout;
    static const char* xyzLabels[] = { "X (n, min[m], max[m])", "Y (n, min[m], max[m])", "Z (n, min[m], max[m])", "Energy (n, min[m], max[m])" };
    for(int i = 0; i < MAX_PARAMETER; ++i) {
        //rgbox->addWidget(new QLabel(xyzLabels[i], frame), i + 1, 0, Qt::AlignCenter);
        rgbox->addWidget(new QLabel(xyzLabels[i]), i + 1, 0, Qt::AlignCenter);
        rgbox->addWidget(&nSpin[i], i + 1, 1);
        rgbox->addWidget(&minSpin[i], i + 1, 2);
        rgbox->addWidget(&maxSpin[i], i + 1, 3);

        if(i < 3) {
            nSpin[i].setAlignment(Qt::AlignCenter);
            nSpin[i].setRange(2, 100);
            nSpin[i].setValue(3);
            minSpin[i].setAlignment(Qt::AlignCenter);
            minSpin[i].setRange(-100, 100.);
            minSpin[i].setDecimals(1);
            minSpin[i].setSingleStep(0.1);
            minSpin[i].setValue(-1.0);
            maxSpin[i].setAlignment(Qt::AlignCenter);
            maxSpin[i].setRange(-100., 100.);
            maxSpin[i].setDecimals(1);
            maxSpin[i].setSingleStep(0.1);
            maxSpin[i].setValue(1.0);
        } else {
            nSpin[i].setAlignment(Qt::AlignCenter);
            nSpin[i].setRange(1, 100);
            nSpin[i].setValue(1);
            minSpin[i].setAlignment(Qt::AlignCenter);
            minSpin[i].setRange(0.0, 100.);
            minSpin[i].setDecimals(1);
            minSpin[i].setSingleStep(0.1);
            minSpin[i].setValue(0.0);
            maxSpin[i].setAlignment(Qt::AlignCenter);
            maxSpin[i].setRange(0.0, 100.);
            maxSpin[i].setDecimals(1);
            maxSpin[i].setSingleStep(0.1);
            maxSpin[i].setValue(10.0);
        }
    }

    messageCheck = new CheckBox;
    messageCheck->setChecked(true);
    messageCheck->setText(_("Put messages"));
    messageCheck->sigToggled().connect([&](bool on){ phitsRunner.putMessages(on); });

    QGroupBox* rangeBox = new QGroupBox;
    rangeBox->setTitle(_("Dose Distribution Range"));
    rangeBox->setAlignment(Qt::AlignCenter);
    rangeBox->setLayout(rgbox);

    QGridLayout* gbox0 = new QGridLayout;
    int index = 0;
    gbox0->addWidget(new QLabel(_("maxcas")), index, 0);
    gbox0->addWidget(maxCasSpin, index, 1);
    gbox0->addWidget(new QLabel(_("maxbch")), index, 2);
    gbox0->addWidget(maxBchSpin, index++, 3);
    gbox0->addWidget(new QLabel(_("Code")), index, 0);
    gbox0->addWidget(codeCombo, index, 1);
    gbox0->addWidget(messageCheck, index++, 2, 1, 2);

    QGridLayout* gbox1 = new QGridLayout;
    static const char* labels[] = { "XY", "YZ", "ZX" };

    plainCombo = new ComboBox;
    for(int i = 0; i < 3; ++i) {
        plainCombo->addItem(labels[i]);
    }
    dspin = new DoubleSpinBox;
    dspin->setRange(0.0, 100.0);
    dspin->setDecimals(3);
    dspin->setSingleStep(0.01);
    slider = new Slider(Qt::Horizontal);
    slider->setRange(0, 100);
    gbox1->addWidget(plainCombo, 0, 0, 1, 1);
    gbox1->addWidget(slider, 0, 1, 1, 2);
    gbox1->addWidget(dspin, 0, 3, 1, 1);

    dspin->sigValueChanged().connect([&](double value){
        sigValueChanged_(value);
        double min = dspin->minimum();
        double max = dspin->maximum();
        double rate = (value - min) / (max - min) * 100.0;
        slider->setValue((int)rate);
    });
    slider->sigValueChanged().connect([&](int rate){
        double min = dspin->minimum();
        double max = dspin->maximum();
        double value = (max - min) * (double)rate / 100.0 + min;
        dspin->setValue(value);
    });

    auto buttonBox = new QDialogButtonBox(this);
    auto okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox, &QDialogButtonBox::accepted, [this](){ this->accept(); });

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(new HSeparatorBox(new QLabel("PHITS/QAD")));
    vbox->addWidget(rangeBox);
    vbox->addLayout(gbox0);
    vbox->addLayout(new HSeparatorBox(new QLabel(_("Plain"))));
    vbox->addLayout(gbox1);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    phitsRunner.sigReadPHITSData().connect([&](string filename){ readPHITSData(filename); });
    for(int i = 0; i < MAX_PROCESS; ++i) {
        qadRunners[i].sigReadPHITSData().connect([&](string filename){ readPHITSData(filename); });
    }

    defaultNuclideTableFile = toUTF8((shareDirPath() / "default" / "nuclides.yaml").string());
    defaultElementTableFile = toUTF8((shareDirPath() / "default" / "elements.yaml").string());
}


bool DoseConfigDialog::readPHITSData(const string& filename)
{
    GammaData gammaData;
    bool result = false;
    filesystem::path path(filename);
    gammaDataFile = toUTF8((path.parent_path() / path.stem()).string()) + ".gbin";

    int index = codeCombo->currentIndex();
    if(index == PHITS) {
        result = gammaData.readPHITS(filename, GammaData::DOSERATE);
        if(result) {
            result = gammaData.write(gammaDataFile);
            result = result && gammaData.setDataHeaderInfo(gammaData.geometryInfo(0));
        }
    } else if(index == QAD) {
        QString baseName = path.stem().string().c_str();
        QStringList list = baseName.split("_");
        int numSources;
        if(list.size() > 1) {
            numSources = list[1].toInt();
        } else {
            numSources = 0;
        }
        result = gammaData.readQAD(filename, calcInfo, numSources);
        if(result) {
            countQAD += 1;
            MessageView::instance()->putln(fmt::format("QAD process: {0}", countQAD));
            if(countQAD == 1) {
                qadTotDoseDataFile = gammaData;
            } else {
                qadTotDoseDataFile.addDataInfo(gammaData.dataInfo());
            }
            if(countQAD == calcInfo.nSrc) {
                result = qadTotDoseDataFile.write(gammaDataFile);
                result = result && qadTotDoseDataFile.setDataHeaderInfo(qadTotDoseDataFile.geometryInfo(0));
            }
        }
    }

    if(result) {
        sigReadPHITSData_(gammaDataFile);
    }
    return result;
}


void DoseConfigDialog::onItemTriggered(const bool& on)
{
    if(on) {
        filesystem::path homeDir(getenv("HOME"));
        QDateTime recordingStartTime = QDateTime::currentDateTime();
        string suffix = recordingStartTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
        string phitsDirPath = toUTF8((homeDir / "phits_ws" / ("phits" + suffix).c_str()).string());
        filesystem::path phitsDir(fromUTF8(phitsDirPath));
        if(!filesystem::exists(phitsDir)) {
            filesystem::create_directories(phitsDir);
        }

        int index = codeCombo->currentIndex();
        string filename;
        if(index == PHITS) {
            filename = toUTF8((phitsDir / "phits").string()) + ".inp";
        } else if(index == QAD) {
            filename = toUTF8((phitsDir / "qad").string()) + ".inp";
        }

        bool result = false;

        string filename0;
        calcInfo.maxcas = maxCasSpin->value();
        calcInfo.maxbch = maxBchSpin->value();
        for(size_t i = 0; i < 4; ++i) {
            calcInfo.xyze[i].n = nSpin[i].value();
            calcInfo.xyze[i].min = minSpin[i].value();
            calcInfo.xyze[i].max = maxSpin[i].value();
        }
        calcInfo.inputMode = GammaData::DOSERATE;
        filesystem::path filePath(filename);
        filesystem::path dir(filePath.parent_path());

        if(index == PHITS) {
            filename0 = toUTF8((dir / "dose_xy.out").string());
            PHITSWriter phitsWriter;
            phitsWriter.setDefaultNuclideTableFile(defaultNuclideTableFile);
            phitsWriter.setDefaultElementTableFile(defaultElementTableFile);
            result = writeTextFile(filename, phitsWriter.writePHITS(calcInfo));
        } else if(index == QAD) {
            filename0 = toUTF8((dir / filePath.stem()).string()) + ".out";
            QADWriter qadWriter;
            qadWriter.setDefaultNuclideTableFile(defaultNuclideTableFile);
            qadWriter.setDefaultElementTableFile(defaultElementTableFile);
            result = writeTextFile(filename, qadWriter.writeQAD(calcInfo, 0));
            if(result) {
                copyQADLIB(filename);
                for(int i = 1; i < calcInfo.nSrc; ++i) {
                    string filename1 = toUTF8((dir / filePath.stem()).string()) + "_" + to_string(i) + ".inp";
                    result = writeTextFile(filename1, qadWriter.writeQAD(calcInfo, i));
                }
            }
        }

        if(result) {
            phitsRunner.setReadStandardOutput(filename0, GammaData::DOSERATE);
            int index = codeCombo->currentIndex();
            if(index == PHITS) {
                phitsRunner.startPHITS(filename.c_str());
            } else if(index == QAD) {
                countQAD = 0;
                phitsRunner.startQAD(filename.c_str(), filename0.c_str());
                for(int i = 1; i < calcInfo.nSrc; ++i) {
                    string filename1 = toUTF8((dir / filePath.stem()).string()) + "_" + to_string(i) + ".inp";
                    string filename2 = toUTF8((dir / filePath.stem()).string()) + "_" + to_string(i) + ".out";
                    qadRunners[i].setReadStandardOutput(filename2, GammaData::DOSERATE);
                    qadRunners[i].startQAD(filename1, filename2);
                }
            }
        }
    } else {
        phitsRunner.stop();
    }
}


bool DoseConfigDialog::storeState(Archive& archive)
{
    archive.write("code", codeCombo->currentIndex());
    archive.write("maxcas", maxCasSpin->value());
    archive.write("maxbch", maxBchSpin->value());
    archive.write("nx", nSpin[0].value());
    archive.write("ny", nSpin[1].value());
    archive.write("nz", nSpin[2].value());
    archive.write("ne", nSpin[3].value());
    archive.write("xmin", minSpin[0].value());
    archive.write("ymin", minSpin[1].value());
    archive.write("zmin", minSpin[2].value());
    archive.write("emin", minSpin[3].value());
    archive.write("xmax", maxSpin[0].value());
    archive.write("ymax", maxSpin[1].value());
    archive.write("zmax", maxSpin[2].value());
    archive.write("emax", maxSpin[3].value());
    archive.write("z", dspin->value());
    archive.write("plain", plainCombo->currentIndex());
    archive.write("put_messages", messageCheck->isChecked());
    archive.writeRelocatablePath("default_nuclide_table_file", defaultNuclideTableFile);
    archive.writeRelocatablePath("default_element_table_file", defaultElementTableFile);
    return true;
}


bool DoseConfigDialog::restoreState(const Archive& archive)
{
    codeCombo->setCurrentIndex(archive.get("code", 0));
    maxCasSpin->setValue(archive.get("maxcas", 0));
    maxBchSpin->setValue(archive.get("maxbch", 0));
    nSpin[0].setValue(archive.get("nx", 0));
    nSpin[1].setValue(archive.get("ny", 0));
    nSpin[2].setValue(archive.get("nz", 0));
    nSpin[3].setValue(archive.get("ne", 0));
    minSpin[0].setValue(archive.get("xmin", 0.0));
    minSpin[1].setValue(archive.get("ymin", 0.0));
    minSpin[2].setValue(archive.get("zmin", 0.0));
    minSpin[3].setValue(archive.get("emin", 0.0));
    maxSpin[0].setValue(archive.get("xmax", 0.0));
    maxSpin[1].setValue(archive.get("ymax", 0.0));
    maxSpin[2].setValue(archive.get("zmax", 0.0));
    maxSpin[3].setValue(archive.get("emax", 0.0));
    dspin->setValue(archive.get("z", 0.0));
    plainCombo->setCurrentIndex(archive.get("plain", 0));
    messageCheck->setChecked(archive.get("put_messages", true));
    archive.readRelocatablePath("default_nuclide_table_file", defaultNuclideTableFile);
    archive.readRelocatablePath("default_element_table_file", defaultElementTableFile);
    return true;
}