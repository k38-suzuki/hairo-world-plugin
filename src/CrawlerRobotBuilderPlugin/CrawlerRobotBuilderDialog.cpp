/**
   \file
   \author Kenta Suzuki
*/

#include "CrawlerRobotBuilderDialog.h"
#include <cnoid/Button>
#include <cnoid/CheckBox>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/Widget>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <cmath>
#include <stdio.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;

CrawlerRobotBuilderDialog* builderDialog = nullptr;

namespace {

void putKeyVector3(YAMLWriter* writer, const string key, const Vector3 value)
{
    writer->putKey(key);
    writer->startFlowStyleListing();
    for(int i = 0; i < 3; i++) {
        writer->putScalar(value[i]);
    }
    writer->endListing();
}


string boxInertia(double mass, double x, double y, double z)
{
    double ixx = mass * (y * y + z * z) / 12.0;
    double iyy = mass * (z * z + x * x) / 12.0;
    double izz = mass * (x * x + y * y) / 12.0;

    string inertia = to_string(ixx) + ", 0.0, 0.0, "
            + "0.0, " + to_string(iyy) + ", 0.0, "
            + "0.0, 0.0, " + to_string(izz);
    return inertia;
}


string cylinderInertia(double mass, double radius, double height)
{
    double i = mass * (3.0 * radius * radius + height * height) / 12.0;
    double iyy = mass * radius * radius / 2.0;

    string inertia = to_string(i) + ", 0.0, 0.0, "
            + "0.0, " + to_string(iyy) + ", 0.0, "
            + "0.0, 0.0, " + to_string(i);
    return inertia;
}

}


namespace cnoid {

class CrawlerRobotBuilderDialogImpl
{
public:
    CrawlerRobotBuilderDialogImpl(CrawlerRobotBuilderDialog* self);


    CrawlerRobotBuilderDialog* self;
    DoubleSpinBox* chassisMassSpin;
    DoubleSpinBox* chassisXSizeSpin;
    DoubleSpinBox* chassisYSizeSpin;
    DoubleSpinBox* chassisZSizeSpin;
    PushButton* chassisColorButton;

    DoubleSpinBox* trackMassSpin;
    DoubleSpinBox* trackRadiusSpin;
    DoubleSpinBox* trackWidthSpin;
    DoubleSpinBox* trackWheelBaseSpin;
    PushButton* trackColorButton;

    DoubleSpinBox* frontSubTrackMassSpin;
    DoubleSpinBox* frontSubTrackForwardRadiusSpin;
    DoubleSpinBox* frontSubTrackBackwardRadiusSpin;
    DoubleSpinBox* frontSubTrackWidthSpin;
    DoubleSpinBox* frontSubTrackWheelBaseSpin;
    CheckBox* frontSubTrackCheck;
    PushButton* frontSubTrackColorButton;

    DoubleSpinBox* rearSubTrackMassSpin;
    DoubleSpinBox* rearSubTrackForwardRadiusSpin;
    DoubleSpinBox* rearSubTrackBackwardRadiusSpin;
    DoubleSpinBox* rearSubTrackWidthSpin;
    DoubleSpinBox* rearSubTrackWheelBaseSpin;
    CheckBox* rearSubTrackCheck;
    PushButton* rearSubTrackColorButton;

    DoubleSpinBox* spacerMassSpin;
    DoubleSpinBox* spacerRadiusSpin;
    DoubleSpinBox* spacerWidthSpin;
    PushButton* spacerColorButton;

    SpinBox* trackBeltNumberOfNodesSpin;
    DoubleSpinBox* trackBeltNodeThicknessSpin;
    DoubleSpinBox* trackBeltNodeWidthSpin;
    DoubleSpinBox* trackBeltNodeThickerThicknessSpin;
    SpinBox* trackBeltUseThickerNodeEverySpin;
    DoubleSpinBox* trackBeltNodeDistanceTensionMantissaSpin;
    SpinBox* trackBeltNodeDistanceTensionExponentSpin;
    DoubleSpinBox* trackBeltStabilizingHingeFrictionParameterMantissaSpin;
    SpinBox* trackBeltStabilizingHingeFrictionParameterExponentSpin;
    SpinBox* trackBeltMinStabilizingHingeNormalForceSpin;
    DoubleSpinBox* trackBeltHingeComplianceMantissaSpin;
    SpinBox* trackBeltHingeComplianceExponentSpin;
    DoubleSpinBox* trackBeltHingeSpookDampingSpin;
    DoubleSpinBox* trackBeltNodesToWheelsMergeThresholdSpin;
    DoubleSpinBox* trackBeltNodesToWheelsSplitThresholdSpin;

    SpinBox* subTrackBeltNumberOfNodesSpin;
    DoubleSpinBox* subTrackBeltNodeThicknessSpin;
    DoubleSpinBox* subTrackBeltNodeWidthSpin;
    DoubleSpinBox* subTrackBeltNodeThickerThicknessSpin;
    SpinBox* subTrackBeltUseThickerNodeEverySpin;
    DoubleSpinBox* subTrackBeltNodeDistanceTensionMantissaSpin;
    SpinBox* subTrackBeltNodeDistanceTensionExponentSpin;
    DoubleSpinBox* subTrackBeltStabilizingHingeFrictionParameterMantissaSpin;
    SpinBox* subTrackBeltStabilizingHingeFrictionParameterExponentSpin;
    SpinBox* subTrackBeltMinStabilizingHingeNormalForceSpin;
    DoubleSpinBox* subTrackBeltHingeComplianceMantissaSpin;
    SpinBox* subTrackBeltHingeComplianceExponentSpin;
    DoubleSpinBox* subTrackBeltHingeSpookDampingSpin;
    DoubleSpinBox* subTrackBeltNodesToWheelsMergeThresholdSpin;
    DoubleSpinBox* subTrackBeltNodesToWheelsSplitThresholdSpin;

    CheckBox* trackBeltCheck;

    void onAccepted();
    void onRejected();
    void initialize();
    void onNewBodyButtonClicked();
    void onExportYamlButtonClicked();
    void onImportYamlButtonClicked();
    void onExportBodyButtonClicked();
    void onEnableAgxCheckToggled(bool on);
    void onExportBody(QString fileName);
    void onExportAGXBody(QString fileName);
    void onColorChanged(PushButton* pushbutton);
    void setColor(PushButton* pushbutton, Vector3 color);
    Vector3 getColor(PushButton* colorButton);
};

}


CrawlerRobotBuilderDialog::CrawlerRobotBuilderDialog()
{
    impl = new CrawlerRobotBuilderDialogImpl(this);
}


