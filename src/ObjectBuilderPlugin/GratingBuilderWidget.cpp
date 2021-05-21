/**
   \file
   \author Kenta Suzuki
*/

#include "GratingBuilderWidget.h"
#include <cnoid/Button>
#include <cnoid/EigenTypes>
#include <cnoid/MainWindow>
#include <cnoid/SpinBox>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <QColorDialog>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"
using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class GratingBuilderWidgetImpl
{
public:
    GratingBuilderWidgetImpl(GratingBuilderWidget* self);
    GratingBuilderWidget* self;

    DoubleSpinBox* massSpin;
    DoubleSpinBox* frameWidthSpin;
    DoubleSpinBox* frameHeightSpin;
    DoubleSpinBox* gridWidthSpin;
    DoubleSpinBox* gridHeightSpin;
    SpinBox* horizontalGridSpin;
    SpinBox* verticalGridSpin;
    DoubleSpinBox* heightSpin;
    QLabel* sizeLabel;
    PushButton* colorButton;

    void writeYaml(const string& filename);
    void onColorButtonClicked();
    void onValueChanged();
    VectorXd calcInertia();
};

}


GratingBuilderWidget::GratingBuilderWidget()
{
    impl = new GratingBuilderWidgetImpl(this);
}


GratingBuilderWidgetImpl::GratingBuilderWidgetImpl(GratingBuilderWidget* self)
    : self(self)
{
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();

    massSpin = new DoubleSpinBox();
    massSpin->setRange(0.001, 1000.0);
    massSpin->setSingleStep(0.01);
    massSpin->setDecimals(3);
    frameWidthSpin = new DoubleSpinBox();
    frameWidthSpin->setRange(0.001, 1000.0);
    frameWidthSpin->setSingleStep(0.01);
    frameWidthSpin->setDecimals(3);
    frameHeightSpin = new DoubleSpinBox();
    frameHeightSpin->setRange(0.001, 1000.0);
    frameHeightSpin->setSingleStep(0.01);
    frameHeightSpin->setDecimals(3);
    gridWidthSpin = new DoubleSpinBox();
    gridWidthSpin->setRange(0.001, 1000.0);
    gridWidthSpin->setSingleStep(0.01);
    gridWidthSpin->setDecimals(3);
    gridHeightSpin = new DoubleSpinBox();
    gridHeightSpin->setRange(0.001, 1000.0);
    gridHeightSpin->setSingleStep(0.01);
    gridHeightSpin->setDecimals(3);
    horizontalGridSpin = new SpinBox();
    horizontalGridSpin->setRange(0, 1000);
    verticalGridSpin = new SpinBox();
    verticalGridSpin->setRange(0, 1000);
    heightSpin = new DoubleSpinBox();
    heightSpin->setRange(0.001, 1000.0);
    heightSpin->setSingleStep(0.01);
    heightSpin->setDecimals(3);
    sizeLabel = new QLabel(_(" "));
    colorButton = new PushButton();

    massSpin->setValue(1.0);
    frameWidthSpin->setValue(0.005);
    frameHeightSpin->setValue(0.006);
    gridWidthSpin->setValue(0.01);
    gridHeightSpin->setValue(0.1);
    horizontalGridSpin->setValue(50);
    verticalGridSpin->setValue(5);
    heightSpin->setValue(0.038);

    int index = 0;
    gbox->addWidget(new QLabel(_("Mass [kg]")), index, 0);
    gbox->addWidget(massSpin, index, 1);
    gbox->addWidget(new QLabel(_("Height [m]")), index, 2);
    gbox->addWidget(heightSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Horizontal grid [-]")), index, 0);
    gbox->addWidget(horizontalGridSpin, index, 1);
    gbox->addWidget(new QLabel(_("Vertical grid [-]")), index, 2);
    gbox->addWidget(verticalGridSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Frame width [m]")), index, 0);
    gbox->addWidget(frameWidthSpin, index, 1);
    gbox->addWidget(new QLabel(_("Frame height [m]")), index, 2);
    gbox->addWidget(frameHeightSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Grid width [m]")), index, 0);
    gbox->addWidget(gridWidthSpin, index, 1);
    gbox->addWidget(new QLabel(_("Grid height [m]")), index, 2);
    gbox->addWidget(gridHeightSpin, index++, 3);
    gbox->addWidget(new QLabel(_("Color [-]")), index, 0);
    gbox->addWidget(colorButton, index++, 1);
    gbox->addWidget(new QLabel(_("Size [m, m, m]")), index, 0);
    gbox->addWidget(sizeLabel, index++, 1, 1, 3);

    vbox->addLayout(gbox);
    self->setLayout(vbox);

    onValueChanged();

    colorButton->sigClicked().connect([&](){ onColorButtonClicked(); });
    frameWidthSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    frameHeightSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    gridWidthSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    gridHeightSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    horizontalGridSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    verticalGridSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
    heightSpin->sigValueChanged().connect([&](double value){ onValueChanged(); });
}


GratingBuilderWidget::~GratingBuilderWidget()
{
    delete impl;
}


void GratingBuilderWidget::save(const string& filename)
{
    impl->writeYaml(filename);
}


void GratingBuilderWidgetImpl::writeYaml(const string& filename)
{
    filesystem::path path(filename);

    double mass = massSpin->value();
    double frameWidth = frameWidthSpin->value();
    double frameHeight = frameHeightSpin->value();
    double gridWidth = gridWidthSpin->value();
    double gridHeight = gridHeightSpin->value();
    int horizontalGrid = horizontalGridSpin->value();
    int verticalGrid = verticalGridSpin->value();
    double height = heightSpin->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    if(!filename.empty()) {
        YAMLWriter writer(filename);
        string name = path.stem();

        writer.startMapping(); // start of body map
        writer.putKeyValue("format", "ChoreonoidBody");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("angleUnit", "degree");
        writer.putKeyValue("name", name);
        writer.putKey("links");
        writer.startListing(); // start of links list
        writer.startMapping(); // start of links map
        writer.putKeyValue("name", name);
        writer.putKeyValue("jointType", "free");
        writer.putKey("centerOfMass");
        writer.startFlowStyleListing(); // start of centerOfMass list
        for(int i = 0; i < 3; ++i) {
            writer.putScalar(0.0);
        }
        writer.endListing(); // end of centerOfMass list
        writer.putKeyValue("mass", mass);
        writer.putKey("inertia");
        writer.startFlowStyleListing(); // start of inertia list
        VectorXd inertia;
        inertia.resize(9);
        inertia = calcInertia();
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
        writer.endListing(); // end of inertia list
        writer.putKey("elements");
        writer.startMapping(); // start of elements map
        writer.putKey("Visual");
        writer.startMapping(); // start of Visual map
        writer.putKey("elements");
        writer.startMapping(); // start of elements map
        writer.putKey("Shape");
        writer.startMapping(); // start of Shape map
        writer.putKey("geometry");
        writer.startMapping(); // start of geometry map
        writer.putKeyValue("type", "Extrusion");
        writer.putKey("crossSection");
        writer.startFlowStyleListing(); // start of crossSection list
        double sx = -1.0 * w / 2.0;
        double sy = -1.0 * h / 2.0;
        writer.putScalar(sx);
        writer.putScalar(sy);

        for(int i = 0; i < horizontalGrid; ++i) {
            double x = sx + frameWidth + (frameWidth + gridWidth) * i;
            writer.putScalar(x);
            writer.putScalar(sy);

            writer.putScalar(x - 0.0002);
            writer.putScalar(sy);
            writer.putScalar(x - 0.0002);
            writer.putScalar(sy + (frameHeight + gridHeight) * verticalGrid);
            writer.putScalar(x - 0.0001);
            writer.putScalar(sy + (frameHeight + gridHeight) * verticalGrid);
            writer.putScalar(x - 0.0001);
            writer.putScalar(sy);

            for(int j = 0; j < verticalGrid; ++j) {
                double y = sy + (frameHeight + gridHeight) * j;
                writer.putScalar(x + gridWidth - 0.0001);
                writer.putScalar(y);
                writer.putScalar(x + gridWidth - 0.0001);
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
        writer.endListing(); // end of crossSection list
        writer.putKey("spine");
        writer.startFlowStyleListing(); // start of spine list
        Vector6 spine;
        spine << 0.0, 0.0, -height / 2.0, 0.0, 0.0, height / 2.0;
        for(int i = 0; i < 6; ++i) {
            writer.putScalar(spine[i]);
        }
        writer.endListing(); // end of spine list
        writer.endMapping(); // end of geometry map
        writer.putKey("appearance");
        writer.startFlowStyleMapping(); // start of appearance map
        writer.putKey("material");
        writer.startMapping(); // start of material map
        writer.putKey("diffuseColor");
        QPalette palette = colorButton->palette();
        QColor color = palette.color(QPalette::Button);
        double red = (double)color.red() / 255.0;
        double green = (double)color.green() / 255.0;
        double blue = (double)color.blue() / 255.0;
        Vector3 diffuseColor(red, green, blue);
        writer.startFlowStyleListing(); // start of diffuseColor list
        for(int i = 0; i < 3; ++i) {
            writer.putScalar(diffuseColor[i]);
        }
        writer.endListing(); // end of diffuseColor list
        writer.endMapping(); // end of material map
        writer.endMapping(); // end of appearance map
        writer.endMapping(); // end of Shape map
        writer.endMapping(); // end of elements map
        writer.endMapping(); // end of Visual map

        writer.putKey("Collision");
        writer.startMapping(); // start of Visual map
        writer.putKey("elements");
        writer.startMapping(); // start of elements map
        writer.putKey("Shape");
        writer.startMapping(); // start of Shape map
        writer.putKey("geometry");
        writer.startMapping(); // start of geometry map
        writer.putKeyValue("type", "Extrusion");
        writer.putKey("crossSection");
        writer.startFlowStyleListing(); // start of crossSection list

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

        writer.endListing(); // end of crossSection list
        writer.putKey("spine");
        writer.startFlowStyleListing(); // start of spine list

        for(int i = 0; i < 6; ++i) {
            writer.putScalar(spine[i]);
        }
        writer.endListing(); // end of spine list
        writer.endMapping(); // end of geometry map

        writer.endMapping(); // end of Shape map
        writer.endMapping(); // end of elements map
        writer.endMapping(); // end of Collision map
        writer.endMapping(); // end of elements map
        writer.endMapping(); // end of links map
        writer.endListing(); // end of links list
        writer.endMapping(); // end of body map
    }
}


void GratingBuilderWidgetImpl::onColorButtonClicked()
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


void GratingBuilderWidgetImpl::onValueChanged()
{
    double frameWidth = frameWidthSpin->value();
    double frameHeight = frameHeightSpin->value();
    double gridWidth = gridWidthSpin->value();
    double gridHeight = gridHeightSpin->value();
    int horizontalGrid = horizontalGridSpin->value();
    int verticalGrid = verticalGridSpin->value();
    double height = heightSpin->value();

    double w = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double h = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;

    QString size = QString::number(w, 'f', 3)
            + ", " + QString::number(h, 'f', 3)
            + ", " + QString::number(height, 'f', 3);
    sizeLabel->setText(size);
}


VectorXd GratingBuilderWidgetImpl::calcInertia()
{
    VectorXd inertia;
    inertia.resize(9);
    inertia << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;

    double mass = massSpin->value();
    double frameWidth = frameWidthSpin->value();
    double frameHeight = frameHeightSpin->value();
    double gridWidth = gridWidthSpin->value();
    double gridHeight = gridHeightSpin->value();
    int horizontalGrid = horizontalGridSpin->value();
    int verticalGrid = verticalGridSpin->value();

    double x = frameWidth * (horizontalGrid + 1) + gridWidth * horizontalGrid;
    double y = frameHeight * (verticalGrid + 1) + gridHeight * verticalGrid;
    double z = heightSpin->value();

    double ix = mass / 12.0 * (y * y + z * z);
    double iy = mass / 12.0 * (z * z + x * x);
    double iz = mass / 12.0 * (x * x + y * y);

    inertia << ix, 0.0, 0.0, 0.0, iy, 0.0, 0.0, 0.0, iz;

    return inertia;
}

