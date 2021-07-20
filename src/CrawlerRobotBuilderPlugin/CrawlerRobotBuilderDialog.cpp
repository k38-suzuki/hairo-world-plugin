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
#include <cnoid/FileDialog>
#include <cnoid/MainWindow>
#include <cnoid/MenuManager>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/UTF8>
#include <cnoid/Widget>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <cmath>
#include <stdio.h>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

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


struct CheckInfo {
    bool checked;
};


CheckInfo checkInfo[] = {
    {  true },
    {  true },
    { false }
};


struct ButtonInfo {
    double red;
    double green;
    double blue;
};


ButtonInfo buttonInfo[] = {
    {   0.0 / 255.0, 153.0 / 255.0,  0.0 / 255.0 },
    {  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    {  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    {  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    { 255.0 / 255.0,   0.0 / 255.0,  0.0 / 255.0 }
};


struct DoubleSpinInfo {
    double value;
    double min;
    double max;
    int decimals;
    bool enabled;
};


DoubleSpinInfo doubleSpinInfo[] = {
    {  8.000, 0.0, 9999.999, 3,  true }, {  0.450, 0.0, 9999.999, 3,  true }, {  0.300, 0.0, 9999.999, 3,  true }, {  0.100, 0.0, 9999.999, 3,  true },
    {  1.000, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.100, 0.0, 9999.999, 3,  true }, {  0.420, 0.0, 9999.999, 3,  true },
    {  0.250, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.130, 0.0, 9999.999, 3,  true },
    {  0.250, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.080, 0.0, 9999.999, 3,  true }, {  0.130, 0.0, 9999.999, 3,  true },
    {  0.200, 0.0, 9999.999, 3,  true }, {  0.060, 0.0, 9999.999, 3,  true }, {  0.013, 0.0, 9999.999, 3,  true },
    {  0.010, 0.0, 9999.999, 3, false }, {  0.090, 0.0, 9999.999, 3, false }, {  0.020, 0.0, 9999.999, 3, false }, {  2.000, 0.0, 9999.999, 3, false }, {  1.000, 0.0, 9999.999, 3, false },
    {  9.000, 0.0, 9999.999, 3, false }, {  0.010, 0.0, 9999.999, 3, false }, { -0.001, 0.0, 9999.999, 3, false }, { -0.009, 0.0, 9999.999, 3, false },
    {  0.010, 0.0, 9999.999, 3, false }, {  0.090, 0.0, 9999.999, 3, false }, {  0.020, 0.0, 9999.999, 3, false }, {  2.000, 0.0, 9999.999, 3, false }, {  1.000, 0.0, 9999.999, 3, false },
    {  9.000, 0.0, 9999.999, 3, false }, {  0.010, 0.0, 9999.999, 3, false }, { -0.001, 0.0, 9999.999, 3, false }, { -0.009, 0.0, 9999.999, 3, false }
};


struct SpinInfo {
    double value;
    double min;
    double max;
    bool enabled;
};


SpinInfo spinInfo[] = {
    { 42, 0, 9999, false }, {   3, 0, 9999, false }, {  4, 0, 9999, false },
    {  6, 0, 9999, false }, { 100, 0, 9999, false }, { 10, 0, 9999, false },
    { 42, 0, 9999, false }, {   3, 0, 9999, false }, {  4, 0, 9999, false },
    {  6, 0, 9999, false }, { 100, 0, 9999, false }, { 10, 0, 9999, false }
};


struct DialogButtonInfo {
    QDialogButtonBox::ButtonRole role;
};


DialogButtonInfo dialogButtonInfo[] = {
    {  QDialogButtonBox::ResetRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::ActionRole },
    { QDialogButtonBox::AcceptRole }
};

}


namespace cnoid {

class CrawlerRobotBuilderDialogImpl
{
public:
    CrawlerRobotBuilderDialogImpl(CrawlerRobotBuilderDialog* self);
    CrawlerRobotBuilderDialog* self;

    enum DoubleSpinId {
        CHS_MAS, CHS_XSZ, CHS_YSZ, CHS_ZSZ,
        TRK_MAS, TRK_RAD, TRK_WDT, TRK_WBS,
        FFL_MAS, FFL_FRD, FFL_RRD, FFL_WDT, FFL_WBS,
        RFL_MAS, RFL_FRD, RFL_RRD, RFL_WDT, RFL_WBS,
        SPC_MAS, SPC_RAD, SPC_WDT,
        TRK_BNT, TRK_BNW, TRK_BNTT, TRK_BNDTM, TRK_BSHFPM,
        TRK_BHCM, TRK_BHSD, TRK_BNWMT, TRK_BNWST,
        FLP_BNT, FLP_BNW, FLP_BNTT, FLP_BNDTM, FLP_BSHFPM,
        FLP_BHCM, FLP_BHSD, FLP_BNWMT, FLP_BNWST,
        NUM_DSPINS
    };

    enum SpinId {
        TRK_BNN, TRK_BUTNE, TRK_BNDTE,
        TRK_BSHFPE, TRK_BMSHNF, TRK_BHCE,
        FLP_BNN, FLP_BUTNE, FLP_BNDTE,
        FLP_BSHFPE, FLP_BMSHNF, FLP_BHCE,
        NUM_SPINS
    };

    enum ButtonId { CHS_CLR, TRK_CLR, FFL_CLR,
                    RFL_CLR, SPC_CLR, NUM_BUTTONS
    };

    enum CheckId { FFL_CHK, RFL_CHK, AGX_CHK, NUM_CHECKS };

    enum DialogButtonId { RESET, SAVEAS, LOAD, EXPORT, OK, NUM_DBUTTONS };

    CheckBox* checks[NUM_CHECKS];
    PushButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* dspins[NUM_DSPINS];
    SpinBox* spins[NUM_SPINS];
    PushButton* dialogButtons[NUM_DBUTTONS];

    void onAccepted();
    void onRejected();
    void initialize();
    void onResetButtonClicked();
    void onExportYamlButtonClicked();
    void onImportYamlButtonClicked();
    void onExportBodyButtonClicked();
    void onEnableAgxCheckToggled(const bool& on);
    void onExportBody(const string& fileName);
    void onExportAGXBody(const string& fileName);
    void onColorChanged(PushButton* pushbutton);
    void setColor(PushButton* pushbutton, const Vector3& color);
    Vector3 getColor(PushButton* colorButton);
    string getSaveFilename(FileDialog& dialog, const string& suffix);
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

    const char* clabels[] = { _("Front SubTrack"), _("Rear SubTrack"), _("AGX") };
    const char* blabels[] = { _("color"), _("color"), _("color"), _("color"), _("color") };

    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new DoubleSpinBox();
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        spins[i] = new SpinBox();
    }

    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton();
    }

    for(int i = 0; i < NUM_CHECKS; ++i) {
        checks[i] = new CheckBox();
        checks[i]->setText(clabels[i]);
    }

    //chassis
    QVBoxLayout* chassisVbox = new QVBoxLayout();
    QHBoxLayout* cmassHbox = new QHBoxLayout();
    chassisVbox->addLayout(new HSeparatorBox(new QLabel(_("Chassis"))));
    cmassHbox->addWidget(new QLabel(_("mass [kg]")));
    cmassHbox->addWidget(dspins[CHS_MAS]);
    cmassHbox->addWidget(new QLabel(_("color")));
    buttons[CHS_CLR]->sigClicked().connect([&](){ onColorChanged(buttons[CHS_CLR]); });
    cmassHbox->addWidget(buttons[CHS_CLR]);
    chassisVbox->addLayout(cmassHbox);
    QHBoxLayout* csizeHbox = new QHBoxLayout();
    csizeHbox->addWidget(new QLabel(_("size(x-y-z) [m, m, m]")));
    csizeHbox->addWidget(dspins[CHS_XSZ]);
    csizeHbox->addWidget(dspins[CHS_YSZ]);
    csizeHbox->addWidget(dspins[CHS_ZSZ]);
    chassisVbox->addLayout(csizeHbox);

    //track
    QVBoxLayout* trackVbox = new QVBoxLayout();
    QHBoxLayout* tmassHbox = new QHBoxLayout();
    trackVbox->addLayout(new HSeparatorBox(new QLabel(_("Track"))));
    tmassHbox->addWidget(new QLabel(_("mass [kg]")));
    tmassHbox->addWidget(dspins[TRK_MAS]);
    tmassHbox->addWidget(new QLabel(_("color")));
    buttons[TRK_CLR]->sigClicked().connect([&](){ onColorChanged(buttons[TRK_CLR]); });
    tmassHbox->addWidget(buttons[TRK_CLR]);
    trackVbox->addLayout(tmassHbox);
    QHBoxLayout* tradiusHbox = new QHBoxLayout();
    tradiusHbox->addWidget(new QLabel(_("radius [m]")));
    tradiusHbox->addWidget(dspins[TRK_RAD]);
    trackVbox->addLayout(tradiusHbox);
    QHBoxLayout* twidthHbox = new QHBoxLayout();
    twidthHbox->addWidget(new QLabel(_("width [m]")));
    twidthHbox->addWidget(dspins[TRK_WDT]);
    twidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    twidthHbox->addWidget(dspins[TRK_WBS]);
    trackVbox->addLayout(twidthHbox);

    //front subtrack
    QVBoxLayout* frontSubTrackVbox = new QVBoxLayout();
    QWidget* frontSubTrackWidget = new QWidget();
    QHBoxLayout* frontSubTrackTitleHbox = new QHBoxLayout();
    frontSubTrackTitleHbox->addWidget(new QLabel(_("Front SubTrack")));
    frontSubTrackTitleHbox->addWidget(checks[FFL_CHK]);
    frontSubTrackWidget->setLayout(frontSubTrackTitleHbox);
    frontSubTrackVbox->addLayout(new HSeparatorBox(frontSubTrackWidget));
    QHBoxLayout* frontSubTrackmassHbox = new QHBoxLayout();
    frontSubTrackmassHbox->addWidget(new QLabel(_("mass [kg]")));
    frontSubTrackmassHbox->addWidget(dspins[FFL_MAS]);
    frontSubTrackmassHbox->addWidget(new QLabel(_("color")));
    buttons[FFL_CLR]->sigClicked().connect([&](){ onColorChanged(buttons[FFL_CLR]); });
    frontSubTrackmassHbox->addWidget(buttons[FFL_CLR]);
    frontSubTrackVbox->addLayout(frontSubTrackmassHbox);
    QHBoxLayout* frontSubTrackradiusHbox = new QHBoxLayout();
    frontSubTrackradiusHbox->addWidget(new QLabel(_("radius(forward-backward) [m]")));
    frontSubTrackradiusHbox->addWidget(dspins[FFL_FRD]);
    frontSubTrackradiusHbox->addWidget(dspins[FFL_RRD]);
    frontSubTrackVbox->addLayout(frontSubTrackradiusHbox);
    QHBoxLayout* frontSubTrackwidthHbox = new QHBoxLayout();
    frontSubTrackwidthHbox->addWidget(new QLabel(_("width [m]")));
    frontSubTrackwidthHbox->addWidget(dspins[FFL_WDT]);
    frontSubTrackwidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    frontSubTrackwidthHbox->addWidget(dspins[FFL_WBS]);
    frontSubTrackVbox->addLayout(frontSubTrackwidthHbox);

    //rear subtrack
    QVBoxLayout* rearSubTrackVbox = new QVBoxLayout();
    QHBoxLayout* rearSubTrackmassHbox = new QHBoxLayout();
    QWidget* rearSubTrackWidget = new QWidget();
    QHBoxLayout* rearSubTrackHbox = new QHBoxLayout();
    rearSubTrackHbox->addWidget(new QLabel(_("Rear SubTrack")));
    rearSubTrackHbox->addWidget(checks[RFL_CHK]);
    rearSubTrackWidget->setLayout(rearSubTrackHbox);
    rearSubTrackVbox->addLayout(new HSeparatorBox(rearSubTrackWidget));
    rearSubTrackmassHbox->addWidget(new QLabel(_("mass [kg]")));
    rearSubTrackmassHbox->addWidget(dspins[RFL_MAS]);
    rearSubTrackmassHbox->addWidget(new QLabel(_("color")));
    buttons[RFL_CLR]->sigClicked().connect([&](){ onColorChanged(buttons[RFL_CLR]); });
    rearSubTrackmassHbox->addWidget(buttons[RFL_CLR]);
    rearSubTrackVbox->addLayout(rearSubTrackmassHbox);
    QHBoxLayout* rearSubTrackradiusHbox = new QHBoxLayout();
    rearSubTrackradiusHbox->addWidget(new QLabel(_("radius(forward-backward) [m]")));
    rearSubTrackradiusHbox->addWidget(dspins[RFL_FRD]);
    rearSubTrackradiusHbox->addWidget(dspins[RFL_RRD]);
    rearSubTrackVbox->addLayout(rearSubTrackradiusHbox);
    QHBoxLayout* rearSubTrackwidthHbox = new QHBoxLayout();
    rearSubTrackwidthHbox->addWidget(new QLabel(_("width [m]")));
    rearSubTrackwidthHbox->addWidget(dspins[RFL_WDT]);
    rearSubTrackwidthHbox->addWidget(new QLabel(_("wheelbase [m]")));
    rearSubTrackwidthHbox->addWidget(dspins[RFL_WBS]);
    rearSubTrackVbox->addLayout(rearSubTrackwidthHbox);

    //spacer
    QVBoxLayout* spacerVbox = new QVBoxLayout();
    QHBoxLayout* smassHbox = new QHBoxLayout();
    spacerVbox->addLayout(new HSeparatorBox(new QLabel(_("Spacer"))));
    smassHbox->addWidget(new QLabel(_("mass [kg]")));
    smassHbox->addWidget(dspins[SPC_MAS]);
    smassHbox->addWidget(new QLabel(_("color")));
    buttons[SPC_CLR]->sigClicked().connect([&](){ onColorChanged(buttons[SPC_CLR]); });
    smassHbox->addWidget(buttons[SPC_CLR]);
    spacerVbox->addLayout(smassHbox);
    QHBoxLayout* sradiusHbox = new QHBoxLayout();
    sradiusHbox->addWidget(new QLabel(_("radius [m]")));
    sradiusHbox->addWidget(dspins[SPC_RAD]);
    spacerVbox->addLayout(sradiusHbox);
    QHBoxLayout* swidthHbox = new QHBoxLayout();
    swidthHbox->addWidget(new QLabel(_("width [m]")));
    swidthHbox->addWidget(dspins[SPC_WDT]);
    spacerVbox->addLayout(swidthHbox);

    //track belt
    QVBoxLayout* trackBeltVbox = new QVBoxLayout();
    trackBeltVbox->addLayout(new HSeparatorBox(new QLabel(_("Track Belt"))));
    QHBoxLayout* option0Hbox = new QHBoxLayout();
    option0Hbox->addWidget(new QLabel(_("number of nodes [-]")));
    option0Hbox->addWidget(spins[TRK_BNN]);
    option0Hbox->addWidget(new QLabel(_("node thickness [m]")));
    option0Hbox->addWidget(dspins[TRK_BNT]);
    QHBoxLayout* option1Hbox = new QHBoxLayout();
    option1Hbox->addWidget(new QLabel(_("node width [m]")));
    option1Hbox->addWidget(dspins[TRK_BNW]);
    option1Hbox->addWidget(new QLabel(_("node thickerthickness [m]")));
    option1Hbox->addWidget(dspins[TRK_BNTT]);
    QHBoxLayout* option2Hbox = new QHBoxLayout();
    option2Hbox->addWidget(new QLabel(_("use thicker node every [-]")));
    option2Hbox->addWidget(spins[TRK_BUTNE]);
    option2Hbox->addStretch();
    option2Hbox->addWidget(new QLabel(_("node distance tension [m]")));
    option2Hbox->addWidget(dspins[TRK_BNDTM]);
    option2Hbox->addWidget(new QLabel(_("e-")));
    option2Hbox->addWidget(spins[TRK_BNDTE]);
    QHBoxLayout* option3Hbox = new QHBoxLayout();
    option3Hbox->addWidget(new QLabel(_("stabilizing hinge friction parameter [-]")));
    option3Hbox->addWidget(dspins[TRK_BSHFPM]);
    option3Hbox->addWidget(new QLabel(_("e-")));
    option3Hbox->addWidget(spins[TRK_BSHFPE]);
    option3Hbox->addStretch();
    option3Hbox->addWidget(new QLabel(_("min stabilizing hinge normal force [N]")));
    option3Hbox->addWidget(spins[TRK_BMSHNF]);
    QHBoxLayout* option4Hbox = new QHBoxLayout();
    option4Hbox->addWidget(new QLabel(_("hinge compliance [rad/Nm]")));
    option4Hbox->addWidget(dspins[TRK_BHCM]);
    option4Hbox->addWidget(new QLabel(_("e-")));
    option4Hbox->addWidget(spins[TRK_BHCE]);
    option4Hbox->addStretch();
    option4Hbox->addWidget(new QLabel(_("hinge spook damping [s]")));
    option4Hbox->addWidget(dspins[TRK_BHSD]);
    QHBoxLayout* option5Hbox = new QHBoxLayout();
    option5Hbox->addWidget(new QLabel(_("nodes to wheels merge threshold [-]")));
    option5Hbox->addWidget(dspins[TRK_BNWMT]);
    option5Hbox->addWidget(new QLabel(_("nodes to wheels split threshold [-]")));
    option5Hbox->addWidget(dspins[TRK_BNWST]);

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
    option6Hbox->addWidget(spins[FLP_BNN]);
    option6Hbox->addWidget(new QLabel(_("node thickness [m]")));
    option6Hbox->addWidget(dspins[FLP_BNT]);
    QHBoxLayout* option7Hbox = new QHBoxLayout();
    option7Hbox->addWidget(new QLabel(_("node width [m]")));
    option7Hbox->addWidget(dspins[FLP_BNW]);
    option7Hbox->addWidget(new QLabel(_("node thickerthickness [m]")));
    option7Hbox->addWidget(dspins[FLP_BNTT]);
    QHBoxLayout* option8Hbox = new QHBoxLayout();
    option8Hbox->addWidget(new QLabel(_("use thicker node every [-]")));
    option8Hbox->addWidget(spins[FLP_BUTNE]);
    option8Hbox->addStretch();
    option8Hbox->addWidget(new QLabel(_("node distance tension [m]")));
    option8Hbox->addWidget(dspins[FLP_BNDTM]);
    option8Hbox->addWidget(new QLabel(_("e-")));
    option8Hbox->addWidget(spins[FLP_BNDTE]);
    QHBoxLayout* option9Hbox = new QHBoxLayout();
    option9Hbox->addWidget(new QLabel(_("stabilizing hinge friction parameter [-]")));
    option9Hbox->addWidget(dspins[FLP_BSHFPM]);
    option9Hbox->addWidget(new QLabel(_("e-")));
    option9Hbox->addWidget(spins[FLP_BSHFPE]);
    option9Hbox->addWidget(new QLabel(_("min stabilizing hinge normal force [N]")));
    option9Hbox->addWidget(spins[FLP_BMSHNF]);
    QHBoxLayout* option10Hbox = new QHBoxLayout();
    option10Hbox->addWidget(new QLabel(_("hinge compliance [rad/Nm]")));
    option10Hbox->addWidget(dspins[FLP_BHCM]);
    option10Hbox->addWidget(new QLabel(_("e-")));
    option10Hbox->addWidget(spins[FLP_BHCE]);
    option10Hbox->addStretch();
    option10Hbox->addWidget(new QLabel(_("hinge spook damping [s]")));
    option10Hbox->addWidget(dspins[FLP_BHSD]);
    QHBoxLayout* option11Hbox = new QHBoxLayout();
    option11Hbox->addWidget(new QLabel(_("nodes to wheels merge threshold [-]")));
    option11Hbox->addWidget(dspins[FLP_BNWMT]);
    option11Hbox->addWidget(new QLabel(_("nodes to wheels split threshold [-]")));
    option11Hbox->addWidget(dspins[FLP_BNWST]);

    subTrackBeltVbox->addLayout(option6Hbox);
    subTrackBeltVbox->addLayout(option7Hbox);
    subTrackBeltVbox->addLayout(option8Hbox);
    subTrackBeltVbox->addLayout(option9Hbox);
    subTrackBeltVbox->addLayout(option10Hbox);
    subTrackBeltVbox->addLayout(option11Hbox);

    QHBoxLayout* trackBeltHbox = new QHBoxLayout();
    trackBeltHbox->addWidget(checks[AGX_CHK]);
    trackBeltHbox->addStretch();

    const char* plabels[] = {
        _("&Reset"), _("&Save As..."), _("&Load"),
        _("&Export"), _("&Ok")
    };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    for(int i = 0; i < NUM_DBUTTONS; ++i) {
        DialogButtonInfo info = dialogButtonInfo[i];
        dialogButtons[i] = new PushButton(plabels[i]);
        PushButton* dialogButton = dialogButtons[i];
        buttonBox->addButton(dialogButton, info.role);
        if(i == OK) {
            dialogButtons[i]->setDefault(true);
        }
    }

    self->connect(buttonBox,SIGNAL(rejected()), self, SLOT(reject()));
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

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
    mainLeftVbox->addWidget(buttonBox);
    mainLeftVbox->addStretch();
    mainRightVbox->addLayout(trackBeltVbox);
    mainRightVbox->addLayout(subTrackBeltVbox);
    mainRightVbox->addStretch();
    mainHbox->addLayout(mainLeftVbox);
    mainHbox->addSpacing(5);
//    mainHbox->addLayout(mainRightVbox);
    mainVbox->addLayout(trackBeltHbox);
    mainVbox->addLayout(mainHbox);

    initialize();
    dialogButtons[RESET]->sigClicked().connect([&](){ onResetButtonClicked(); });
    dialogButtons[SAVEAS]->sigClicked().connect([&](){ onExportYamlButtonClicked(); });
    dialogButtons[LOAD]->sigClicked().connect([&](){ onImportYamlButtonClicked(); });
    dialogButtons[EXPORT]->sigClicked().connect([&](){ onExportBodyButtonClicked(); });
    checks[AGX_CHK]->sigToggled().connect([&](bool on){ onEnableAgxCheckToggled(on); });
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
    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        DoubleSpinBox* dspin = dspins[i];
        dspin->setValue(info.value);
        dspin->setRange(info.min, info.max);
        dspin->setDecimals(info.decimals);
        dspin->setEnabled(info.enabled);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = spinInfo[i];
        SpinBox* spin = spins[i];
        spin->setValue(info.value);
        spin->setRange(info.min, info.max);
        spin->setEnabled(info.enabled);
    }

    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        PushButton* button = buttons[i];
        setColor(button, Vector3(info.red, info.green, info.blue));
    }

    for(int i = 0; i < NUM_CHECKS; ++i) {
        CheckInfo info = checkInfo[i];
        checks[i]->setChecked(info.checked);
    }
}