CrawlerRobotBuilderDialogImpl::CrawlerRobotBuilderDialogImpl(CrawlerRobotBuilderDialog* self)
    : self(self)
{
    self->setWindowTitle(_("CrawlerRobotBuilder"));
    //menu layout
    QHBoxLayout* menuHbox = new QHBoxLayout();
    QVBoxLayout* menuVbox = new QVBoxLayout();
    PushButton* newBodyButton = new PushButton(_("New Body"));
    PushButton* exportBodyButton = new PushButton(_("Export Body"));
    PushButton* importYamlButton = new PushButton(_("Import YAML"));
    PushButton* exportYamlButton = new PushButton(_("Export YAML"));
    menuHbox->addWidget(newBodyButton);
    menuHbox->addWidget(exportBodyButton);
    menuHbox->addWidget(new VSeparator());
    menuHbox->addWidget(importYamlButton);
    menuHbox->addWidget(exportYamlButton);
    menuVbox->addLayout(menuHbox);

    chassisMassSpin = new DoubleSpinBox();
    chassisXSizeSpin = new DoubleSpinBox();
    chassisYSizeSpin = new DoubleSpinBox();
    chassisZSizeSpin = new DoubleSpinBox();
    trackMassSpin = new DoubleSpinBox();
    trackRadiusSpin = new DoubleSpinBox();
    trackWidthSpin = new DoubleSpinBox();
    trackWheelBaseSpin = new DoubleSpinBox();
    frontSubTrackMassSpin = new DoubleSpinBox();
    frontSubTrackForwardRadiusSpin = new DoubleSpinBox();
    frontSubTrackBackwardRadiusSpin = new DoubleSpinBox();
    frontSubTrackWidthSpin = new DoubleSpinBox();
    frontSubTrackWheelBaseSpin = new DoubleSpinBox();
    rearSubTrackMassSpin = new DoubleSpinBox();
    rearSubTrackForwardRadiusSpin = new DoubleSpinBox();
    rearSubTrackBackwardRadiusSpin = new DoubleSpinBox();
    rearSubTrackWidthSpin = new DoubleSpinBox();
    rearSubTrackWheelBaseSpin = new DoubleSpinBox();
    spacerMassSpin = new DoubleSpinBox();
    spacerRadiusSpin = new DoubleSpinBox();
    spacerWidthSpin = new DoubleSpinBox();
    trackBeltNumberOfNodesSpin = new SpinBox();
    trackBeltNodeThicknessSpin = new DoubleSpinBox();
    trackBeltNodeWidthSpin = new DoubleSpinBox();
    trackBeltNodeThickerThicknessSpin = new DoubleSpinBox();
    trackBeltUseThickerNodeEverySpin = new SpinBox();
    trackBeltNodeDistanceTensionMantissaSpin = new DoubleSpinBox();
    trackBeltNodeDistanceTensionExponentSpin = new SpinBox();
    trackBeltStabilizingHingeFrictionParameterMantissaSpin = new DoubleSpinBox();
    trackBeltStabilizingHingeFrictionParameterExponentSpin = new SpinBox();
    trackBeltMinStabilizingHingeNormalForceSpin = new SpinBox();
    trackBeltHingeComplianceMantissaSpin = new DoubleSpinBox();
    trackBeltHingeComplianceExponentSpin = new SpinBox();
    trackBeltHingeSpookDampingSpin = new DoubleSpinBox();
    trackBeltNodesToWheelsMergeThresholdSpin = new DoubleSpinBox();
    trackBeltNodesToWheelsSplitThresholdSpin = new DoubleSpinBox();
    subTrackBeltNumberOfNodesSpin = new SpinBox();
    subTrackBeltNodeThicknessSpin = new DoubleSpinBox();
    subTrackBeltNodeWidthSpin = new DoubleSpinBox();
    subTrackBeltNodeThickerThicknessSpin = new DoubleSpinBox();
    subTrackBeltUseThickerNodeEverySpin = new SpinBox();
    subTrackBeltNodeDistanceTensionMantissaSpin= new DoubleSpinBox();
    subTrackBeltNodeDistanceTensionExponentSpin = new SpinBox();
    subTrackBeltStabilizingHingeFrictionParameterMantissaSpin = new DoubleSpinBox();
    subTrackBeltStabilizingHingeFrictionParameterExponentSpin = new SpinBox();
    subTrackBeltMinStabilizingHingeNormalForceSpin = new SpinBox();
    subTrackBeltHingeComplianceMantissaSpin = new DoubleSpinBox();
    subTrackBeltHingeComplianceExponentSpin = new SpinBox();
    subTrackBeltHingeSpookDampingSpin = new DoubleSpinBox();
    subTrackBeltNodesToWheelsMergeThresholdSpin = new DoubleSpinBox();
    subTrackBeltNodesToWheelsSplitThresholdSpin = new DoubleSpinBox();

    chassisMassSpin->setRange(0.0, 9999.99);
    chassisXSizeSpin->setRange(0.0, 9999.99);
    chassisYSizeSpin->setRange(0.0, 9999.99);
    chassisZSizeSpin->setRange(0.0, 9999.99);
    trackMassSpin->setRange(0.0, 9999.99);
    trackRadiusSpin->setRange(0.0, 9999.99);
    trackWidthSpin->setRange(0.0, 9999.99);
    trackWheelBaseSpin->setRange(0.0, 9999.99);
    frontSubTrackMassSpin->setRange(0.0, 9999.99);
    frontSubTrackForwardRadiusSpin->setRange(0.0, 9999.99);
    frontSubTrackBackwardRadiusSpin->setRange(0.0, 9999.99);
    frontSubTrackWidthSpin->setRange(0.0, 9999.99);
    frontSubTrackWheelBaseSpin->setRange(0.0, 9999.99);
    rearSubTrackMassSpin->setRange(0.0, 9999.99);
    rearSubTrackForwardRadiusSpin->setRange(0.0, 9999.99);
    rearSubTrackBackwardRadiusSpin->setRange(0.0, 9999.99);
    rearSubTrackWidthSpin->setRange(0.0, 9999.99);
    rearSubTrackWheelBaseSpin->setRange(0.0, 9999.99);
    spacerMassSpin->setRange(0.0, 9999.99);
    spacerRadiusSpin->setRange(0.0, 9999.99);
    spacerWidthSpin->setRange(0.0, 9999.99);

    trackBeltNodeDistanceTensionMantissaSpin->setDecimals(3);
    trackBeltStabilizingHingeFrictionParameterMantissaSpin->setDecimals(3);
    trackBeltHingeComplianceMantissaSpin->setDecimals(3);
    trackBeltHingeSpookDampingSpin->setDecimals(3);
    trackBeltNodesToWheelsMergeThresholdSpin->setDecimals(3);
    trackBeltNodesToWheelsSplitThresholdSpin->setDecimals(3);

    trackBeltNumberOfNodesSpin->setRange(0, 9999);
    trackBeltNodeThicknessSpin->setRange(0.0, 9999.999);
    trackBeltNodeWidthSpin->setRange(0.0, 9999.99999);
    trackBeltNodeThickerThicknessSpin->setRange(0.0, 9999.999);
    trackBeltUseThickerNodeEverySpin->setRange(0, 9999);
    trackBeltMinStabilizingHingeNormalForceSpin->setRange(0, 9999);
    trackBeltHingeSpookDampingSpin->setRange(0.0, 9999.999);
    trackBeltNodesToWheelsMergeThresholdSpin->setRange(-9999.999, 9999.999);
    trackBeltNodesToWheelsSplitThresholdSpin->setRange(-9999.999, 9999.999);

    subTrackBeltNodeDistanceTensionMantissaSpin->setDecimals(3);
    subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->setDecimals(3);
    subTrackBeltHingeComplianceMantissaSpin->setDecimals(3);
    subTrackBeltHingeSpookDampingSpin->setDecimals(3);
    subTrackBeltNodesToWheelsMergeThresholdSpin->setDecimals(3);
    subTrackBeltNodesToWheelsSplitThresholdSpin->setDecimals(3);

    subTrackBeltNumberOfNodesSpin->setRange(0, 9999);
    subTrackBeltNodeThicknessSpin->setRange(0.0, 9999.999);
    subTrackBeltNodeWidthSpin->setRange(0.0, 9999.999);
    subTrackBeltNodeThickerThicknessSpin->setRange(0.0, 9999.999);
    subTrackBeltUseThickerNodeEverySpin->setRange(0, 9999);
    subTrackBeltMinStabilizingHingeNormalForceSpin->setRange(0, 9999);
    subTrackBeltHingeSpookDampingSpin->setRange(0.0, 9999.999);
    subTrackBeltNodesToWheelsMergeThresholdSpin->setRange(-9999.999, 9999.999);
    subTrackBeltNodesToWheelsSplitThresholdSpin->setRange(-9999.999, 9999.999);

    //chassis
    QVBoxLayout* chassisVbox = new QVBoxLayout();
    QHBoxLayout* cmassHbox = new QHBoxLayout();
    chassisVbox->addLayout(new HSeparatorBox(new QLabel(_("Chassis"))));
    cmassHbox->addWidget(new QLabel(_("mass [kg]")));
    cmassHbox->addWidget(chassisMassSpin);
    cmassHbox->addWidget(new QLabel(_("color")));
    chassisColorButton = new PushButton();
    chassisColorButton->sigClicked().connect([&](){ onColorChanged(chassisColorButton); });
    cmassHbox->addWidget(chassisColorButton);
    chassisVbox->addLayout(cmassHbox);
    QHBoxLayout* csizeHbox = new QHBoxLayout();
    csizeHbox->addWidget(new QLabel(_("size(x-y-z) [m, m, m]")));
    csizeHbox->addWidget(chassisXSizeSpin);
    csizeHbox->addWidget(chassisYSizeSpin);
    csizeHbox->addWidget(chassisZSizeSpin);
    chassisVbox->addLayout(csizeHbox);

    //track
    QVBoxLayout* trackVbox = new QVBoxLayout();
    QHBoxLayout* tmassHbox = new QHBoxLayout();
    trackVbox->addLayout(new HSeparatorBox(new QLabel(_("Track"))));
    tmassHbox->addWidget(new QLabel(_("mass [kg]")));
    tmassHbox->addWidget(trackMassSpin);
    tmassHbox->addWidget(new QLabel(_("color")));
    trackColorButton = new PushButton();
    trackColorButton->sigClicked().connect([&](){ onColorChanged(trackColorButton); });
    tmassHbox->addWidget(trackColorButton);
    trackVbox->addLayout(tmassHbox);
    QHBoxLayout* tradiusHbox = new QHBoxLayout();
    tradiusHbox->addWidget(new QLabel(_("radius [m]")));
    tradiusHbox->addWidget(trackRadiusSpin);
    trackVbox->addLayout(tradiusHbox);
    QHBoxLayout* twidthHbox = new QHBoxLayout();
    twidthHbox->addWidget(new QLabel(_("width [m]")));
    twidthHbox->addWidget(trackWidthSpin);
    twidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    twidthHbox->addWidget(trackWheelBaseSpin);
    trackVbox->addLayout(twidthHbox);

    //front subtrack
    QVBoxLayout* frontSubTrackVbox = new QVBoxLayout();
    QWidget* frontSubTrackWidget = new QWidget();
    frontSubTrackCheck = new CheckBox();
    QHBoxLayout* frontSubTrackTitleHbox = new QHBoxLayout();
    frontSubTrackTitleHbox->addWidget(frontSubTrackCheck);
    frontSubTrackTitleHbox->addWidget(new QLabel(_("Front SubTrack")));
    frontSubTrackWidget->setLayout(frontSubTrackTitleHbox);
    frontSubTrackVbox->addLayout(new HSeparatorBox(frontSubTrackWidget));
    QHBoxLayout* frontSubTrackmassHbox = new QHBoxLayout();
    frontSubTrackmassHbox->addWidget(new QLabel(_("mass [kg]")));
    frontSubTrackmassHbox->addWidget(frontSubTrackMassSpin);
    frontSubTrackmassHbox->addWidget(new QLabel(_("color")));
    frontSubTrackColorButton = new PushButton();
    frontSubTrackColorButton->sigClicked().connect([&](){ onColorChanged(frontSubTrackColorButton); });
    frontSubTrackmassHbox->addWidget(frontSubTrackColorButton);
    frontSubTrackVbox->addLayout(frontSubTrackmassHbox);
    QHBoxLayout* frontSubTrackradiusHbox = new QHBoxLayout();
    frontSubTrackradiusHbox->addWidget(new QLabel(_("radius(forward-backward) [m]")));
    frontSubTrackradiusHbox->addWidget(frontSubTrackForwardRadiusSpin);
    frontSubTrackradiusHbox->addWidget(frontSubTrackBackwardRadiusSpin);
    frontSubTrackVbox->addLayout(frontSubTrackradiusHbox);
    QHBoxLayout* frontSubTrackwidthHbox = new QHBoxLayout();
    frontSubTrackwidthHbox->addWidget(new QLabel(_("width [m]")));
    frontSubTrackwidthHbox->addWidget(frontSubTrackWidthSpin);
    frontSubTrackwidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    frontSubTrackwidthHbox->addWidget(frontSubTrackWheelBaseSpin);
    frontSubTrackVbox->addLayout(frontSubTrackwidthHbox);

    //rear subtrack
    QVBoxLayout* rearSubTrackVbox = new QVBoxLayout();
    QHBoxLayout* rearSubTrackmassHbox = new QHBoxLayout();
    QWidget* rearSubTrackWidget = new QWidget();
    rearSubTrackCheck = new CheckBox();
    QHBoxLayout* rearSubTrackHbox = new QHBoxLayout();
    rearSubTrackHbox->addWidget(rearSubTrackCheck);
    rearSubTrackHbox->addWidget(new QLabel(_("Rear SubTrack")));
    rearSubTrackWidget->setLayout(rearSubTrackHbox);
    rearSubTrackVbox->addLayout(new HSeparatorBox(rearSubTrackWidget));
    rearSubTrackmassHbox->addWidget(new QLabel(_("mass [kg]")));
    rearSubTrackmassHbox->addWidget(rearSubTrackMassSpin);
    rearSubTrackmassHbox->addWidget(new QLabel(_("color")));
    rearSubTrackColorButton = new PushButton();
    rearSubTrackColorButton->sigClicked().connect([&](){ onColorChanged(rearSubTrackColorButton); });
    rearSubTrackmassHbox->addWidget(rearSubTrackColorButton);
    rearSubTrackVbox->addLayout(rearSubTrackmassHbox);
    QHBoxLayout* rearSubTrackradiusHbox = new QHBoxLayout();
    rearSubTrackradiusHbox->addWidget(new QLabel(_("radius(forward-backward) [m]")));
    rearSubTrackradiusHbox->addWidget(rearSubTrackForwardRadiusSpin);
    rearSubTrackradiusHbox->addWidget(rearSubTrackBackwardRadiusSpin);
    rearSubTrackVbox->addLayout(rearSubTrackradiusHbox);
    QHBoxLayout* rearSubTrackwidthHbox = new QHBoxLayout();
    rearSubTrackwidthHbox->addWidget(new QLabel(_("width [m]")));
    rearSubTrackwidthHbox->addWidget(rearSubTrackWidthSpin);
    rearSubTrackwidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    rearSubTrackwidthHbox->addWidget(rearSubTrackWheelBaseSpin);
    rearSubTrackVbox->addLayout(rearSubTrackwidthHbox);

    //spacer
    QVBoxLayout* spacerVbox = new QVBoxLayout();
    QHBoxLayout* smassHbox = new QHBoxLayout();
    spacerVbox->addLayout(new HSeparatorBox(new QLabel(_("Spacer"))));
    smassHbox->addWidget(new QLabel(_("mass [kg]")));
    smassHbox->addWidget(spacerMassSpin);
    smassHbox->addWidget(new QLabel(_("color")));
    spacerColorButton = new PushButton();
    spacerColorButton->sigClicked().connect([&](){ onColorChanged(spacerColorButton); });
    smassHbox->addWidget(spacerColorButton);
    spacerVbox->addLayout(smassHbox);
    QHBoxLayout* sradiusHbox = new QHBoxLayout();
    sradiusHbox->addWidget(new QLabel(_("radius [m]")));
    sradiusHbox->addWidget(spacerRadiusSpin);
    spacerVbox->addLayout(sradiusHbox);
    QHBoxLayout* swidthHbox = new QHBoxLayout();
    swidthHbox->addWidget(new QLabel(_("width [m]")));
    swidthHbox->addWidget(spacerWidthSpin);
    spacerVbox->addLayout(swidthHbox);

    //track belt
    QVBoxLayout* trackBeltVbox = new QVBoxLayout();
    trackBeltVbox->addLayout(new HSeparatorBox(new QLabel(_("Track Belt"))));
    QHBoxLayout* option0Hbox = new QHBoxLayout();
    option0Hbox->addWidget(new QLabel(_("number of nodes [-]")));
    option0Hbox->addWidget(trackBeltNumberOfNodesSpin);
    option0Hbox->addWidget(new QLabel(_("node thickness [m]")));
    option0Hbox->addWidget(trackBeltNodeThicknessSpin);
    QHBoxLayout* option1Hbox = new QHBoxLayout();
    option1Hbox->addWidget(new QLabel(_("node width [m]")));
    option1Hbox->addWidget(trackBeltNodeWidthSpin);
    option1Hbox->addWidget(new QLabel(_("node thickerthickness [m]")));
    option1Hbox->addWidget(trackBeltNodeThickerThicknessSpin);
    QHBoxLayout* option2Hbox = new QHBoxLayout();
    option2Hbox->addWidget(new QLabel(_("use thicker node every [-]")));
    option2Hbox->addWidget(trackBeltUseThickerNodeEverySpin);
    option2Hbox->addStretch();
    option2Hbox->addWidget(new QLabel(_("node distance tension [m]")));
    option2Hbox->addWidget(trackBeltNodeDistanceTensionMantissaSpin);
    option2Hbox->addWidget(new QLabel(_("e-")));
    option2Hbox->addWidget(trackBeltNodeDistanceTensionExponentSpin);
    QHBoxLayout* option3Hbox = new QHBoxLayout();
    option3Hbox->addWidget(new QLabel(_("stabilizing hinge friction parameter [-]")));
    option3Hbox->addWidget(trackBeltStabilizingHingeFrictionParameterMantissaSpin);
    option3Hbox->addWidget(new QLabel(_("e-")));
    option3Hbox->addWidget(trackBeltStabilizingHingeFrictionParameterExponentSpin);
    option3Hbox->addStretch();
    option3Hbox->addWidget(new QLabel(_("min stabilizing hinge normal force [N]")));
    option3Hbox->addWidget(trackBeltMinStabilizingHingeNormalForceSpin);
    QHBoxLayout* option4Hbox = new QHBoxLayout();
    option4Hbox->addWidget(new QLabel(_("hinge compliance [rad/Nm]")));
    option4Hbox->addWidget(trackBeltHingeComplianceMantissaSpin);
    option4Hbox->addWidget(new QLabel(_("e-")));
    option4Hbox->addWidget(trackBeltHingeComplianceExponentSpin);
    option4Hbox->addStretch();
    option4Hbox->addWidget(new QLabel(_("hinge spook damping [s]")));
    option4Hbox->addWidget(trackBeltHingeSpookDampingSpin);
    QHBoxLayout* option5Hbox = new QHBoxLayout();
    option5Hbox->addWidget(new QLabel(_("nodes to wheels merge threshold [-]")));
    option5Hbox->addWidget(trackBeltNodesToWheelsMergeThresholdSpin);
    option5Hbox->addWidget(new QLabel(_("nodes to wheels split threshold [-]")));
    option5Hbox->addWidget(trackBeltNodesToWheelsSplitThresholdSpin);

    trackBeltVbox->addLayout(option0Hbox);
    trackBeltVbox->addLayout(option1Hbox);
    trackBeltVbox->addLayout(option2Hbox);
    trackBeltVbox->addLayout(option3Hbox);
    trackBeltVbox->addLayout(option4Hbox);
    trackBeltVbox->addLayout(option5Hbox);

    //subtrack belt
    QVBoxLayout* subTrackBeltVbox = new QVBoxLayout();
    subTrackBeltVbox->addLayout(new HSeparatorBox(new QLabel(_("SubTrack Belt"))));
    QHBoxLayout* option6Hbox = new QHBoxLayout();
    option6Hbox->addWidget(new QLabel(_("number of nodes [-]")));
    option6Hbox->addWidget(subTrackBeltNumberOfNodesSpin);
    option6Hbox->addWidget(new QLabel(_("node thickness [m]")));
    option6Hbox->addWidget(subTrackBeltNodeThicknessSpin);
    QHBoxLayout* option7Hbox = new QHBoxLayout();
    option7Hbox->addWidget(new QLabel(_("node width [m]")));
    option7Hbox->addWidget(subTrackBeltNodeWidthSpin);
    option7Hbox->addWidget(new QLabel(_("node thickerthickness [m]")));
    option7Hbox->addWidget(subTrackBeltNodeThickerThicknessSpin);
    QHBoxLayout* option8Hbox = new QHBoxLayout();
    option8Hbox->addWidget(new QLabel(_("use thicker node every [-]")));
    option8Hbox->addWidget(subTrackBeltUseThickerNodeEverySpin);
    option8Hbox->addStretch();
    option8Hbox->addWidget(new QLabel(_("node distance tension [m]")));
    option8Hbox->addWidget(subTrackBeltNodeDistanceTensionMantissaSpin);
    option8Hbox->addWidget(new QLabel(_("e-")));
    option8Hbox->addWidget(subTrackBeltNodeDistanceTensionExponentSpin);
    QHBoxLayout* option9Hbox = new QHBoxLayout();
    option9Hbox->addWidget(new QLabel(_("stabilizing hinge friction parameter [-]")));
    option9Hbox->addWidget(subTrackBeltStabilizingHingeFrictionParameterMantissaSpin);
    option9Hbox->addWidget(new QLabel(_("e-")));
    option9Hbox->addWidget(subTrackBeltStabilizingHingeFrictionParameterExponentSpin);
    option9Hbox->addWidget(new QLabel(_("min stabilizing hinge normal force [N]")));
    option9Hbox->addWidget(subTrackBeltMinStabilizingHingeNormalForceSpin);
    QHBoxLayout* option10Hbox = new QHBoxLayout();
    option10Hbox->addWidget(new QLabel(_("hinge compliance [rad/Nm]")));
    option10Hbox->addWidget(subTrackBeltHingeComplianceMantissaSpin);
    option10Hbox->addWidget(new QLabel(_("e-")));
    option10Hbox->addWidget(subTrackBeltHingeComplianceExponentSpin);
    option10Hbox->addStretch();
    option10Hbox->addWidget(new QLabel(_("hinge spook damping [s]")));
    option10Hbox->addWidget(subTrackBeltHingeSpookDampingSpin);
    QHBoxLayout* option11Hbox = new QHBoxLayout();
    option11Hbox->addWidget(new QLabel(_("nodes to wheels merge threshold [-]")));
    option11Hbox->addWidget(subTrackBeltNodesToWheelsMergeThresholdSpin);
    option11Hbox->addWidget(new QLabel(_("nodes to wheels split threshold [-]")));
    option11Hbox->addWidget(subTrackBeltNodesToWheelsSplitThresholdSpin);

    subTrackBeltVbox->addLayout(option6Hbox);
    subTrackBeltVbox->addLayout(option7Hbox);
    subTrackBeltVbox->addLayout(option8Hbox);
    subTrackBeltVbox->addLayout(option9Hbox);
    subTrackBeltVbox->addLayout(option10Hbox);
    subTrackBeltVbox->addLayout(option11Hbox);

    QHBoxLayout* trackBeltHbox = new QHBoxLayout();
    trackBeltCheck = new CheckBox();
    trackBeltHbox->addWidget(trackBeltCheck);
    trackBeltHbox->addWidget(new QLabel(_("AGX")));
    trackBeltHbox->addStretch();

    //main layout
    QVBoxLayout* mainVbox = new QVBoxLayout();
    QHBoxLayout* mainHbox = new QHBoxLayout();
    QVBoxLayout* mainLeftVbox = new QVBoxLayout();
    QVBoxLayout* mainRightVbox = new QVBoxLayout();
    mainLeftVbox->addLayout(chassisVbox);
    mainLeftVbox->addLayout(trackVbox);
    mainLeftVbox->addLayout(frontSubTrackVbox);
    mainLeftVbox->addLayout(rearSubTrackVbox);
    mainLeftVbox->addLayout(spacerVbox);
    mainLeftVbox->addStretch();
    mainRightVbox->addLayout(trackBeltVbox);
    mainRightVbox->addLayout(subTrackBeltVbox);
    mainRightVbox->addStretch();
    mainHbox->addLayout(mainLeftVbox);
    mainHbox->addSpacing(5);
//    mainHbox->addLayout(mainRightVbox);
    mainVbox->addLayout(menuVbox);
    mainVbox->addLayout(trackBeltHbox);
    mainVbox->addLayout(mainHbox);

    initialize();
    newBodyButton->sigClicked().connect([&](){ onNewBodyButtonClicked(); });
    exportBodyButton->sigClicked().connect([&](){ onExportBodyButtonClicked(); });
    importYamlButton->sigClicked().connect([&](){ onImportYamlButtonClicked(); });
    exportYamlButton->sigClicked().connect([&](){ onExportYamlButtonClicked(); });
    trackBeltCheck->sigToggled().connect([&](bool on){ onEnableAgxCheckToggled(on); });
    self->setLayout(mainVbox);
}


