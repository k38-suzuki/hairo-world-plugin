/**
   \file
   \author Kenta Suzuki
*/

#include "GratingGenerator.h"
#include <cnoid/Button>
#include <cnoid/Dialog>
#include <cnoid/EigenTypes>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

struct DoubleSpinInfo
{
    int row;
    int column;
    double min;
    double max;
    double step;
    double decimals;
    double value;
};


DoubleSpinInfo doubleSpinInfo[] = {
    { 0, 1, 0.01, 1000.0, 0.01, 3, 1.000 },
    { 0, 3, 0.01, 1000.0, 0.01, 3, 0.038 },
    { 2, 1, 0.01, 1000.0, 0.01, 3, 0.005 },
    { 2, 3, 0.01, 1000.0, 0.01, 3, 0.006 },
    { 3, 1, 0.01, 1000.0, 0.01, 3, 0.010 },
    { 3, 3, 0.01, 1000.0, 0.01, 3, 0.100 }
};


struct SpinInfo
{
    int row;
    int column;
    int min;
    int max;
    int value;
};


SpinInfo spinInfo[] = {
    { 1, 1, 0, 1000, 50 },
    { 1, 3, 0, 1000,  5 }
};

}


namespace cnoid {

class GratingConfigDialog : public Dialog
{
public:
    GratingConfigDialog();

    enum DoubleSpinId {
        MASS, HEIGHT, FRAME_WDT,
        FRAME_HGT, GRID_WDT, GRID_HGT,
        NUM_DSPINS
    };
    enum SpinId { H_GRID, V_GRID, NUM_SPINS };

    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];

    QLabel* sizeLabel;
    PushButton* colorButton;
    FileFormWidget* formWidget;

    bool writeYaml(const string& filename);
    void onColorButtonClicked();
    void onValueChanged();
    VectorXd calcInertia();
};


class GratingGeneratorImpl
{
public:
    GratingGeneratorImpl(GratingGenerator* self, ExtensionManager* ext);
    GratingGenerator* self;

    GratingConfigDialog* dialog;
};

}


GratingGenerator::GratingGenerator(ExtensionManager* ext)
{
    impl = new GratingGeneratorImpl(this, ext);
}


GratingGeneratorImpl::GratingGeneratorImpl(GratingGenerator* self, ExtensionManager* ext)
    : self(self)
{
    dialog = new GratingConfigDialog;

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("BodyGenerator"));
    mm.addItem(_("Grating"))->sigTriggered().connect([&](){ dialog->show(); });
}


GratingGenerator::~GratingGenerator()
{
    delete impl;
}


void GratingGenerator::initialize(ExtensionManager* ext)
{
    ext->manage(new GratingGenerator(ext));
}