void CrawlerRobotBuilderDialogImpl::onResetButtonClicked()
{
    initialize();
}


void CrawlerRobotBuilderDialogImpl::onImportYamlButtonClicked()
{
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Open a configuration file"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Open"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));

    QStringList filters;
    filters << _("YAML files (*.yaml *.yml)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    string filename;
    if(dialog.exec()) {
        filename = dialog.selectedFiles().front().toStdString();
    }

    if(!filename.empty()) {
        filesystem::path path(filename);
        string ext = path.extension().string();
        if(ext.empty()) {
            filename += ".yaml";
        }

        YAMLReader reader;
        if(reader.load(filename)) {
            Mapping* topNode = reader.loadDocument(filename)->toMapping();
            Listing* configList = topNode->findListing("configs");
            if(configList->isValid()) {
                for(int i = 0; i < configList->size(); i++) {
                    Mapping& node = *configList->at(i)->toMapping();

                    Listing* doubleSpinList = node.findListing("doubleSpin");
                    if(doubleSpinList->isValid()) {
                        for(int j = 0; j < doubleSpinList->size(); ++j) {
                            double value = doubleSpinList->at(j)->toDouble();
                            dspins[j]->setValue(value);
                        }
                    }

                    Listing* spinList = node.findListing("spin");
                    if(spinList->isValid()) {
                        for(int j = 0; j < spinList->size(); ++j) {
                            int value = spinList->at(j)->toInt();
                            spins[j]->setValue(value);
                        }
                    }

                    for(int j = 0; j < NUM_BUTTONS; ++j) {
                        Vector3 color;
                        string key = "button" + to_string(j);
                        if(read(node, key, color)) {
                            setColor(buttons[j], color);
                        }
                    }

                    Listing* checkList = node.findListing("check");
                    if(checkList->isValid()) {
                        for(int j = 0; j < checkList->size(); ++j) {
                            bool checked = checkList->at(j)->toBool() ? true : false;
                            checks[j]->setChecked(checked);
                        }
                    }
                }
            }
        }
    }
}