CrawlerRobotBuilderDialog::~CrawlerRobotBuilderDialog()
{
    delete impl;
}


void CrawlerRobotBuilderDialog::initializeClass(ExtensionManager* ext)
{
    if(!builderDialog) {
        builderDialog = ext->manage(new CrawlerRobotBuilderDialog());
    }

    MenuManager& menuManager = ext->menuManager();
    menuManager.setPath("/Tools");
    menuManager.addItem(_("CrawlerRobotBuilder"))
            ->sigTriggered().connect([](){ builderDialog->show(); });
}


void CrawlerRobotBuilderDialog::onAccepted()
{
    impl->onAccepted();
}


void CrawlerRobotBuilderDialogImpl::onAccepted()
{

}


void CrawlerRobotBuilderDialog::onRejected()
{
    impl->onRejected();
}


void CrawlerRobotBuilderDialogImpl::onRejected()
{

}


void CrawlerRobotBuilderDialogImpl::initialize()
{
    frontSubTrackCheck->setChecked(Qt::Checked);
    rearSubTrackCheck->setChecked(Qt::Checked);
    trackBeltCheck->setChecked(Qt::Unchecked);

    chassisMassSpin->setValue(8.0);
    chassisXSizeSpin->setValue(0.45);
    chassisYSizeSpin->setValue(0.3);
    chassisZSizeSpin->setValue(0.1);
    trackMassSpin->setValue(1.0);
    trackRadiusSpin->setValue(0.08);
    trackWidthSpin->setValue(0.1);
    trackWheelBaseSpin->setValue(0.42);
    frontSubTrackMassSpin->setValue(0.25);
    frontSubTrackForwardRadiusSpin->setValue(0.08);
    frontSubTrackBackwardRadiusSpin->setValue(0.08);
    frontSubTrackWidthSpin->setValue(0.08);
    frontSubTrackWheelBaseSpin->setValue(0.13);
    rearSubTrackMassSpin->setValue(0.25);
    rearSubTrackForwardRadiusSpin->setValue(0.08);
    rearSubTrackBackwardRadiusSpin->setValue(0.08);
    rearSubTrackWheelBaseSpin->setValue(0.13);
    rearSubTrackWidthSpin->setValue(0.08);
    spacerMassSpin->setValue(0.2);
    spacerRadiusSpin->setValue(0.06);
    spacerWidthSpin->setValue(0.0125);
    trackBeltNumberOfNodesSpin->setValue(42);
    trackBeltNodeThicknessSpin->setValue(0.01);
    trackBeltNodeWidthSpin->setValue(0.09);
    trackBeltNodeThickerThicknessSpin->setValue(0.02);
    trackBeltUseThickerNodeEverySpin->setValue(3);
    trackBeltNodeDistanceTensionMantissaSpin->setValue(2.0);
    trackBeltNodeDistanceTensionExponentSpin->setValue(4);
    trackBeltStabilizingHingeFrictionParameterMantissaSpin->setValue(1.0);
    trackBeltStabilizingHingeFrictionParameterExponentSpin->setValue(6);
    trackBeltMinStabilizingHingeNormalForceSpin->setValue(100);
    trackBeltHingeComplianceMantissaSpin->setValue(9.0);
    trackBeltHingeComplianceExponentSpin->setValue(10);
    trackBeltHingeSpookDampingSpin->setValue(0.01);
    trackBeltNodesToWheelsMergeThresholdSpin->setValue(-0.01);
    trackBeltNodesToWheelsSplitThresholdSpin->setValue(-0.009);

    subTrackBeltNumberOfNodesSpin->setValue(42);
    subTrackBeltNodeThicknessSpin->setValue(0.01);
    subTrackBeltNodeWidthSpin->setValue(0.09);
    subTrackBeltNodeThickerThicknessSpin->setValue(0.02);
    subTrackBeltUseThickerNodeEverySpin->setValue(3);
    subTrackBeltNodeDistanceTensionMantissaSpin->setValue(2.0);
    subTrackBeltNodeDistanceTensionExponentSpin->setValue(4);
    subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->setValue(1.0);
    subTrackBeltStabilizingHingeFrictionParameterExponentSpin->setValue(6);
    subTrackBeltMinStabilizingHingeNormalForceSpin->setValue(100);
    subTrackBeltHingeComplianceMantissaSpin->setValue(9.0);
    subTrackBeltHingeComplianceExponentSpin->setValue(10);
    subTrackBeltHingeSpookDampingSpin->setValue(0.01);
    subTrackBeltNodesToWheelsMergeThresholdSpin->setValue(-0.01);
    subTrackBeltNodesToWheelsSplitThresholdSpin->setValue(-0.009);

    trackBeltNumberOfNodesSpin->setEnabled(false);
    trackBeltNodeThicknessSpin->setEnabled(false);
    trackBeltNodeWidthSpin->setEnabled(false);
    trackBeltNodeThickerThicknessSpin->setEnabled(false);
    trackBeltUseThickerNodeEverySpin->setEnabled(false);
    trackBeltNodeDistanceTensionMantissaSpin->setEnabled(false);
    trackBeltNodeDistanceTensionExponentSpin->setEnabled(false);
    trackBeltStabilizingHingeFrictionParameterMantissaSpin->setEnabled(false);
    trackBeltStabilizingHingeFrictionParameterExponentSpin->setEnabled(false);
    trackBeltMinStabilizingHingeNormalForceSpin->setEnabled(false);
    trackBeltHingeComplianceMantissaSpin->setEnabled(false);
    trackBeltHingeComplianceExponentSpin->setEnabled(false);
    trackBeltHingeSpookDampingSpin->setEnabled(false);
    trackBeltNodesToWheelsMergeThresholdSpin->setEnabled(false);
    trackBeltNodesToWheelsSplitThresholdSpin->setEnabled(false);

    subTrackBeltNumberOfNodesSpin->setEnabled(false);
    subTrackBeltNodeThicknessSpin->setEnabled(false);
    subTrackBeltNodeWidthSpin->setEnabled(false);
    subTrackBeltNodeThickerThicknessSpin->setEnabled(false);
    subTrackBeltUseThickerNodeEverySpin->setEnabled(false);
    subTrackBeltNodeDistanceTensionMantissaSpin->setEnabled(false);
    subTrackBeltNodeDistanceTensionExponentSpin->setEnabled(false);
    subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->setEnabled(false);
    subTrackBeltStabilizingHingeFrictionParameterExponentSpin->setEnabled(false);
    subTrackBeltMinStabilizingHingeNormalForceSpin->setEnabled(false);
    subTrackBeltHingeComplianceMantissaSpin->setEnabled(false);
    subTrackBeltHingeComplianceExponentSpin->setEnabled(false);
    subTrackBeltHingeSpookDampingSpin->setEnabled(false);
    subTrackBeltNodesToWheelsMergeThresholdSpin->setEnabled(false);
    subTrackBeltNodesToWheelsSplitThresholdSpin->setEnabled(false);

    setColor(chassisColorButton, Vector3(0.0 / 255.0, 153.0 / 255.0, 0.0 / 255.0));
    setColor(trackColorButton, Vector3(51.0 / 255.0, 51.0 / 255.0, 51.0 / 255.0));
    setColor(frontSubTrackColorButton, Vector3(51.0 / 255.0, 51.0 / 255.0, 51.0 / 255.0));
    setColor(rearSubTrackColorButton, Vector3(51.0 / 255.0, 51.0 / 255.0, 51.0 / 255.0));
    setColor(spacerColorButton, Vector3(255.0 / 255.0, 0.0 / 255.0, 0.0 / 255.0));
}