GratingConfigDialog::GratingConfigDialog()
{
    setWindowTitle(_("Grating Builder"));
    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;

    const char* dlabels[] = {
        _("Mass [kg]"), _("Height [m]"), _("Frame width [m]"),
        _("Frame height [m]"), _("Grid width [m]"), _("Grid height [m]")
    };

    const char* slabels[] = { _("Horizontal grid [-]"), _("Vertical grid [-]") };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        dspin->setRange(info.min, info.max);
        dspin->setSingleStep(info.step);
        dspin->setDecimals(info.decimals);
        dspin->setValue(info.value);
        gbox->addWidget(new QLabel(dlabels[i]), info.row, info.column - 1);
        gbox->addWidget(dspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        spins[i] = new SpinBox;
        SpinBox* spin = spins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
        gbox->addWidget(new QLabel(slabels[i]), info.row, info.column - 1);
        gbox->addWidget(spin, info.row, info.column);
    }

    sizeLabel = new QLabel(_(" "));
    colorButton = new PushButton;

    dspins[MASS]->setValue(1.0);
    dspins[FRAME_WDT]->setValue(0.005);
    dspins[FRAME_HGT]->setValue(0.006);
    dspins[GRID_WDT]->setValue(0.01);
    dspins[GRID_HGT]->setValue(0.1);
    spins[H_GRID]->setValue(50);
    spins[V_GRID]->setValue(5);
    dspins[HEIGHT]->setValue(0.038);

    gbox->addWidget(new QLabel(_("Color [-]")), 4, 0);
    gbox->addWidget(colorButton, 4, 1);
    gbox->addWidget(new QLabel(_("Size [m, m, m]")), 5, 0);
    gbox->addWidget(sizeLabel, 5, 1, 1, 3);

    formWidget = new FileFormWidget;

    vbox->addLayout(gbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    onValueChanged();

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    dspins[FRAME_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[FRAME_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[GRID_WDT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[GRID_HGT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    spins[H_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    spins[V_GRID]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    dspins[HEIGHT]->sigValueChanged().connect([&](double value){ onValueChanged(); });
    formWidget->sigClicked().connect([&](string filename){ writeYaml(filename); });
}


bool GratingConfigDialog::writeYaml(const string& filename)
{
    filesystem::path path(filename);
    string name = path.stem().string();

    double mass = dspins[MASS]->value();
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();
    double height = dspins[HEIGHT]->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    if(!filename.empty()) {
        YAMLWriter writer(filename);
        writer.startMapping(); {
            writer.putKeyValue("format", "ChoreonoidBody");
            writer.putKeyValue("formatVersion", "1.0");
            writer.putKeyValue("angleUnit", "degree");
            writer.putKeyValue("name", name);
            writer.putKey("links");
            writer.startListing(); {
                writer.startMapping(); {
                    writer.putKeyValue("name", name);
                    writer.putKeyValue("jointType", "free");
                    writer.putKey("centerOfMass");
                    writer.startFlowStyleListing(); {
                        for(int i = 0; i < 3; ++i) {
                            writer.putScalar(0.0);
                        }
                    } writer.endListing(); // end of centerOfMass list
                    writer.putKeyValue("mass", mass);
                    writer.putKey("inertia");
                    writer.startFlowStyleListing(); {
                        VectorXd inertia;
                        inertia.resize(9);
                        inertia = calcInertia();
                        for(int i = 0; i < 9; ++i) {
                            writer.putScalar(inertia[i]);
                        }
                    } writer.endListing(); // end of inertia list
                    writer.putKey("elements");
                    writer.startMapping(); {
                        double sx = -1.0 * w / 2.0;
                        double sy = -1.0 * h / 2.0;
                        Vector6 spine;
                        spine << 0.0, 0.0, -height / 2.0, 0.0, 0.0, height / 2.0;
                        writer.putKey("Visual");
                        writer.startMapping(); {
                            writer.putKey("elements");
                            writer.startMapping(); {
                                writer.putKey("Shape");
                                writer.startMapping(); {
                                    writer.putKey("geometry");
                                    writer.startMapping(); {
                                        writer.putKeyValue("type", "Extrusion");
                                        writer.putKey("crossSection");
                                        writer.startFlowStyleListing(); {
                                            writer.putScalar(sx);
                                            writer.putScalar(sy);

                                            for(int i = 0; i < horizontalGrid; ++i) {
                                                double x = sx + frameWidth + (frameWidth + gridWidth) * i;
                                                writer.putScalar(x - 0.0002);
                                                writer.putScalar(sy);
                                                writer.putScalar(x - 0.0002);
                                                writer.putScalar(sy + (frameHeight + gridHeight) * verticalGrid);
                                                writer.putScalar(x - 0.0001);
                                                writer.putScalar(sy + (frameHeight + gridHeight) * verticalGrid);
                                                writer.putScalar(x - 0.0001);
                                                writer.putScalar(sy);

                                                writer.putScalar(x);
                                                writer.putScalar(sy);

                                                for(int j = 0; j < verticalGrid; ++j) {
                                                    double y = sy + (frameHeight + gridHeight) * j;
                                                    writer.putScalar(x + gridWidth - 0.000001);
                                                    writer.putScalar(y);
                                                    writer.putScalar(x + gridWidth - 0.000001);
                                                    writer.putScalar(y + frameHeight);
                                                    writer.putScalar(x);
                                                    writer.putScalar(y + frameHeight);
                                                    writer.putScalar(x);
                                                    writer.putScalar(y + frameHeight + gridHeight);
                                                }
                                                writer.putScalar(x + gridWidth);
                                                writer.putScalar(sy + (frameHeight + gridHeight) * verticalGrid);
                                                writer.putScalar(x + gridWidth);
                                                writer.putScalar(sy);
                                            }

                                            writer.putScalar(-sx);
                                            writer.putScalar(sy);
                                            writer.putScalar(-sx);
                                            writer.putScalar(-sy);
                                            writer.putScalar(sx);
                                            writer.putScalar(-sy);

                                            writer.putScalar(sx);
                                            writer.putScalar(-sy - frameHeight + 0.000002);
                                            writer.putScalar(-sx - 0.000001);
                                            writer.putScalar(-sy - frameHeight + 0.000002);
                                            writer.putScalar(-sx - 0.000001);
                                            writer.putScalar(-sy - frameHeight + 0.000001);
                                            writer.putScalar(sx);
                                            writer.putScalar(-sy - frameHeight + 0.000001);

                                            writer.putScalar(sx);
                                            writer.putScalar(sy);
                                        } writer.endListing(); // end of crossSection list
                                        writer.putKey("spine");
                                        writer.startFlowStyleListing(); {
                                            for(int i = 0; i < 6; ++i) {
                                                writer.putScalar(spine[i]);
                                            }
                                        } writer.endListing(); // end of spine list
                                    } writer.endMapping(); // end of geometry map
                                    writer.putKey("appearance");
                                    writer.startFlowStyleMapping(); {
                                        writer.putKey("material");
                                        writer.startMapping(); {
                                            writer.putKey("diffuseColor");
                                            QPalette palette = colorButton->palette();
                                            QColor color = palette.color(QPalette::Button);
                                            double red = (double)color.red() / 255.0;
                                            double green = (double)color.green() / 255.0;
                                            double blue = (double)color.blue() / 255.0;
                                            Vector3 diffuseColor(red, green, blue);
                                            writer.startFlowStyleListing(); {
                                                for(int i = 0; i < 3; ++i) {
                                                    writer.putScalar(diffuseColor[i]);
                                                }
                                            } writer.endListing(); // end of diffuseColor list
                                        } writer.endMapping(); // end of material map
                                    } writer.endMapping(); // end of appearance map
                                } writer.endMapping(); // end of Shape map
                            } writer.endMapping(); // end of elements map
                        } writer.endMapping(); // end of Visual map

                        writer.putKey("Collision");
                        writer.startMapping(); {
                            writer.putKey("elements");
                            writer.startMapping(); {
                                writer.putKey("Shape");
                                writer.startMapping(); {
                                    writer.putKey("geometry");
                                    writer.startMapping(); {
                                        writer.putKeyValue("type", "Extrusion");
                                        writer.putKey("crossSection");
                                        writer.startFlowStyleListing(); {
                                            writer.putScalar(sx);
                                            writer.putScalar(sy);
                                            writer.putScalar(-sx);
                                            writer.putScalar(sy);
                                            writer.putScalar(-sx);
                                            writer.putScalar(-sy);
                                            writer.putScalar(sx);
                                            writer.putScalar(-sy);
                                            writer.putScalar(sx);
                                            writer.putScalar(sy);
                                        } writer.endListing(); // end of crossSection list
                                        writer.putKey("spine");
                                        writer.startFlowStyleListing(); {
                                            for(int i = 0; i < 6; ++i) {
                                                writer.putScalar(spine[i]);
                                            }
                                        } writer.endListing(); // end of spine list
                                    } writer.endMapping(); // end of geometry map
                                } writer.endMapping(); // end of Shape map
                            } writer.endMapping(); // end of elements map
                        } writer.endMapping(); // end of Collision map
                    } writer.endMapping(); // end of elements map
                } writer.endMapping(); // end of links map
            } writer.endListing(); // end of links list
        } writer.endMapping(); // end of body map
    }
    return true;
}


void GratingConfigDialog::onColorButtonClicked()
{
    QColor selectedColor;
    QColor currentColor = colorButton->palette().color(QPalette::Button);
    QColorDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Select a color"));
    dialog.setCurrentColor(currentColor);
    dialog.setOption (QColorDialog::DontUseNativeDialog);
    if(dialog.exec()) {
        selectedColor = dialog.currentColor();
    } else {
        selectedColor = currentColor;
    }

    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    colorButton->setPalette(palette);
}


void GratingConfigDialog::onValueChanged()
{
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();
    double height = dspins[HEIGHT]->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    QString size = QString::number(w, 'f', 3)
            + ", " + QString::number(h, 'f', 3)
            + ", " + QString::number(height, 'f', 3);
    sizeLabel->setText(size);
}


VectorXd GratingConfigDialog::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);
    inertia << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

    double mass = dspins[MASS]->value();
    double frameWidth = dspins[FRAME_WDT]->value();
    double frameHeight = dspins[FRAME_HGT]->value();
    double gridWidth = dspins[GRID_WDT]->value();
    double gridHeight = dspins[GRID_HGT]->value();
    int horizontalGrid = spins[H_GRID]->value();
    int verticalGrid = spins[V_GRID]->value();

    double x = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double y = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;
    double z = dspins[HEIGHT]->value();

    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    inertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;

    return inertia;
}