void CrawlerRobotBuilderDialogImpl::onExportYamlButtonClicked()
{
    FileDialog dialog(MainWindow::instance());
    dialog.setWindowTitle(_("Save a configuration file"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, _("Save"));
    dialog.setLabelText(QFileDialog::Reject, _("Cancel"));
    dialog.setOption(QFileDialog::DontConfirmOverwrite);

    QStringList filters;
    filters << _("YAML files (*.yaml *.yml)");
    filters << _("Any files (*)");
    dialog.setNameFilters(filters);

    dialog.updatePresetDirectories();

    ProjectManager* pm = ProjectManager::instance();
    string currentProjectFile = pm->currentProjectFile();
    filesystem::path cpfpath(currentProjectFile);
    string currentProjectName = cpfpath.stem().string();
    if(!dialog.selectFilePath(currentProjectFile)) {
        dialog.selectFile(currentProjectName);
    }

    string filename;
    if(dialog.exec() == QDialog::Accepted) {
        string suffix = ".yaml";
        filename = getSaveFilename(dialog, suffix);
    }

    if(!filename.empty()) {
        filesystem::path path(filename);
        string bodyName = path.stem().string();

        YAMLWriter writer(filename);

        writer.startMapping();
        writer.putKeyValue("format", "CrawlerRobotBuilderYaml");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("name", bodyName);
        writer.putKey("configs");
        writer.startListing();

        writer.startMapping();
        writer.putKey("doubleSpin");
        writer.startFlowStyleListing();
        for(int i = 0; i < NUM_DSPINS; ++i) {
            writer.putScalar(dspins[i]->value());
        }
        writer.endListing();

        writer.putKey("spin");
        writer.startFlowStyleListing();
        for(int i = 0; i < NUM_SPINS; ++i) {
            writer.putScalar(spins[i]->value());
        }
        writer.endListing();

        for(int i = 0; i < NUM_BUTTONS; ++i) {
            string key = "button" + to_string(i);
            putKeyVector3(&writer, key, getColor(buttons[i]));
        }

        writer.putKey("check");
        writer.startFlowStyleListing();
        for(int i = 0; i < NUM_CHECKS; ++i) {
            writer.putScalar(checks[i]->isChecked());
        }
        writer.endListing();

        writer.endListing();
        writer.endMapping();
    }
}


void CrawlerRobotBuilderDialogImpl::onEnableAgxCheckToggled(const bool& on)
{
    spins[TRK_BNN]->setEnabled(on);
    dspins[TRK_BNT]->setEnabled(on);
    dspins[TRK_BNW]->setEnabled(on);
    dspins[TRK_BNTT]->setEnabled(on);
    spins[TRK_BUTNE]->setEnabled(on);
    dspins[TRK_BNDTM]->setEnabled(on);
    spins[TRK_BNDTE]->setEnabled(on);
    dspins[TRK_BSHFPM]->setEnabled(on);
    spins[TRK_BSHFPE]->setEnabled(on);
    spins[TRK_BMSHNF]->setEnabled(on);
    dspins[TRK_BHCM]->setEnabled(on);
    spins[TRK_BHCE]->setEnabled(on);
    dspins[TRK_BHSD]->setEnabled(on);
    dspins[TRK_BNWMT]->setEnabled(on);
    dspins[TRK_BNWST]->setEnabled(on);

    spins[FLP_BNN]->setEnabled(on);
    dspins[FLP_BNT]->setEnabled(on);
    dspins[FLP_BNW]->setEnabled(on);
    dspins[FLP_BNTT]->setEnabled(on);
    spins[FLP_BUTNE]->setEnabled(on);
    dspins[FLP_BNDTM]->setEnabled(on);
    spins[FLP_BNDTE]->setEnabled(on);
    dspins[FLP_BSHFPM]->setEnabled(on);
    spins[FLP_BSHFPE]->setEnabled(on);
    spins[FLP_BMSHNF]->setEnabled(on);
    dspins[FLP_BHCM]->setEnabled(on);
    spins[FLP_BHCE]->setEnabled(on);
    dspins[FLP_BHSD]->setEnabled(on);
    dspins[FLP_BNWMT]->setEnabled(on);
    dspins[FLP_BNWST]->setEnabled(on);
}


void CrawlerRobotBuilderDialogImpl::onExportBodyButtonClicked()
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

    string filename;
    if(dialog.exec() == QDialog::Accepted) {
        string suffix = ".body";
        filename = getSaveFilename(dialog, suffix);
    }

    if(!filename.empty()) {
        if(!checks[AGX_CHK]->isChecked()) {
            onExportBody(filename);
        } else {
            onExportAGXBody(filename);
        }
    }
}