void CrawlerRobotBuilderDialogImpl::onNewBodyButtonClicked()
{
    initialize();
}


void CrawlerRobotBuilderDialogImpl::onImportYamlButtonClicked()
{
    QString currentDirectory = QString::fromStdString(ProjectManager::instance()->currentProjectDirectory());
    QString yamlFileName = QFileDialog::getOpenFileName(nullptr, "Save Config YAML", currentDirectory,
            "Config YAML (*.yaml *.yml)");
    if(!yamlFileName.isEmpty()) {
        QFileInfo info(yamlFileName);
        if(info.suffix().isEmpty()) {
            yamlFileName += ".yaml";
        }

        YAMLReader reader;
        if(reader.load(yamlFileName.toStdString())) {
            Mapping* topNode = reader.loadDocument(yamlFileName.toStdString())->toMapping();
            Listing* configList = topNode->findListing("configs");
            if(configList->isValid()) {
                for(int i = 0; i < configList->size(); i++) {
                    Mapping& node = *configList->at(i)->toMapping();
                    string name;
                    if(node.read("name", name)) {}

                    double d;
                    bool on;
                    int v;
                    Vector3 diffuseColor;

                    if(name == "CHASSIS") {
                        if(node.read("mass", d)) chassisMassSpin->setValue(d);
                        if(node.read("xSize", d)) chassisXSizeSpin->setValue(d);
                        if(node.read("ySize", d)) chassisYSizeSpin->setValue(d);
                        if(node.read("zSize", d)) chassisZSizeSpin->setValue(d);
                        if(read(node, "diffuseColor", diffuseColor)) setColor(chassisColorButton, diffuseColor);
                    }
                    else if(name == "TRACK") {
                        if(node.read("mass", d)) trackMassSpin->setValue(d);
                        if(node.read("radius", d)) trackRadiusSpin->setValue(d);
                        if(node.read("width", d)) trackWidthSpin->setValue(d);
                        if(node.read("wheelBase", d)) trackWheelBaseSpin->setValue(d);
                        if(read(node, "diffuseColor", diffuseColor)) setColor(trackColorButton, diffuseColor);
                    }
                    else if(name == "FRONTSUBTRACK") {
                        if(node.read("mass", d)) frontSubTrackMassSpin->setValue(d);
                        if(node.read("forwardRadius", d)) frontSubTrackForwardRadiusSpin->setValue(d);
                        if(node.read("backwardRadius", d)) frontSubTrackBackwardRadiusSpin->setValue(d);
                        if(node.read("width", d)) frontSubTrackWidthSpin->setValue(d);
                        if(node.read("wheelBase", d)) frontSubTrackWheelBaseSpin->setValue(d);
                        if(read(node, "diffuseColor", diffuseColor)) setColor(frontSubTrackColorButton, diffuseColor);
                        if(node.read("on", on)) frontSubTrackCheck->setChecked(on);
                    }
                    else if(name == "REARSUBTRACK") {
                        if(node.read("mass", d)) rearSubTrackMassSpin->setValue(d);
                        if(node.read("forwardRadius", d)) rearSubTrackForwardRadiusSpin->setValue(d);
                        if(node.read("backwardRadius", d)) rearSubTrackBackwardRadiusSpin->setValue(d);
                        if(node.read("width", d)) rearSubTrackWidthSpin->setValue(d);
                        if(node.read("wheelBase", d)) rearSubTrackWheelBaseSpin->setValue(d);
                        if(read(node, "diffuseColor", diffuseColor)) setColor(rearSubTrackColorButton, diffuseColor);
                        if(node.read("on", on)) rearSubTrackCheck->setChecked(on);
                    }
                    else if(name == "SPACER") {
                        if(node.read("mass", d)) spacerMassSpin->setValue(d);
                        if(node.read("radius", d)) spacerRadiusSpin->setValue(d);
                        if(node.read("width", d)) spacerWidthSpin->setValue(d);
                        if(read(node, "diffuseColor", diffuseColor)) setColor(spacerColorButton, diffuseColor);
                    }
                    else if(name == "TRACKBELT") {
                        if(node.read("numberOfNodes", v)) trackBeltNumberOfNodesSpin->setValue(v);
                        if(node.read("nodeThickness", d)) trackBeltNodeThicknessSpin->setValue(d);
                        if(node.read("nodeWidth", d)) trackBeltNodeWidthSpin->setValue(d);
                        if(node.read("nodeThickerThickness", d)) trackBeltNodeThickerThicknessSpin->setValue(d);
                        if(node.read("useThickerNodeEvery", v)) trackBeltUseThickerNodeEverySpin->setValue(v);
                        if(node.read("nodeDistanceTensionMantissa", d)) trackBeltNodeDistanceTensionMantissaSpin->setValue(d);
                        if(node.read("nodeDistanceTensionExponent", v)) trackBeltNodeDistanceTensionExponentSpin->setValue(v);
                        if(node.read("stabilizingHingeFrictionParameterMantissa", d)) trackBeltStabilizingHingeFrictionParameterMantissaSpin->setValue(d);
                        if(node.read("stabilizingHingeFrictionParameterExponent", v)) trackBeltStabilizingHingeFrictionParameterExponentSpin->setValue(v);
                        if(node.read("minStabilizingHingeNormalForceSpin", v)) trackBeltMinStabilizingHingeNormalForceSpin->setValue(v);
                        if(node.read("hingeComplianceMantissa", d)) trackBeltHingeComplianceMantissaSpin->setValue(d);
                        if(node.read("hingeComplianceExponent", v)) trackBeltHingeComplianceExponentSpin->setValue(v);
                        if(node.read("hingeSpookDamping", d)) trackBeltHingeSpookDampingSpin->setValue(d);
                        if(node.read("nodesToWheelsMergeThreshold", d)) trackBeltNodesToWheelsMergeThresholdSpin->setValue(d);
                        if(node.read("nodesToWheelsSplitThreshold", d)) trackBeltNodesToWheelsSplitThresholdSpin->setValue(d);
                        if(node.read("on", on)) trackBeltCheck->setChecked(on);
                    }
                    else if(name == "SUBTRACKBELT") {
                        if(node.read("numberOfNodes", v)) subTrackBeltNumberOfNodesSpin->setValue(v);
                        if(node.read("nodeThickness", d)) subTrackBeltNodeThicknessSpin->setValue(d);
                        if(node.read("nodeWidth", d)) subTrackBeltNodeWidthSpin->setValue(d);
                        if(node.read("nodeThickerThickness", d)) subTrackBeltNodeThickerThicknessSpin->setValue(d);
                        if(node.read("useThickerNodeEvery", v)) subTrackBeltUseThickerNodeEverySpin->setValue(v);
                        if(node.read("nodeDistanceTensionMantissa", d)) subTrackBeltNodeDistanceTensionMantissaSpin->setValue(d);
                        if(node.read("nodeDistanceTensionExponent", v)) subTrackBeltNodeDistanceTensionExponentSpin->setValue(v);
                        if(node.read("stabilizingHingeFrictionParameterMantissa", d)) subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->setValue(d);
                        if(node.read("stabilizingHingeFrictionParameterExponent", v)) subTrackBeltStabilizingHingeFrictionParameterExponentSpin->setValue(v);
                        if(node.read("minStabilizingHingeNormalForceSpin", v)) subTrackBeltMinStabilizingHingeNormalForceSpin->setValue(v);
                        if(node.read("hingeComplianceMantissa", d)) subTrackBeltHingeComplianceMantissaSpin->setValue(d);
                        if(node.read("hingeComplianceExponent", v)) subTrackBeltHingeComplianceExponentSpin->setValue(v);
                        if(node.read("hingeSpookDamping", d)) subTrackBeltHingeSpookDampingSpin->setValue(d);
                        if(node.read("nodesToWheelsMergeThreshold", d)) subTrackBeltNodesToWheelsMergeThresholdSpin->setValue(d);
                        if(node.read("nodesToWheelsSplitThreshold", d)) subTrackBeltNodesToWheelsSplitThresholdSpin->setValue(d);
                    }
                }
            }
        }
    }
}


void CrawlerRobotBuilderDialogImpl::onExportYamlButtonClicked()
{
    QString currentDirectory = QString::fromStdString(ProjectManager::instance()->currentProjectDirectory());
    QString yamlFileName = QFileDialog::getSaveFileName(nullptr, "Save Config YAML", currentDirectory,
            "Config YAML (*.yaml *.yml)");
    if(!yamlFileName.isEmpty()) {
        QFileInfo info(yamlFileName);
        if(info.suffix().isEmpty()) {
            yamlFileName += ".yaml";
        }
        string bodyName = info.baseName().toStdString();

        YAMLWriter writer(yamlFileName.toStdString());

        writer.startMapping();
        writer.putKeyValue("format", "CrawlerRobotBuilderYaml");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("name", bodyName);
        writer.putKey("configs");
        writer.startListing();

        writer.startMapping();
        writer.putKeyValue("name", "CHASSIS");
        writer.putKeyValue("mass", chassisMassSpin->value());
        writer.putKeyValue("xSize", chassisXSizeSpin->value());
        writer.putKeyValue("ySize", chassisYSizeSpin->value());
        writer.putKeyValue("zSize", chassisZSizeSpin->value());
        putKeyVector3(&writer, "diffuseColor", getColor(chassisColorButton));
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "TRACK");
        writer.putKeyValue("mass", trackMassSpin->value());
        writer.putKeyValue("radius", trackRadiusSpin->value());
        writer.putKeyValue("width", trackWidthSpin->value());
        writer.putKeyValue("wheelBase", trackWheelBaseSpin->value());
        putKeyVector3(&writer, "diffuseColor", getColor(trackColorButton));
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "FRONTSUBTRACK");
        writer.putKeyValue("mass", frontSubTrackMassSpin->value());
        writer.putKeyValue("forwardRadius", frontSubTrackForwardRadiusSpin->value());
        writer.putKeyValue("backwardRadius", frontSubTrackBackwardRadiusSpin->value());
        writer.putKeyValue("width", frontSubTrackWidthSpin->value());
        writer.putKeyValue("wheelBase", frontSubTrackWheelBaseSpin->value());
        writer.putKeyValue("on", frontSubTrackCheck->isChecked());
        putKeyVector3(&writer, "diffuseColor", getColor(frontSubTrackColorButton));
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "REARSUBTRACK");
        writer.putKeyValue("mass", rearSubTrackMassSpin->value());
        writer.putKeyValue("forwardRadius", rearSubTrackForwardRadiusSpin->value());
        writer.putKeyValue("backwardRadius", rearSubTrackBackwardRadiusSpin->value());
        writer.putKeyValue("width", rearSubTrackWidthSpin->value());
        writer.putKeyValue("wheelBase", rearSubTrackWheelBaseSpin->value());
        writer.putKeyValue("on", rearSubTrackCheck->isChecked());
        putKeyVector3(&writer, "diffuseColor", getColor(rearSubTrackColorButton));
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "SPACER");
        writer.putKeyValue("mass", spacerMassSpin->value());
        writer.putKeyValue("radius", spacerRadiusSpin->value());
        writer.putKeyValue("width", spacerWidthSpin->value());
        putKeyVector3(&writer, "diffuseColor", getColor(spacerColorButton));
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "TRACKBELT");
        writer.putKeyValue("numberOfNodes", trackBeltNumberOfNodesSpin->value());
        writer.putKeyValue("nodeThickness", trackBeltNodeThicknessSpin->value());
        writer.putKeyValue("nodeWidth", trackBeltNodeWidthSpin->value());
        writer.putKeyValue("nodeThickerThickness", trackBeltNodeThickerThicknessSpin->value());
        writer.putKeyValue("useThickerNodeEvery", trackBeltUseThickerNodeEverySpin->value());
        writer.putKeyValue("nodeDistanceTensionMantissa", trackBeltNodeDistanceTensionMantissaSpin->value());
        writer.putKeyValue("nodeDistanceTensionExponent", trackBeltNodeDistanceTensionExponentSpin->value());
        writer.putKeyValue("stabilizingHingeFrictionParameterMantissa", trackBeltStabilizingHingeFrictionParameterMantissaSpin->value());
        writer.putKeyValue("stabilizingHingeFrictionParameterExponent", trackBeltStabilizingHingeFrictionParameterExponentSpin->value());
        writer.putKeyValue("minStabilizingHingeNormalForce", trackBeltMinStabilizingHingeNormalForceSpin->value());
        writer.putKeyValue("hingeComplianceMantissa", trackBeltHingeComplianceMantissaSpin->value());
        writer.putKeyValue("hingeComplianceExponent", trackBeltHingeComplianceExponentSpin->value());
        writer.putKeyValue("hingeSpookDamping", trackBeltHingeSpookDampingSpin->value());
        writer.putKeyValue("nodesToWheelsMergeThreshold", trackBeltNodesToWheelsMergeThresholdSpin->value());
        writer.putKeyValue("nodesToWheelsSplitThreshold", trackBeltNodesToWheelsSplitThresholdSpin->value());
        writer.putKeyValue("on", trackBeltCheck->isChecked());
        writer.endMapping();

        writer.startMapping();
        writer.putKeyValue("name", "SUBTRACKBELT");
        writer.putKeyValue("numberOfNodes", subTrackBeltNumberOfNodesSpin->value());
        writer.putKeyValue("nodeThickness", subTrackBeltNodeThicknessSpin->value());
        writer.putKeyValue("nodeWidth", subTrackBeltNodeWidthSpin->value());
        writer.putKeyValue("nodeThickerThickness", subTrackBeltNodeThickerThicknessSpin->value());
        writer.putKeyValue("useThickerNodeEvery", subTrackBeltUseThickerNodeEverySpin->value());
        writer.putKeyValue("nodeDistanceTensionMantissa", subTrackBeltNodeDistanceTensionMantissaSpin->value());
        writer.putKeyValue("nodeDistanceTensionExponent", subTrackBeltNodeDistanceTensionExponentSpin->value());
        writer.putKeyValue("stabilizingHingeFrictionParameterMantissa", subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->value());
        writer.putKeyValue("stabilizingHingeFrictionParameteExponentr", subTrackBeltStabilizingHingeFrictionParameterExponentSpin->value());
        writer.putKeyValue("minStabilizingHingeNormalForce", subTrackBeltMinStabilizingHingeNormalForceSpin->value());
        writer.putKeyValue("hingeComplianceMantissa", subTrackBeltHingeComplianceMantissaSpin->value());
        writer.putKeyValue("hingeComplianceExponent", subTrackBeltHingeComplianceExponentSpin->value());
        writer.putKeyValue("hingeSpookDamping", subTrackBeltHingeSpookDampingSpin->value());
        writer.putKeyValue("nodesToWheelsMergeThreshold", subTrackBeltNodesToWheelsMergeThresholdSpin->value());
        writer.putKeyValue("nodesToWheelsSplitThreshold", subTrackBeltNodesToWheelsSplitThresholdSpin->value());
        writer.endMapping();

        writer.endListing();
        writer.endMapping();
    }
}