void CrawlerRobotBuilderDialogImpl::onExportBody(const string& fileName)
{
    filesystem::path path(fileName);
    string bodyName = path.stem().string();

    FILE* fp = fopen(fileName.c_str(), "w");
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
    fprintf(fp, "  jointType: pseudo_continuous_track\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[TRK_MAS]->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[TRK_MAS]->value(), dspins[TRK_WBS]->value(), dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[TRK_RAD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                dspins[TRK_RAD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[TRK_RAD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -dspins[TRK_WDT]->value() / 2.0, dspins[TRK_WDT]->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[TRK_CLR])[0], getColor(buttons[TRK_CLR])[1], getColor(buttons[TRK_CLR])[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[TRK_CLR])[0], getColor(buttons[TRK_CLR])[1], getColor(buttons[TRK_CLR])[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "SPACER_BODY: &SpacerBody\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: -Y\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[SPC_MAS]->value());
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(dspins[SPC_MAS]->value(), dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value()).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[SPC_RAD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[SPC_WDT]->value());
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[SPC_CLR])[0], getColor(buttons[SPC_CLR])[1], getColor(buttons[SPC_CLR])[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[SPC_CLR])[0], getColor(buttons[SPC_CLR])[1], getColor(buttons[SPC_CLR])[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "TRACKF_BODY: &TrackFBody\n");
    fprintf(fp, "  jointType: pseudo_continuous_track\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[FFL_MAS]->value());
    double frontSubTrackRadius = std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[FFL_MAS]->value(), dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), frontSubTrackRadius * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[FFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_RRD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                dspins[FFL_RRD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[FFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -dspins[FFL_WDT]->value() / 2.0, dspins[FFL_WDT]->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[FFL_CLR])[0], getColor(buttons[FFL_CLR])[1], getColor(buttons[FFL_CLR])[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[FFL_CLR])[0], getColor(buttons[FFL_CLR])[1], getColor(buttons[FFL_CLR])[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "TRACKR_BODY: &TrackRBody\n");
    fprintf(fp, "  jointType: pseudo_continuous_track\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[RFL_MAS]->value());
    double rearSubTrackRadius = std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value());
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[RFL_MAS]->value(), dspins[RFL_WBS]->value(), dspins[RFL_WDT]->value(), rearSubTrackRadius * 2.0).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Extrusion\n");
    fprintf(fp, "        crossSection: [\n");
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[RFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 9; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                -dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_RRD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN),
                dspins[RFL_RRD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
    }
    for(int i = 0; i < 1; i++) {
        fprintf(fp, "           %lf, %lf, \n",
                dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN),
                dspins[RFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
    }
    fprintf(fp, "          ]\n");
    fprintf(fp, "        spine: [ 0, %lf, 0, 0, %lf, 0 ]\n", -dspins[RFL_WDT]->value() / 2.0, dspins[RFL_WDT]->value() / 2.0);
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[RFL_CLR])[0], getColor(buttons[RFL_CLR])[1], getColor(buttons[RFL_CLR])[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", getColor(buttons[RFL_CLR])[0], getColor(buttons[RFL_CLR])[1], getColor(buttons[RFL_CLR])[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");
    fprintf(fp, "links:\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: CHASSIS\n");
    fprintf(fp, "    translation: [ 0, 0, 0 ]\n");
    fprintf(fp, "    jointType: free\n");
    fprintf(fp, "    centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "    mass: %3.2lf\n", dspins[CHS_MAS]->value());
    fprintf(fp, "    inertia: [ %s ]\n", boxInertia(dspins[CHS_MAS]->value(), dspins[CHS_XSZ]->value(), dspins[CHS_YSZ]->value(), dspins[CHS_ZSZ]->value()).c_str());
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: Shape\n");
    fprintf(fp, "        geometry:\n");
    fprintf(fp, "          type: Box\n");
    fprintf(fp, "          size: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            dspins[CHS_XSZ]->value(),
            dspins[CHS_YSZ]->value(),
            dspins[CHS_ZSZ]->value());
    fprintf(fp, "        appearance:\n");
    fprintf(fp, "          material:\n");
    fprintf(fp, "            diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            getColor(buttons[CHS_CLR])[0],
            getColor(buttons[CHS_CLR])[1],
            getColor(buttons[CHS_CLR])[2]);
    fprintf(fp, "            specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n",
            getColor(buttons[CHS_CLR])[0],
            getColor(buttons[CHS_CLR])[1],
            getColor(buttons[CHS_CLR])[2]);
    fprintf(fp, "            shininess: 0.6\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    <<: *TrackBody\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_R\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (-(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0), -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    <<: *TrackBody\n");

    if(checks[FFL_CHK]->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(),
                -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                dspins[FFL_WBS]->value() / 2.0,
                (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    <<: *TrackFBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                dspins[FFL_WBS]->value() / 2.0,
                -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    <<: *TrackFBody\n");
    }

    if(checks[RFL_CHK]->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LR\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                -dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RR\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                -dspins[TRK_WBS]->value() / 2.0,
                -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(),
                -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                -dspins[RFL_WBS]->value() / 2.0,
                (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    <<: *TrackRBody\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0 ]\n",
                -dspins[RFL_WBS]->value() / 2.0,
                -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    <<: *TrackRBody\n");
    }

    fclose(fp);
}


void CrawlerRobotBuilderDialogImpl::onExportAGXBody(const string& fileName)
{
    filesystem::path path(fileName);
    string bodyName = path.stem().string();

    FILE* fp = fopen(fileName.c_str(), "w");
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
    fprintf(fp, "  mass: %3.2lf\n", dspins[TRK_MAS]->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[TRK_MAS]->value() / 3.0, dspins[TRK_WBS]->value(),
                                                     dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACK_F_COMMON: &SubTrackFCommon\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[FFL_MAS]->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[FFL_MAS]->value() / 3.0, dspins[FFL_WBS]->value(),
                                                     dspins[FFL_WDT]->value(), std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value())).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACK_R_COMMON: &SubTrackRCommon\n");
    fprintf(fp, "  jointType: fixed\n");
    fprintf(fp, "  centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[RFL_MAS]->value() / 3.0);
    fprintf(fp, "  inertia: [ %s ]\n", boxInertia(dspins[RFL_MAS]->value() / 3.0, dspins[RFL_WBS]->value(),
                                                     dspins[RFL_WDT]->value(), std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value())).c_str());
    fprintf(fp, "\n");

    fprintf(fp, "SPACER_COMMON: &SpacerCommon\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: -Y\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[SPC_MAS]->value());
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(dspins[SPC_MAS]->value(), dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value()).c_str());
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[SPC_RAD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[SPC_WDT]->value());
    fprintf(fp, "      appearance:\n");
    fprintf(fp, "        material:\n");
    Vector3 spacerColor = getColor(buttons[SPC_CLR]);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "TRACKBELT_COMMON: &TrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", spins[TRK_BNN]->value());
    fprintf(fp, "  nodeThickness: %lf\n", dspins[TRK_BNT]->value());
    fprintf(fp, "  nodeWidth:  %lf\n", dspins[TRK_BNW]->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", dspins[TRK_BNTT]->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", spins[TRK_BUTNE]->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "#  nodeDistanceTension: %2.1lfe-%d\n", dspins[TRK_BNDTM]->value(), spins[TRK_BNDTE]->value());
    fprintf(fp, "#  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", dspins[TRK_BSHFPM]->value(), spins[TRK_BSHFPE]->value());
    fprintf(fp, "#  minStabilizingHingeNormalForce: %d\n", spins[TRK_BMSHNF]->value());
    fprintf(fp, "#  hingeCompliance: %2.1lfe-%d\n", dspins[TRK_BHCM]->value(), spins[TRK_BHCE]->value());
    fprintf(fp, "#  hingeSpookDamping: %lf\n", dspins[TRK_BHSD]->value());
    fprintf(fp, "#  nodesToWheelsMergeThreshold: %lf\n", dspins[TRK_BNWMT]->value());
    fprintf(fp, "#  nodesToWheelsSplitThreshold: %lf\n", dspins[TRK_BNWST]->value());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACKBELT_COMMON: &SubTrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", spins[FLP_BNN]->value());
    fprintf(fp, "  nodeThickness: %lf\n", dspins[FLP_BNT]->value());
    fprintf(fp, "  nodeWidth: %lf\n", dspins[FLP_BNW]->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", dspins[FLP_BNTT]->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", spins[FLP_BUTNE]->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "#  nodeDistanceTension: %2.1lfe-%d\n", dspins[FLP_BNDTM]->value(), spins[FLP_BNDTE]->value());
    fprintf(fp, "#  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", dspins[FLP_BSHFPM]->value(), spins[FLP_BSHFPE]->value());
    fprintf(fp, "#  minStabilizingHingeNormalForce: %d\n", spins[FLP_BMSHNF]->value());
    fprintf(fp, "#  hingeCompliance: %2.1lfe-%d\n", dspins[FLP_BHCM]->value(), spins[FLP_BHCE]->value());
    fprintf(fp, "#  hingeSpookDamping: %lf\n", dspins[FLP_BHSD]->value());
    fprintf(fp, "#  nodesToWheelsMergeThreshold: %lf\n", dspins[FLP_BNWMT]->value());
    fprintf(fp, "#  nodesToWheelsSplitThreshold: %lf\n", dspins[FLP_BNWST]->value());
    fprintf(fp, "\n");

    fprintf(fp, "WHEEL_COMMON: &WheelCommon\n");
    fprintf(fp, "  jointType: revolute\n");
    fprintf(fp, "  jointAxis: Y\n");
    fprintf(fp, "  centerOfMass: [ 0.0, 0.0, 0.0 ]\n");
    fprintf(fp, "  material: %sWheel\n", bodyName.c_str());
    fprintf(fp, "\n");

    fprintf(fp, "MAINWHEEL_COMMON: &MainWheelCommon\n");
    fprintf(fp, "  parent: CHASSIS\n");
    fprintf(fp, "  mass: %3.2lf\n", dspins[TRK_MAS]->value() * 2.0 / 9.0);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(dspins[TRK_MAS]->value(), dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[TRK_RAD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[TRK_WDT]->value());
    fprintf(fp, "      appearance: &WheelAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 trackColor = getColor(buttons[TRK_CLR]);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", trackColor[0], trackColor[1], trackColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", trackColor[0], trackColor[1], trackColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double spmassf = r2spf * r2spf / totalf * dspins[FFL_MAS]->value();
    double romassf = r2rof * r2rof / totalf * dspins[FFL_MAS]->value();
    double idmassf = r2idf * r2idf / totalf * dspins[FFL_MAS]->value();
    fprintf(fp, "SPROCKET_F_COMMON: &SprocketFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", spmassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spmassf, dspins[FFL_RRD]->value(), dspins[FFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[FFL_RRD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[FFL_WDT]->value());
    fprintf(fp, "      appearance: &SubWheelFAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 subtrackfColor = getColor(buttons[FFL_CLR]);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackfColor[0], subtrackfColor[1], subtrackfColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackfColor[0], subtrackfColor[1], subtrackfColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "ROLLER_F_COMMON: &RollerFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", romassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(romassf, (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0, dspins[FFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    fprintf(fp, "        height: %3.2lf\n", dspins[FFL_WDT]->value());
    fprintf(fp, "      appearance: *SubWheelFAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "IDLER_F_COMMON: &IdlerFCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", idmassf);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(idmassf, dspins[FFL_FRD]->value(), dspins[FFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[FFL_FRD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[FFL_WDT]->value());
    fprintf(fp, "      appearance: *SubWheelFAppearance\n");
    fprintf(fp, "\n");

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double spmassr = r2spr * r2spr / totalr * dspins[RFL_MAS]->value();
    double romassr = r2ror * r2ror / totalr * dspins[RFL_MAS]->value();
    double idmassr = r2idr * r2idr / totalr * dspins[RFL_MAS]->value();
    fprintf(fp, "SPROCKET_R_COMMON: &SprocketRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", spmassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(spmassr, dspins[RFL_FRD]->value(), dspins[RFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[RFL_FRD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[RFL_WDT]->value());
    fprintf(fp, "      appearance: &SubWheelRAppearance\n");
    fprintf(fp, "        material:\n");
    Vector3 subtrackrColor = getColor(buttons[RFL_CLR]);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackrColor[0], subtrackrColor[1], subtrackrColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", subtrackrColor[0], subtrackrColor[1], subtrackrColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "ROLLER_R_COMMON: &RollerRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n", romassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(romassr, (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0, dspins[RFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    fprintf(fp, "        height: %3.2lf\n", dspins[RFL_WDT]->value());
    fprintf(fp, "      appearance: *SubWheelRAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "IDLER_R_COMMON: &IdlerRCommon\n");
    fprintf(fp, "  mass: %3.2lf\n",idmassr);
    fprintf(fp, "  inertia: [ %s ]\n", cylinderInertia(idmassr, dspins[RFL_RRD]->value(), dspins[RFL_WDT]->value()).c_str());
    fprintf(fp, "  <<: *WheelCommon\n");
    fprintf(fp, "  elements:\n");
    fprintf(fp, "    -\n");
    fprintf(fp, "      type: Shape\n");
    fprintf(fp, "      geometry:\n");
    fprintf(fp, "        type: Cylinder\n");
    fprintf(fp, "        radius: %3.2lf\n", dspins[RFL_RRD]->value());
    fprintf(fp, "        height: %3.2lf\n", dspins[RFL_WDT]->value());
    fprintf(fp, "      appearance: *SubWheelRAppearance\n");
    fprintf(fp, "\n");

    fprintf(fp, "links:\n");
    fprintf(fp, "  -\n");
    fprintf(fp, "    name: CHASSIS\n");
    fprintf(fp, "    translation: [ 0, 0, 0 ]\n");
    fprintf(fp, "    jointType: free\n");
    fprintf(fp, "    centerOfMass: [ 0, 0, 0 ]\n");
    fprintf(fp, "    mass: %3.2lf\n", dspins[CHS_MAS]->value());
    fprintf(fp, "    inertia: [ %s ]\n", boxInertia(dspins[CHS_MAS]->value(), dspins[CHS_XSZ]->value(),
                                                       dspins[CHS_YSZ]->value(), dspins[CHS_ZSZ]->value()).c_str());
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: Shape\n");
    fprintf(fp, "        geometry:\n");
    fprintf(fp, "          type: Box\n");
    fprintf(fp, "          size: [ %3.2lf, %3.2lf, %3.2lf ]\n", dspins[CHS_XSZ]->value(),dspins[CHS_YSZ]->value(), dspins[CHS_ZSZ]->value() );
    fprintf(fp, "        appearance:\n");
    fprintf(fp, "          material:\n");
    Vector3 chassisColor = getColor(buttons[CHS_CLR]);
    fprintf(fp, "            diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", chassisColor[0], chassisColor[1], chassisColor[2]);
    fprintf(fp, "            specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", chassisColor[0], chassisColor[1], chassisColor[2]);
    fprintf(fp, "            shininess: 0.6\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: TRACK_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
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
    fprintf(fp, "    translation: [ 0, -%lf, %lf ]\n", (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    <<: *TrackCommon\n");
    fprintf(fp, "    elements:\n");
    fprintf(fp, "      -\n");
    fprintf(fp, "        type: AGXVehicleContinuousTrackDevice\n");
    fprintf(fp, "        name: TRACK_R\n");
    fprintf(fp, "        sprocketNames: [ SPROCKET_R ]\n");
    fprintf(fp, "        rollerNames: [ ROLLER_R ]\n");
    fprintf(fp, "        idlerNames: [ IDLER_R ]\n");
    fprintf(fp, "        nodeWidth: %3.2lf\n", dspins[TRK_WDT]->value());
    fprintf(fp, "        <<: *TrackBeltCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: SPROCKET_L\n");
    fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n", dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: ROLLER_L\n");
    fprintf(fp, "    translation: [ 0, %lf, %lf ]\n", (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: IDLER_L\n");
    fprintf(fp, "    translation: [ -%lf, %lf, %lf ]\n", dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: SPROCKET_R\n");
    fprintf(fp, "    translation: [ %lf, -%lf, %lf ]\n", dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: ROLLER_R\n");
    fprintf(fp, "    translation: [ 0, -%lf, %lf ]\n", (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    fprintf(fp, "  -\n");
    fprintf(fp, "    name: IDLER_R\n");
    fprintf(fp, "    translation: [ -%lf, -%lf, %lf ]\n", dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
    fprintf(fp, "    jointId: %d\n", id++);
    fprintf(fp, "    <<: *MainWheelCommon\n");

    if(checks[FFL_CHK]->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0 ]\n", (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
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
        fprintf(fp, "    translation: [ 0.0, -%lf, 0 ]\n", (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
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
        fprintf(fp, "    translation: [ %lf, %lf, 0.0 ]\n", dspins[FFL_WBS]->value(), (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ %lf, %lf, 0.0 ]\n", dspins[FFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_LF\n");
        fprintf(fp, "    parent: SPACER_LF\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0.0 ]\n", (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, 0.0 ]\n", dspins[FFL_WBS]->value(), (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ %lf, -%lf, 0.0 ]\n", dspins[FFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerFCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_RF\n");
        fprintf(fp, "    parent: SPACER_RF\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0.0 ]\n", (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketFCommon\n");
    }

    if(checks[RFL_CHK]->isChecked()) {
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_LR\n");
        fprintf(fp, "    translation: [ -%lf, %lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, %lf ]\n",
                dspins[TRK_WBS]->value() / 2.0,
                (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SpacerCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: TRACK_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0 ]\n", (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
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
        fprintf(fp, "    translation: [ 0.0, -%lf, 0 ]\n", (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
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
        fprintf(fp, "    translation: [ -%lf, %lf, 0.0 ]\n", dspins[RFL_WBS]->value(), (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ -%lf, %lf, 0.0 ]\n", dspins[RFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_LR\n");
        fprintf(fp, "    parent: SPACER_LR\n");
        fprintf(fp, "    translation: [ 0.0, %lf, 0.0 ]\n", (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: IDLER_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, 0.0 ]\n", dspins[RFL_WBS]->value(), (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *IdlerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: ROLLER_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ -%lf, -%lf, 0.0 ]\n", dspins[RFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *RollerRCommon\n");
        fprintf(fp, "  -\n");
        fprintf(fp, "    name: SPROCKET_RR\n");
        fprintf(fp, "    parent: SPACER_RR\n");
        fprintf(fp, "    translation: [ 0.0, -%lf, 0.0 ]\n", (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0);
        fprintf(fp, "    jointId: %d\n", id++);
        fprintf(fp, "    <<: *SprocketRCommon\n");
    }
    fclose(fp);
}


void CrawlerRobotBuilderDialogImpl::onColorChanged(PushButton* pushbutton)
{
    QColor selectedColor;
    QColor currentColor = pushbutton->palette().color(QPalette::Button);
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
    pushbutton->setPalette(palette);
}


void CrawlerRobotBuilderDialogImpl::setColor(PushButton* pushbutton, const Vector3& color)
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


string CrawlerRobotBuilderDialogImpl::getSaveFilename(FileDialog& dialog, const string& suffix)
{
    string filename;
    auto filenames = dialog.selectedFiles();
    if(!filenames.isEmpty()){
        filename = filenames.front().toStdString();
        filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != suffix){
            filename += suffix;
        }
    }
    return filename;
}