void CrawlerRobotBuilderDialogImpl::onEnableAgxCheckToggled(bool on)
{
    trackBeltNumberOfNodesSpin->setEnabled(on);
    trackBeltNodeThicknessSpin->setEnabled(on);
    trackBeltNodeWidthSpin->setEnabled(on);
    trackBeltNodeThickerThicknessSpin->setEnabled(on);
    trackBeltUseThickerNodeEverySpin->setEnabled(on);
    trackBeltNodeDistanceTensionMantissaSpin->setEnabled(on);
    trackBeltNodeDistanceTensionExponentSpin->setEnabled(on);
    trackBeltStabilizingHingeFrictionParameterMantissaSpin->setEnabled(on);
    trackBeltStabilizingHingeFrictionParameterExponentSpin->setEnabled(on);
    trackBeltMinStabilizingHingeNormalForceSpin->setEnabled(on);
    trackBeltHingeComplianceMantissaSpin->setEnabled(on);
    trackBeltHingeComplianceExponentSpin->setEnabled(on);
    trackBeltHingeSpookDampingSpin->setEnabled(on);
    trackBeltNodesToWheelsMergeThresholdSpin->setEnabled(on);
    trackBeltNodesToWheelsSplitThresholdSpin->setEnabled(on);

    subTrackBeltNumberOfNodesSpin->setEnabled(on);
    subTrackBeltNodeThicknessSpin->setEnabled(on);
    subTrackBeltNodeWidthSpin->setEnabled(on);
    subTrackBeltNodeThickerThicknessSpin->setEnabled(on);
    subTrackBeltUseThickerNodeEverySpin->setEnabled(on);
    subTrackBeltNodeDistanceTensionMantissaSpin->setEnabled(on);
    subTrackBeltNodeDistanceTensionExponentSpin->setEnabled(on);
    subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->setEnabled(on);
    subTrackBeltStabilizingHingeFrictionParameterExponentSpin->setEnabled(on);
    subTrackBeltMinStabilizingHingeNormalForceSpin->setEnabled(on);
    subTrackBeltHingeComplianceMantissaSpin->setEnabled(on);
    subTrackBeltHingeComplianceExponentSpin->setEnabled(on);
    subTrackBeltHingeSpookDampingSpin->setEnabled(on);
    subTrackBeltNodesToWheelsMergeThresholdSpin->setEnabled(on);
    subTrackBeltNodesToWheelsSplitThresholdSpin->setEnabled(on);
}


void CrawlerRobotBuilderDialogImpl::onExportBodyButtonClicked()
{
    QString currentDirectory = QString::fromStdString(ProjectManager::instance()->currentProjectDirectory());
    QString bodyFileName = QFileDialog::getSaveFileName(nullptr, "export body file", currentDirectory,
                                                        "Body files (*.body)");
    if(!bodyFileName.isEmpty()) {
        QFileInfo info(bodyFileName);
        if(info.suffix().isEmpty()) {
            bodyFileName += ".body";
        }
        if(!trackBeltCheck->isChecked()) {
            onExportBody(bodyFileName);
        }
        else {
            onExportAGXBody(bodyFileName);
        }
    }
}


void CrawlerRobotBuilderDialogImpl::onExportBody(QString fileName)
{
    QFileInfo info(fileName);
    string bodyName = info.baseName().toStdString();

    FILE* fp = fopen(fileName.toStdString().c_str(), "w");
    if(fp == NULL) {
        return;
    }

    int id = 0;
    fprintf(fp, "format: ChoreonoidBody\n");
    fprintf(fp, "formatVersion: 1.0\n");
    fprintf(fp, "angleUnit: degree\n");
    fprintf(fp, "name: %s\n", bodyName.c_str());
    fprintf(fp, "\n");
    fprintf(fp, "TRACK_BODY: &TrackBody\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  actuationMode: jointSurfaceVelocity\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", trackMassSpin->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(trackMassSpin->value(), trackWheelBaseSpin->value(), trackWidthSpin->value(), trackRadiusSpin->value() * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                trackWheelBaseSpin->value() / 2.0 + trackRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                trackRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -trackWheelBaseSpin->value() / 2.0 + trackRadiusSpin->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                trackRadiusSpin->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                trackWheelBaseSpin->value() / 2.0 + trackRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                trackRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -trackWidthSpin->value() / 2.0, trackWidthSpin->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(trackColorButton)[0], getColor(trackColorButton)[1], getColor(trackColorButton)[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(trackColorButton)[0], getColor(trackColorButton)[1], getColor(trackColorButton)[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "SPACER_BODY: &SpacerBody\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: -Y\n");
    fprintf(fp, "  mass: %3.2lf\n", spacerMassSpin->value());
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spacerMassSpin->value(), spacerRadiusSpin->value(), spacerWidthSpin->value()).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", spacerRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", spacerWidthSpin->value());
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(spacerColorButton)[0], getColor(spacerColorButton)[1], getColor(spacerColorButton)[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(spacerColorButton)[0], getColor(spacerColorButton)[1], getColor(spacerColorButton)[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "TRACKF_BODY: &TrackFBody\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  actuationMode: jointSurfaceVelocity\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", frontSubTrackMassSpin->value());
    double frontSubTrackRadius = std::max(frontSubTrackForwardRadiusSpin->value(), frontSubTrackBackwardRadiusSpin->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(frontSubTrackMassSpin->value(), frontSubTrackWheelBaseSpin->value(), frontSubTrackWidthSpin->value(), frontSubTrackRadius * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                frontSubTrackWheelBaseSpin->value() / 2.0 + frontSubTrackForwardRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                frontSubTrackForwardRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -frontSubTrackWheelBaseSpin->value() / 2.0 + frontSubTrackBackwardRadiusSpin->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                frontSubTrackBackwardRadiusSpin->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                frontSubTrackWheelBaseSpin->value() / 2.0 + frontSubTrackForwardRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                frontSubTrackForwardRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -frontSubTrackWidthSpin->value() / 2.0, frontSubTrackWidthSpin->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(frontSubTrackColorButton)[0], getColor(frontSubTrackColorButton)[1], getColor(frontSubTrackColorButton)[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(frontSubTrackColorButton)[0], getColor(frontSubTrackColorButton)[1], getColor(frontSubTrackColorButton)[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "TRACKR_BODY: &TrackRBody\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  actuationMode: jointSurfaceVelocity\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", rearSubTrackMassSpin->value());
    double rearSubTrackRadius = std::max(frontSubTrackForwardRadiusSpin->value(), frontSubTrackBackwardRadiusSpin->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(rearSubTrackMassSpin->value(), rearSubTrackWheelBaseSpin->value(), rearSubTrackWidthSpin->value(), rearSubTrackRadius * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                rearSubTrackWheelBaseSpin->value() / 2.0 + rearSubTrackForwardRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                rearSubTrackForwardRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -rearSubTrackWheelBaseSpin->value() / 2.0 + rearSubTrackBackwardRadiusSpin->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                rearSubTrackBackwardRadiusSpin->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                rearSubTrackWheelBaseSpin->value() / 2.0 + rearSubTrackForwardRadiusSpin->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                rearSubTrackForwardRadiusSpin->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -rearSubTrackWidthSpin->value() / 2.0, rearSubTrackWidthSpin->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(rearSubTrackColorButton)[0], getColor(rearSubTrackColorButton)[1], getColor(rearSubTrackColorButton)[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(rearSubTrackColorButton)[0], getColor(rearSubTrackColorButton)[1], getColor(rearSubTrackColorButton)[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "links:\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: CHASSIS\n");
    fprintf(fp, "    translation: [ 0, 0, 0 ]\n");
    fprintf(fp, "    jointType: free\n");
    fprintf(fp, "    centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "    mass: %3.2lf\n", chassisMassSpin->value());
    fprintf(fp, "    inertia: [ %s ]\n", boxInertia(chassisMassSpin->value(), chassisXSizeSpin->value(), chassisYSizeSpin->value(), chassisZSizeSpin->value()).c_str());
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: Shape\n");
    fprintf(fp, "        geometry:\n");
    fprintf(fp, "          type: Box\n");
    fprintf(fp, "          size: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            chassisXSizeSpin->value(),
            chassisYSizeSpin->value(),
            chassisZSizeSpin->value());
    fprintf(fp, "        appearance:\n");
    fprintf(fp, "          material:\n");
    fprintf(fp, "            diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            getColor(chassisColorButton)[0],
            getColor(chassisColorButton)[1],
            getColor(chassisColorButton)[2]);
    fprintf(fp, "            specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            getColor(chassisColorButton)[0],
            getColor(chassisColorButton)[1],
            getColor(chassisColorButton)[2]);
    fprintf(fp, "            shininess: 0.6\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    <<: *TrackBody\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_R\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (-(chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0), -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    <<: *TrackBody\n");

    if(frontSubTrackCheck->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(),
                -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                -(chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 - trackWidthSpin->value(),
                -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                frontSubTrackWheelBaseSpin->value() / 2.0,
                (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *TrackFBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                frontSubTrackWheelBaseSpin->value() / 2.0,
                -(spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *TrackFBody\n");
    }

    if(rearSubTrackCheck->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LR\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                -trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(),
                -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RR\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                -trackWheelBaseSpin->value() / 2.0,
                -(chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 - trackWidthSpin->value(),
                -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                -rearSubTrackWheelBaseSpin->value() / 2.0,
                (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *TrackRBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                -rearSubTrackWheelBaseSpin->value() / 2.0,
                -(spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *TrackRBody\n");
    }

    fclose(fp);
}


void CrawlerRobotBuilderDialogImpl::onExportAGXBody(QString fileName)
{
    QFileInfo info(fileName);
    string bodyName = info.baseName().toStdString();

    FILE* fp = fopen(fileName.toStdString().c_str(), "w");
    if(fp == NULL) {
        return;
    }

    int id = 0;
    fprintf(fp, "format: ChoreonoidBody\n");
    fprintf(fp, "formatVersion: 1.0\n");
    fprintf(fp, "angleUnit: degree\n");
    fprintf(fp, "name: %s\n", bodyName.c_str());
    fprintf(fp, "\n");

    fprintf(fp, "TRACK_COMMON: &TrackCommon\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", trackMassSpin->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(trackMassSpin->value() / 3.0, trackWheelBaseSpin->value(),
                                                     trackWidthSpin->value(), trackRadiusSpin->value() * 2.0).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACK_F_COMMON: &SubTrackFCommon\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", frontSubTrackMassSpin->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(frontSubTrackMassSpin->value() / 3.0, frontSubTrackWheelBaseSpin->value(),
                                                     frontSubTrackWidthSpin->value(), std::max(frontSubTrackForwardRadiusSpin->value(), frontSubTrackBackwardRadiusSpin->value())).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACK_R_COMMON: &SubTrackRCommon\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", rearSubTrackMassSpin->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(rearSubTrackMassSpin->value() / 3.0, rearSubTrackWheelBaseSpin->value(),
                                                     rearSubTrackWidthSpin->value(), std::max(rearSubTrackForwardRadiusSpin->value(), rearSubTrackBackwardRadiusSpin->value())).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SPACER_COMMON: &SpacerCommon\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: -Y\n");
    fprintf(fp, "  mass: %3.2lf\n", spacerMassSpin->value());
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spacerMassSpin->value(), spacerRadiusSpin->value(), spacerWidthSpin->value()).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", spacerRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", spacerWidthSpin->value());
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    Vector3 spacerColor = getColor(spacerColorButton);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "TRACKBELT_COMMON: &TrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", trackBeltNumberOfNodesSpin->value());
    fprintf(fp, "  nodeThickness: %lf\n", trackBeltNodeThicknessSpin->value());
    fprintf(fp, "  nodeWidth:  %lf\n", trackBeltNodeWidthSpin->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", trackBeltNodeThickerThicknessSpin->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", trackBeltUseThickerNodeEverySpin->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "#  nodeDistanceTension: %2.1lfe-%d\n", trackBeltNodeDistanceTensionMantissaSpin->value(), trackBeltNodeDistanceTensionExponentSpin->value());
    fprintf(fp, "#  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", trackBeltStabilizingHingeFrictionParameterMantissaSpin->value(), trackBeltStabilizingHingeFrictionParameterExponentSpin->value());
    fprintf(fp, "#  minStabilizingHingeNormalForce: %d\n", trackBeltMinStabilizingHingeNormalForceSpin->value());
    fprintf(fp, "#  hingeCompliance: %2.1lfe-%d\n", trackBeltHingeComplianceMantissaSpin->value(), trackBeltHingeComplianceExponentSpin->value());
    fprintf(fp, "#  hingeSpookDamping: %lf\n", trackBeltHingeSpookDampingSpin->value());
    fprintf(fp, "#  nodesToWheelsMergeThreshold: %lf\n", trackBeltNodesToWheelsMergeThresholdSpin->value());
    fprintf(fp, "#  nodesToWheelsSplitThreshold: %lf\n", trackBeltNodesToWheelsSplitThresholdSpin->value());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACKBELT_COMMON: &SubTrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", subTrackBeltNumberOfNodesSpin->value());
    fprintf(fp, "  nodeThickness: %lf\n", subTrackBeltNodeThicknessSpin->value());
    fprintf(fp, "  nodeWidth: %lf\n", subTrackBeltNodeWidthSpin->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", subTrackBeltNodeThickerThicknessSpin->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", subTrackBeltUseThickerNodeEverySpin->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "#  nodeDistanceTension: %2.1lfe-%d\n", subTrackBeltNodeDistanceTensionMantissaSpin->value(), subTrackBeltNodeDistanceTensionExponentSpin->value());
    fprintf(fp, "#  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", subTrackBeltStabilizingHingeFrictionParameterMantissaSpin->value(), subTrackBeltStabilizingHingeFrictionParameterExponentSpin->value());
    fprintf(fp, "#  minStabilizingHingeNormalForce: %d\n", subTrackBeltMinStabilizingHingeNormalForceSpin->value());
    fprintf(fp, "#  hingeCompliance: %2.1lfe-%d\n", subTrackBeltHingeComplianceMantissaSpin->value(), subTrackBeltHingeComplianceExponentSpin->value());
    fprintf(fp, "#  hingeSpookDamping: %lf\n", subTrackBeltHingeSpookDampingSpin->value());
    fprintf(fp, "#  nodesToWheelsMergeThreshold: %lf\n", subTrackBeltNodesToWheelsMergeThresholdSpin->value());
    fprintf(fp, "#  nodesToWheelsSplitThreshold: %lf\n", subTrackBeltNodesToWheelsSplitThresholdSpin->value());
    fprintf(fp, "\n");

    fprintf(fp, "WHEEL_COMMON: &WheelCommon\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  centerOfMass: [ 0.0, 0.0, 0.0 ]\n");
    fprintf(fp, "  material: %sWheel\n", bodyName.c_str());
    fprintf(fp, "\n");

    fprintf(fp, "MAINWHEEL_COMMON: &MainWheelCommon\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  mass: %3.2lf\n", trackMassSpin->value() * 2.0 / 9.0);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(trackMassSpin->value(), trackRadiusSpin->value(), trackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", trackRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", trackWidthSpin->value());
    fprintf(fp, "      appearance: &WheelAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 trackColor = getColor(trackColorButton);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", trackColor[0], trackColor[1], trackColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", trackColor[0], trackColor[1], trackColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    double r2spf = frontSubTrackBackwardRadiusSpin->value() * frontSubTrackBackwardRadiusSpin->value();
    double r2rof = ((frontSubTrackBackwardRadiusSpin->value() + frontSubTrackForwardRadiusSpin->value()) / 2.0) * ((frontSubTrackBackwardRadiusSpin->value() + frontSubTrackForwardRadiusSpin->value()) / 2.0);
    double r2idf = frontSubTrackForwardRadiusSpin->value() * frontSubTrackForwardRadiusSpin->value();
    double totalf = r2spf + r2rof + r2idf;
    double spmassf = r2spf * r2spf / totalf * frontSubTrackMassSpin->value();
    double romassf = r2rof * r2rof / totalf * frontSubTrackMassSpin->value();
    double idmassf = r2idf * r2idf / totalf * frontSubTrackMassSpin->value();
    fprintf(fp, "SPROCKET_F_COMMON: &SprocketFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", spmassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spmassf, frontSubTrackBackwardRadiusSpin->value(), frontSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", frontSubTrackBackwardRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", frontSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: &SubWheelFAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 subtrackfColor = getColor(frontSubTrackColorButton);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackfColor[0], subtrackfColor[1], subtrackfColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackfColor[0], subtrackfColor[1], subtrackfColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "ROLLER_F_COMMON: &RollerFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", romassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(romassf, (frontSubTrackBackwardRadiusSpin->value() + frontSubTrackForwardRadiusSpin->value()) / 2.0, frontSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", (frontSubTrackBackwardRadiusSpin->value() + frontSubTrackForwardRadiusSpin->value()) / 2.0);
    fprintf(fp, "        height: %3.2lf\n", frontSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: *SubWheelFAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "IDLER_F_COMMON: &IdlerFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", idmassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(idmassf, frontSubTrackForwardRadiusSpin->value(), frontSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", frontSubTrackForwardRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", frontSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: *SubWheelFAppearance\n");
    fprintf(fp, "\n");

    double r2spr = rearSubTrackBackwardRadiusSpin->value() * rearSubTrackBackwardRadiusSpin->value();
    double r2ror = ((rearSubTrackBackwardRadiusSpin->value() + rearSubTrackForwardRadiusSpin->value()) / 2.0) * ((rearSubTrackBackwardRadiusSpin->value() + rearSubTrackForwardRadiusSpin->value()) / 2.0);
    double r2idr = rearSubTrackForwardRadiusSpin->value() * rearSubTrackForwardRadiusSpin->value();
    double totalr = r2spr + r2ror + r2idr;
    double spmassr = r2spr * r2spr / totalr * rearSubTrackMassSpin->value();
    double romassr = r2ror * r2ror / totalr * rearSubTrackMassSpin->value();
    double idmassr = r2idr * r2idr / totalr * rearSubTrackMassSpin->value();
    fprintf(fp, "SPROCKET_R_COMMON: &SprocketRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", spmassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spmassr, rearSubTrackForwardRadiusSpin->value(), rearSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", rearSubTrackForwardRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", rearSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: &SubWheelRAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 subtrackrColor = getColor(rearSubTrackColorButton);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackrColor[0], subtrackrColor[1], subtrackrColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackrColor[0], subtrackrColor[1], subtrackrColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "ROLLER_R_COMMON: &RollerRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", romassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(romassr, (rearSubTrackBackwardRadiusSpin->value() + rearSubTrackForwardRadiusSpin->value()) / 2.0, rearSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", (rearSubTrackBackwardRadiusSpin->value() + rearSubTrackForwardRadiusSpin->value()) / 2.0);
    fprintf(fp, "        height: %3.2lf\n", rearSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: *SubWheelRAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "IDLER_R_COMMON: &IdlerRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n",idmassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(idmassr, rearSubTrackBackwardRadiusSpin->value(), rearSubTrackWidthSpin->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", rearSubTrackBackwardRadiusSpin->value());
    fprintf(fp, "        height: %3.2lf\n", rearSubTrackWidthSpin->value());
    fprintf(fp, "      appearance: *SubWheelRAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "links:\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: CHASSIS\n");
    fprintf(fp, "    translation: [ 0, 0, 0 ]\n");
    fprintf(fp, "    jointType: free\n");
    fprintf(fp, "    centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "    mass: %3.2lf\n", chassisMassSpin->value());
    fprintf(fp, "    inertia: [ %s ]\n", boxInertia(chassisMassSpin->value(), chassisXSizeSpin->value(),
                                                       chassisYSizeSpin->value(), chassisZSizeSpin->value()).c_str());
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: Shape\n");
    fprintf(fp, "        geometry:\n");
    fprintf(fp, "          type: Box\n");
    fprintf(fp, "          size: [ %3.2lf, %3.2lf, %3.2lf ]\n", chassisXSizeSpin->value(),chassisYSizeSpin->value(), chassisZSizeSpin->value() );
    fprintf(fp, "        appearance:\n");
    fprintf(fp, "          material:\n");
    Vector3 chassisColor = getColor(chassisColorButton);
    fprintf(fp, "            diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", chassisColor[0], chassisColor[1], chassisColor[2]);
    fprintf(fp, "            specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", chassisColor[0], chassisColor[1], chassisColor[2]);
    fprintf(fp, "            shininess: 0.6\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    <<: *TrackCommon\n");
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
    fprintf(fp, "        name: TRACK_L\n");
    fprintf(fp, "        sprocketNames: [ SPROCKET_L ]\n");
    fprintf(fp, "        rollerNames: [ ROLLER_L ]\n");
    fprintf(fp, "        idlerNames: [ IDLER_L ]\n");
    fprintf(fp, "        <<: *TrackBeltCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_R\n");
    fprintf(fp, "    translation: [ 0, -%lf, %lf ]\n", (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    <<: *TrackCommon\n");
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
    fprintf(fp, "        name: TRACK_R\n");
    fprintf(fp, "        sprocketNames: [ SPROCKET_R ]\n");
    fprintf(fp, "        rollerNames: [ ROLLER_R ]\n");
    fprintf(fp, "        idlerNames: [ IDLER_R ]\n");
    fprintf(fp, "        nodeWidth: %3.2lf\n", trackWidthSpin->value());
    fprintf(fp, "        <<: *TrackBeltCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: SPROCKET_L\n");
    fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n", trackWheelBaseSpin->value() / 2.0, (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: ROLLER_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: IDLER_L\n");
    fprintf(fp, "    translation: [ -%lf, %lf, %lf ]\n", trackWheelBaseSpin->value() / 2.0, (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: SPROCKET_R\n");
    fprintf(fp, "    translation: [ %lf, -%lf, %lf ]\n", trackWheelBaseSpin->value() / 2.0, (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: ROLLER_R\n");
    fprintf(fp, "    translation: [ 0, -%lf, %lf ]\n", (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: IDLER_R\n");
    fprintf(fp, "    translation: [ -%lf, -%lf, %lf ]\n", trackWheelBaseSpin->value() / 2.0, (chassisYSizeSpin->value() + trackWidthSpin->value()) / 2.0, -chassisZSizeSpin->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    if(frontSubTrackCheck->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(), -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(), -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0 ]\n", (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *SubTrackFCommon\n");
        fprintf(fp, "    elements:\n");
        fprintf(fp, "      -\n");
        fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
        fprintf(fp, "        name: TRACK_LF\n");
        fprintf(fp, "        sprocketNames: [ SPROCKET_LF ]\n");
        fprintf(fp, "        rollerNames: [ ROLLER_LF ]\n");
        fprintf(fp, "        idlerNames: [ IDLER_LF ]\n");
        fprintf(fp, "        <<: *SubTrackBeltCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0 ]\n", (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *SubTrackFCommon\n");
        fprintf(fp, "    elements:\n");
        fprintf(fp, "      -\n");
        fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
        fprintf(fp, "        name: TRACK_RF\n");
        fprintf(fp, "        sprocketNames: [ SPROCKET_RF ]\n");
        fprintf(fp, "        rollerNames: [ ROLLER_RF ]\n");
        fprintf(fp, "        idlerNames: [ IDLER_RF ]\n");
        fprintf(fp, "        <<: *SubTrackBeltCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0.0 ]\n", frontSubTrackWheelBaseSpin->value(), (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0.0 ]\n", frontSubTrackWheelBaseSpin->value() / 2.0, (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0.0 ]\n", (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, 0.0 ]\n", frontSubTrackWheelBaseSpin->value(), (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, 0.0 ]\n", frontSubTrackWheelBaseSpin->value() / 2.0, (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0.0 ]\n", (spacerWidthSpin->value() + frontSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketFCommon\n");
    }

    if(rearSubTrackCheck->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LR\n");
        fprintf(fp, "    translation: [ -%lf, %lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(), -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, %lf ]\n",
                trackWheelBaseSpin->value() / 2.0,
                (chassisYSizeSpin->value() + spacerWidthSpin->value()) / 2.0 + trackWidthSpin->value(), -chassisZSizeSpin->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0 ]\n", (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *SubTrackFCommon\n");
        fprintf(fp, "    elements:\n");
        fprintf(fp, "      -\n");
        fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
        fprintf(fp, "        name: TRACK_LR\n");
        fprintf(fp, "        sprocketNames: [ SPROCKET_LR ]\n");
        fprintf(fp, "        rollerNames: [ ROLLER_LR ]\n");
        fprintf(fp, "        idlerNames: [ IDLER_LR ]\n");
        fprintf(fp, "        <<: *SubTrackBeltCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0 ]\n", (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    <<: *SubTrackFCommon\n");
        fprintf(fp, "    elements:\n");
        fprintf(fp, "      -\n");
        fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
        fprintf(fp, "        name: TRACK_RR\n");
        fprintf(fp, "        sprocketNames: [ SPROCKET_RR ]\n");
        fprintf(fp, "        rollerNames: [ ROLLER_RR ]\n");
        fprintf(fp, "        idlerNames: [ IDLER_RR ]\n");
        fprintf(fp, "        <<: *SubTrackBeltCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ -%lf, %lf, 0.0 ]\n", rearSubTrackWheelBaseSpin->value(), (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ -%lf, %lf, 0.0 ]\n", rearSubTrackWheelBaseSpin->value() / 2.0, (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0.0 ]\n", (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, 0.0 ]\n", rearSubTrackWheelBaseSpin->value(), (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, 0.0 ]\n", rearSubTrackWheelBaseSpin->value() / 2.0, (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0.0 ]\n", (spacerWidthSpin->value() + rearSubTrackWidthSpin->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketRCommon\n");
    }
    fclose(fp);
}


void CrawlerRobotBuilderDialogImpl::onColorChanged(PushButton* pushbutton)
{
    QColor selectedColor = QColorDialog::getColor();
    if(!selectedColor.isValid()) {
        selectedColor = QColor(255.0, 255.0, 255.0);
    }
    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    pushbutton->setPalette(palette);
}


void CrawlerRobotBuilderDialogImpl::setColor(PushButton* pushbutton, Vector3 color)
{
    QColor selectedColor;
    selectedColor.setRed(color[0] * 255.0);
    selectedColor.setGreen(color[1] * 255.0);
    selectedColor.setBlue(color[2] * 255.0);
    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    pushbutton->setPalette(palette);
}


Vector3 CrawlerRobotBuilderDialogImpl::getColor(PushButton* colorButton)
{
    QColor selectedColor = colorButton->palette().color(QPalette::Button);
    return Vector3(selectedColor.red() / 255.0, selectedColor.green() / 255.0, selectedColor.blue() / 255.0);
}
