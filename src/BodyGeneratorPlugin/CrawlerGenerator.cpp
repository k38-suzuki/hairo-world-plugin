/**
   \file
   \author Kenta Suzuki
*/

#include "CrawlerGenerator.h"
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
#include <cnoid/NullOut>
#include <cnoid/ProjectManager>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/Widget>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <cmath>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

CrawlerGenerator* cgeneratorInstance = nullptr;

void putKeyVector3(YAMLWriter& writer, const string key, const Vector3 value)
{
    writer.putKey(key);
    writer.startFlowStyleListing();
    for(int i = 0; i < 3; ++i) {
        writer.putScalar(value[i]);
    }
    writer.endListing();
}

VectorXd calcBoxInertia(double mass, double x, double y, double z)
{
    VectorXd inertia;
    inertia.resize(9);
    double ixx = mass * (y * y + z * z) / 12.0;
    double iyy = mass * (z * z + x * x) / 12.0;
    double izz = mass * (x * x + y * y) / 12.0;
    inertia << ixx, 0.0, 0.0, 0.0, iyy, 0.0, 0.0, 0.0, izz;
    return inertia;
}

VectorXd calcCylinderInertia(double mass, double radius, double height)
{
    VectorXd inertia;
    inertia.resize(9);
    double i = mass * (3.0 * radius * radius + height * height) / 12.0;
    double iyy = mass * radius * radius / 2.0;
    inertia << i, 0.0, 0.0, 0.0, iyy, 0.0, 0.0, 0.0, i;
    return inertia;
}

struct CheckInfo {
    int row;
    int column;
    bool checked;
};

CheckInfo checkInfo[] = {
    { 0, 0,  true },
    { 0, 1,  true },
    { 0, 2, false }
};

struct ButtonInfo {
    int row;
    int column;
    double red;
    double green;
    double blue;
};

ButtonInfo buttonInfo[] = {
    {  2, 3,   0.0 / 255.0, 153.0 / 255.0,  0.0 / 255.0 },
    {  5, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    {  9, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    { 13, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0 },
    { 17, 3, 255.0 / 255.0,   0.0 / 255.0,  0.0 / 255.0 }
};

struct DoubleSpinInfo {
    int row;
    int column;
    double value;
    double min;
    double max;
    int decimals;
    bool enabled;
};

DoubleSpinInfo doubleSpinInfo[] = {
    {  2, 1,  8.000, 0.0, 1000.000, 3,  true }, {  3, 1,  0.450, 0.0, 1000.000, 3,  true }, {  3, 2,  0.300, 0.0, 1000.000, 3,  true }, {  3, 3,  0.100, 0.0, 1000.000, 3,  true },
    {  5, 1,  1.000, 0.0, 1000.000, 3,  true }, {  6, 1,  0.080, 0.0, 1000.000, 3,  true }, {  7, 1,  0.100, 0.0, 1000.000, 3,  true }, {  7, 3,  0.420, 0.0, 1000.000, 3,  true },
    {  9, 1,  0.250, 0.0, 1000.000, 3,  true }, { 10, 1,  0.080, 0.0, 1000.000, 3,  true }, { 10, 2,  0.080, 0.0, 1000.000, 3,  true }, { 11, 1,  0.080, 0.0, 1000.000, 3,  true }, { 11, 3,  0.130, 0.0, 1000.000, 3,  true },
    { 13, 1,  0.250, 0.0, 1000.000, 3,  true }, { 14, 1,  0.080, 0.0, 1000.000, 3,  true }, { 14, 2,  0.080, 0.0, 1000.000, 3,  true }, { 15, 1,  0.080, 0.0, 1000.000, 3,  true }, { 15, 3,  0.130, 0.0, 1000.000, 3,  true },
    { 17, 1,  0.200, 0.0, 1000.000, 3,  true }, { 18, 1,  0.060, 0.0, 1000.000, 3,  true }, { 19, 1,  0.013, 0.0, 1000.000, 3,  true }
};

//DoubleSpinInfo agxdoubleSpinInfo[] = {
//    {  1, 4,  0.010, 0.0, 1000.000, 3, false }, {  2, 1,  0.090, 0.0, 1000.000, 3, false }, {  2, 4,  0.020,       0.0, 1000.000, 3, false }, {  3, 4,  2.000,       0.0, 1000.000, 3, false }, {  4, 1,  1.000, 0.0, 1000.000, 3, false },
//    {  5, 1,  9.000, 0.0, 1000.000, 3, false }, {  5, 4,  0.010, 0.0, 1000.000, 3, false }, {  6, 1, -0.001, -1000.000, 1000.000, 3, false }, {  6, 4, -0.009, -1000.000, 1000.000, 3, false },
//    {  8, 4,  0.010, 0.0, 1000.000, 3, false }, {  9, 1,  0.090, 0.0, 1000.000, 3, false }, {  9, 4,  0.020,       0.0, 1000.000, 3, false }, { 10, 4,  2.000,       0.0, 1000.000, 3, false }, { 11, 1,  1.000, 0.0, 1000.000, 3, false },
//    { 12, 1,  9.000, 0.0, 1000.000, 3, false }, { 12, 4,  0.010, 0.0, 1000.000, 3, false }, { 13, 1, -0.001, -1000.000, 1000.000, 3, false }, { 13, 4, -0.009, -1000.000, 1000.000, 3, false }
//};

DoubleSpinInfo agxdoubleSpinInfo[] = {
    {  1, 4,  0.010, 0.0, 1000.000, 3, false }, {  2, 1,  0.100, 0.0, 1000.000, 3, false }, {  2, 4,  0.020,       0.0, 1000.000, 3, false }, {  3, 4,  9.000,       0.0, 1000.000, 3, false }, {  4, 1,  4.000, 0.0, 1000.000, 3, false },
    {  5, 1,  9.000, 0.0, 1000.000, 3, false }, {  5, 4,  0.010, 0.0, 1000.000, 3, false }, {  6, 1, -0.001, -1000.000, 1000.000, 3, false }, {  6, 4, -0.009, -1000.000, 1000.000, 3, false },
    {  8, 4,  0.010, 0.0, 1000.000, 3, false }, {  9, 1,  0.080, 0.0, 1000.000, 3, false }, {  9, 4,  0.020,       0.0, 1000.000, 3, false }, { 10, 4,  9.000,       0.0, 1000.000, 3, false }, { 11, 1,  4.000, 0.0, 1000.000, 3, false },
    { 12, 1,  9.000, 0.0, 1000.000, 3, false }, { 12, 4,  0.010, 0.0, 1000.000, 3, false }, { 13, 1, -0.001, -1000.000, 1000.000, 3, false }, { 13, 4, -0.009, -1000.000, 1000.000, 3, false }
};

struct SpinInfo {
    int row;
    int column;
    double value;
    double min;
    double max;
    bool enabled;
};

//SpinInfo agxspinInfo[] = {
//    {  1, 1, 42, 0, 9999, false }, {  3, 1,   3, 0, 9999, false }, {  3, 5,  4, 0, 9999, false },
//    {  4, 2,  6, 0, 9999, false }, {  4, 4, 100, 0, 9999, false }, {  5, 2, 10, 0, 9999, false },
//    {  8, 1, 42, 0, 9999, false }, { 10, 1,   3, 0, 9999, false }, { 10, 5,  4, 0, 9999, false },
//    { 11, 2,  6, 0, 9999, false }, { 11, 4, 100, 0, 9999, false }, { 12, 2, 10, 0, 9999, false }
//};

SpinInfo agxspinInfo[] = {
    {  1, 1, 42, 0, 9999, false }, {  3, 1,   3, 0, 9999, false }, {  3, 5,  4, 0, 9999, false },
    {  4, 2,  7, 0, 9999, false }, {  4, 4, 100, 0, 9999, false }, {  5, 2,  7, 0, 9999, false },
    {  8, 1, 42, 0, 9999, false }, { 10, 1,   3, 0, 9999, false }, { 10, 5,  4, 0, 9999, false },
    { 11, 2,  7, 0, 9999, false }, { 11, 4, 100, 0, 9999, false }, { 12, 2,  7, 0, 9999, false }
};

struct LabelInfo {
    int row;
    int column;
};

LabelInfo labelInfo[] = {
    {  2, 0 }, {  2, 2 }, {  3, 0 },
    {  5, 0 }, {  5, 2 }, {  6, 0 }, {  7, 0 }, {  7, 2 },
    {  9, 0 }, {  9, 2 }, { 10, 0 }, { 11, 0 }, { 11, 2 },
    { 13, 0 }, { 13, 2 }, { 14, 0 }, { 15, 0 }, { 15, 2 },
    { 17, 0 }, { 17, 2 }, { 18, 0 }, { 19, 0 }
};

LabelInfo agxlabelInfo[] = {
    {  1, 0 }, {  1, 3 },
    {  2, 0 }, {  2, 3 },
    {  3, 0 }, {  3, 3 },
    {  4, 0 }, {  4, 3 },
    {  5, 0 }, {  5, 3 },
    {  6, 0 }, {  6, 3 },
    {  8, 0 }, {  8, 3 },
    {  9, 0 }, {  9, 3 },
    { 10, 0 }, { 10, 3 },
    { 11, 0 }, { 11, 3 },
    { 12, 0 }, { 12, 3 },
    { 13, 0 }, { 13, 3 }
};

struct Info {
    int row;
    int column;
};

Info separatorInfo[] = {
    { 1, 0 }, { 4, 0 }, { 8, 0 }, { 12, 0 }, { 16, 0 }
};

Info agxseparatorInfo[] = {
    { 0, 0 }, { 7, 0 }
};

}

namespace cnoid {

class CrawlerGeneratorImpl : public Dialog
{
public:
    CrawlerGeneratorImpl(CrawlerGenerator* self);
    CrawlerGenerator* self;

    enum DoubleSpinId {
        CHS_MAS, CHS_XSZ, CHS_YSZ, CHS_ZSZ,
        TRK_MAS, TRK_RAD, TRK_WDT, TRK_WBS,
        FFL_MAS, FFL_FRD, FFL_RRD, FFL_WDT, FFL_WBS,
        RFL_MAS, RFL_FRD, RFL_RRD, RFL_WDT, RFL_WBS,
        SPC_MAS, SPC_RAD, SPC_WDT,
        NUM_DSPINS
    };
    enum AGXDoubleSpinId {
        TRK_BNT, TRK_BNW, TRK_BNTT, TRK_BNDTM, TRK_BSHFPM,
        TRK_BHCM, TRK_BHSD, TRK_BNWMT, TRK_BNWST,
        FLP_BNT, FLP_BNW, FLP_BNTT, FLP_BNDTM, FLP_BSHFPM,
        FLP_BHCM, FLP_BHSD, FLP_BNWMT, FLP_BNWST,
        NUM_AGXDSPINS
    };
    enum SpinId {
        TRK_BNN, TRK_BUTNE, TRK_BNDTE,
        TRK_BSHFPE, TRK_BMSHNF, TRK_BHCE,
        FLP_BNN, FLP_BUTNE, FLP_BNDTE,
        FLP_BSHFPE, FLP_BMSHNF, FLP_BHCE,
        NUM_SPINS
    };
    enum ButtonId {
        CHS_CLR, TRK_CLR, FFL_CLR,
        RFL_CLR, SPC_CLR, NUM_BUTTONS
    };
    enum CheckId { FFL_CHK, RFL_CHK, AGX_CHK, NUM_CHECKS };
    enum DialogButtonId { RESET, IMPORT, EXPORT, NUMTBUTTONS };

    CheckBox* checks[NUM_CHECKS];
    PushButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* dspins[NUM_DSPINS];
    DoubleSpinBox* agxdspins[NUM_AGXDSPINS];
    SpinBox* agxspins[NUM_SPINS];
    PushButton* toolButtons[NUMTBUTTONS];
    FileFormWidget* formWidget;
    string bodyname;
    YAMLWriter yamlWriter;
    YAMLWriter yamlWriter2;

    bool save(const string& filename);
    bool save2(const string& filename);

    void initialize();
    void onResetButtonClicked();
    void onExportButtonClicked();
    void onImportButtonClicked();

    bool load2(const string& filename, ostream& os = nullout());
    void onEnableAGXCheckToggled(const bool& on);
    void onColorChanged(PushButton* pushbutton);

    void setColor(PushButton* pushbutton, const Vector3& color);
    Vector3 extractColor(PushButton* colorButton);

    MappingPtr writeBody(const string& filename);
    void writeLink(Listing* linksNode);
    MappingPtr writeChassis();
    void writeSpacer(Listing* linksNode);
    void writeAISTTrack(Listing* linksNode);
    void writeAISTTrackF(Listing* linksNode);
    void writeAISTTrackR(Listing* linksNode);

    MappingPtr writeConfig(const string& filename);

    MappingPtr writeAGXTrack();
    MappingPtr writeAGXTrackBelt();
    MappingPtr writeAGXSprocket();
    MappingPtr writeAGXRoller();
    MappingPtr writeAGXIdler();
    MappingPtr writeAGXSubTrackF();
    MappingPtr writeAGXSubTrackR();
    MappingPtr writeAGXSubTrackBelt();
    MappingPtr writeAGXSprocketF();
    MappingPtr writeAGXRollerF();
    MappingPtr writeAGXIdlerF();
    MappingPtr writeAGXSprocketR();
    MappingPtr writeAGXRollerR();
    MappingPtr writeAGXIdlerR();
    MappingPtr writeAGXWheel();

    bool writeAGX(const string& filename);

    bool writeAGXTrack(YAMLWriter& writer);
    bool writeAGXTrackBelt(YAMLWriter& writer);
    bool writeAGXSprocket(YAMLWriter& writer);
    bool writeAGXRoller(YAMLWriter& writer);
    bool writeAGXIdler(YAMLWriter& writer);
    bool writeAGXSpacer(YAMLWriter& writer);
    bool writeAGXSubTrackF(YAMLWriter& writer);
    bool writeAGXSubTrackR(YAMLWriter& writer);
    bool writeAGXSubTrackBelt(YAMLWriter& writer);
    bool writeAGXSprocketF(YAMLWriter& writer);
    bool writeAGXRollerF(YAMLWriter& writer);
    bool writeAGXIdlerF(YAMLWriter& writer);
    bool writeAGXSprocketR(YAMLWriter& writer);
    bool writeAGXRollerR(YAMLWriter& writer);
    bool writeAGXIdlerR(YAMLWriter& writer);
    bool writeAGXWheel(YAMLWriter& writer);
};

}


CrawlerGenerator::CrawlerGenerator()
{
    impl = new CrawlerGeneratorImpl(this);
}


CrawlerGeneratorImpl::CrawlerGeneratorImpl(CrawlerGenerator* self)
    : self(self)
{
    setWindowTitle(_("CrawlerRobot Builder"));
    yamlWriter.setKeyOrderPreservationMode(true);
    yamlWriter2.setKeyOrderPreservationMode(true);

    QGridLayout* gbox = new QGridLayout;
    QGridLayout* agbox = new QGridLayout;

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        gbox->addWidget(dspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_AGXDSPINS; ++i) {
        DoubleSpinInfo info = agxdoubleSpinInfo[i];
        agxdspins[i] = new DoubleSpinBox;
        DoubleSpinBox* agxdspin = agxdspins[i];
        agbox->addWidget(agxdspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        agxspins[i] = new SpinBox;
        SpinInfo info = agxspinInfo[i];
        SpinBox* spin = agxspins[i];
        agbox->addWidget(spin, info.row, info.column);
    }

    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton;
        PushButton* button = buttons[i];
        gbox->addWidget(button, info.row, info.column);
        button->sigClicked().connect([&, button](){ onColorChanged(button); });
    }

    static const char* clabels[] = { _("Front SubTrack"), _("Rear SubTrack"), _("AGX") };

    QHBoxLayout* chbox = new QHBoxLayout;
    for(int i = 0; i < NUM_CHECKS; ++i) {
        CheckInfo info = checkInfo[i];
        checks[i] = new CheckBox;
        CheckBox* check = checks[i];
        check->setText(clabels[i]);
        chbox->addWidget(check);
    }
    gbox->addLayout(chbox, 0, 0, 1, 4);

    static const char* hlabels[] = {
        _("Chassis"), _("Track"), _("Front SubTrack"), _("Rear SubTrack"), _("Spacer")
    };

    for(int i = 0; i < 5; ++i) {
        Info info = separatorInfo[i];
        gbox->addLayout(new HSeparatorBox(new QLabel(hlabels[i])), info.row, info.column, 1, 4);
    }

    static const char* dlabels[] = {
        _("mass [kg]"), _("color"), _("size(x-y-z) [m, m, m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]")
    };

    for(int i = 0; i < 22; ++i) {
        LabelInfo info = labelInfo[i];
        gbox->addWidget(new QLabel(dlabels[i]), info.row, info.column);
    }

    static const char* agxhlabels[] = { _("Track Belt"), _("SubTrack Belt") };

    for(int i = 0; i < 2; ++i) {
        Info info = agxseparatorInfo[i];
        agbox->addLayout(new HSeparatorBox(new QLabel(agxhlabels[i])), info.row, info.column, 1, 6);
    }

    static const char* agxdlabels[] = {
        _("number of nodes [-]"), _("node thickness [m]"),
        _("node width [m]"), _("node thickerthickness [m]"),
        _("use thicker node every [-]"), _("node distance tension [m, e-]"),
        _("stabilizing hinge friction parameter [-, e-]"), _("min stabilizing hinge normal force [N]"),
        _("hinge compliance [rad/Nm, e-]"), _("hinge spook damping [s]"),
        _("nodes to wheels merge threshold [-]"), _("nodes to wheels split threshold [-]")
    };

    for(int i = 0; i < 24; ++i) {
        LabelInfo info = agxlabelInfo[i];
        agbox->addWidget(new QLabel(agxdlabels[i % 12]), info.row, info.column);
    }

    static const char* tlabels[] = { _("&Reset"), _("&Import"), _("&Export") };

    QHBoxLayout* thbox = new QHBoxLayout;
    thbox->addStretch();
    for(int i = 0; i < NUMTBUTTONS; ++i) {
        toolButtons[i] = new PushButton(tlabels[i]);
        PushButton* button = toolButtons[i];
        thbox->addWidget(button);
    }

    formWidget = new FileFormWidget;

    initialize();
    bodyname.clear();

    QVBoxLayout* vbox = new QVBoxLayout;
    QHBoxLayout* hbox = new QHBoxLayout;
    vbox->addLayout(thbox);
    hbox->addLayout(gbox);
//    hbox->addLayout(agbox);
    vbox->addLayout(hbox);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    toolButtons[RESET]->sigClicked().connect([&](){ onResetButtonClicked(); });
    toolButtons[IMPORT]->sigClicked().connect([&](){ onImportButtonClicked(); });
    toolButtons[EXPORT]->sigClicked().connect([&](){ onExportButtonClicked(); });
    checks[AGX_CHK]->sigToggled().connect([&](bool on){ onEnableAGXCheckToggled(on); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


CrawlerGenerator::~CrawlerGenerator()
{
    delete impl;
}


void CrawlerGenerator::initializeClass(ExtensionManager* ext)
{
    if(!cgeneratorInstance) {
        cgeneratorInstance = ext->manage(new CrawlerGenerator);
    }

    MenuManager& mm = ext->menuManager().setPath("/" N_("Tools")).setPath(_("BodyGenerator"));
    mm.addItem(_("CrawlerRobot"))->sigTriggered().connect(
                [&](){ cgeneratorInstance->impl->show(); });
}


bool CrawlerGeneratorImpl::save(const string& filename)
{
    if(!filename.empty()) {
        filesystem::path path(filename);
        bodyname = path.stem().string();

        if(checks[AGX_CHK]->isChecked()) {
            writeAGX(filename);
        } else {
            auto topNode = writeBody(filename);
            if(yamlWriter.openFile(filename)) {
                yamlWriter.putNode(topNode);
                yamlWriter.closeFile();
            }            
        }
    }

    return true;
}


bool CrawlerGeneratorImpl::save2(const string& filename)
{
    if(!filename.empty()) {
        auto topNode = writeConfig(filename);
        if(yamlWriter2.openFile(filename)) {
            yamlWriter2.putNode(topNode);
            yamlWriter2.closeFile();
        }
    }

    return true;
}


void CrawlerGeneratorImpl::initialize()
{
    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        DoubleSpinBox* dspin = dspins[i];
        dspin->setDecimals(info.decimals);
        dspin->setRange(info.min, info.max);
        dspin->setValue(info.value);
        dspin->setEnabled(info.enabled);
    }

    for(int i = 0; i < NUM_AGXDSPINS; ++i) {
        DoubleSpinInfo info = agxdoubleSpinInfo[i];
        DoubleSpinBox* agxdspin = agxdspins[i];
        agxdspin->setDecimals(info.decimals);
        agxdspin->setRange(info.min, info.max);
        agxdspin->setValue(info.value);
        agxdspin->setEnabled(info.enabled);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        SpinInfo info = agxspinInfo[i];
        SpinBox* spin = agxspins[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
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


void CrawlerGeneratorImpl::onResetButtonClicked()
{
    initialize();
}


void CrawlerGeneratorImpl::onImportButtonClicked()
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
        load2(filename);
    }
}


bool CrawlerGeneratorImpl::load2(const string& filename,std::ostream& os )
{
    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& configList = *node->findListing("configs");
            if(configList.isValid()) {
                for(int i = 0; i < configList.size(); i++) {
                    Mapping* info = configList[i].toMapping();

                    auto& doubleSpinList = *info->findListing("doubleSpin");
                    if(doubleSpinList.isValid()) {
                        for(int j = 0; j < doubleSpinList.size(); ++j) {
                            double value = doubleSpinList[j].toDouble();
                            dspins[j]->setValue(value);
                        }
                    }

                    auto& spinList = *info->findListing("spin");
                    if(spinList.isValid()) {
                        for(int j = 0; j < spinList.size(); ++j) {
                            int value = spinList[j].toInt();
                            agxspins[j]->setValue(value);
                        }
                    }

                    for(int j = 0; j < NUM_BUTTONS; ++j) {
                        Vector3 color;
                        string key = "button" + to_string(j);
                        if(read(info, key, color)) {
                            setColor(buttons[j], color);
                        }
                    }

                    auto& checkList = *info->findListing("check");
                    if(checkList.isValid()) {
                        for(int j = 0; j < checkList.size(); ++j) {
                            bool checked = checkList[j].toInt() == 0 ? false : true;
                            checks[j]->setChecked(checked);
                        }
                    }
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    return true;
}


void CrawlerGeneratorImpl::onExportButtonClicked()
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

    if(dialog.exec() == QDialog::Accepted) {
        string filename = dialog.selectedFiles().front().toStdString();
        filesystem::path path(filename);
        string ext = path.extension().string();
        if(ext.empty()) {
            filename += ".yaml";
        }
        save2(filename);
    }
}


void CrawlerGeneratorImpl::onEnableAGXCheckToggled(const bool& on)
{
    agxdspins[TRK_BNT]->setEnabled(on);
    agxdspins[TRK_BNW]->setEnabled(on);
    agxdspins[TRK_BNTT]->setEnabled(on);
    agxdspins[TRK_BNDTM]->setEnabled(on);
    agxdspins[TRK_BSHFPM]->setEnabled(on);

    agxdspins[TRK_BHCM]->setEnabled(on);
    agxdspins[TRK_BHSD]->setEnabled(on);
    agxdspins[TRK_BNWMT]->setEnabled(on);
    agxdspins[TRK_BNWST]->setEnabled(on);

    agxdspins[FLP_BNT]->setEnabled(on);
    agxdspins[FLP_BNW]->setEnabled(on);
    agxdspins[FLP_BNTT]->setEnabled(on);
    agxdspins[FLP_BNDTM]->setEnabled(on);
    agxdspins[FLP_BSHFPM]->setEnabled(on);

    agxdspins[FLP_BHCM]->setEnabled(on);
    agxdspins[FLP_BHSD]->setEnabled(on);
    agxdspins[FLP_BNWMT]->setEnabled(on);
    agxdspins[FLP_BNWST]->setEnabled(on);

    agxspins[TRK_BNN]->setEnabled(on);
    agxspins[TRK_BUTNE]->setEnabled(on);
    agxspins[TRK_BNDTE]->setEnabled(on);
    agxspins[TRK_BSHFPE]->setEnabled(on);
    agxspins[TRK_BMSHNF]->setEnabled(on);
    agxspins[TRK_BHCE]->setEnabled(on);

    agxspins[FLP_BNN]->setEnabled(on);
    agxspins[FLP_BUTNE]->setEnabled(on);
    agxspins[FLP_BNDTE]->setEnabled(on);
    agxspins[FLP_BSHFPE]->setEnabled(on);
    agxspins[FLP_BMSHNF]->setEnabled(on);
    agxspins[FLP_BHCE]->setEnabled(on);
}


void CrawlerGeneratorImpl::onColorChanged(PushButton* pushbutton)
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


void CrawlerGeneratorImpl::setColor(PushButton* pushbutton, const Vector3& color)
{
    QColor selectedColor;
    selectedColor.setRed(color[0] * 255.0);
    selectedColor.setGreen(color[1] * 255.0);
    selectedColor.setBlue(color[2] * 255.0);
    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    pushbutton->setPalette(palette);
}


Vector3 CrawlerGeneratorImpl::extractColor(PushButton* colorButton)
{
    QColor selectedColor = colorButton->palette().color(QPalette::Button);
    return Vector3(selectedColor.red() / 255.0, selectedColor.green() / 255.0, selectedColor.blue() / 255.0);
}


MappingPtr CrawlerGeneratorImpl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(filename);
    string name = path.stem().string();

    node->write("format", "ChoreonoidBody");
    node->write("formatVersion", "1.0");
    node->write("angleUnit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;
    writeLink(linksNode);
    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
}


void CrawlerGeneratorImpl::writeLink(Listing* linksNode)
{
    bool isAGXChecked = checks[AGX_CHK]->isChecked();

    linksNode->append(writeChassis());
    writeSpacer(linksNode);

    if(checks[FFL_CHK]->isChecked()) {
        if(isAGXChecked) {

        } else {
            writeAISTTrackF(linksNode);
        }
    }

    if(checks[RFL_CHK]->isChecked()) {
        if(isAGXChecked) {

        } else {
            writeAISTTrackR(linksNode);
        }
    }

    if(isAGXChecked) {

    } else {
        writeAISTTrack(linksNode);
    }
}


MappingPtr CrawlerGeneratorImpl::writeChassis()
{
    MappingPtr chassisNode = new Mapping;

    chassisNode->write("name", "CHASSIS");
    write(chassisNode, "translation", Vector3(0.0, 0.0, 0.0));
    chassisNode->write("jointType", "free");
    write(chassisNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    chassisNode->write("mass", dspins[CHS_MAS]->value());
    write(chassisNode, "inertia", calcBoxInertia(dspins[CHS_MAS]->value(), dspins[CHS_XSZ]->value(), dspins[CHS_YSZ]->value(), dspins[CHS_ZSZ]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Box");
    write(geometryNode, "size", Vector3(dspins[CHS_XSZ]->value(), dspins[CHS_YSZ]->value(), dspins[CHS_ZSZ]->value()));

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[CHS_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        chassisNode->insert("elements", elementsNode);
    }

    return chassisNode;
}


void CrawlerGeneratorImpl::writeSpacer(Listing* linksNode)
{
    MappingPtr spacerCommonNode = new Mapping;

    spacerCommonNode->write("parent", "CHASSIS");
    spacerCommonNode->write("jointType", "revolute");
    spacerCommonNode->write("jointAxis", "-Y");
    write(spacerCommonNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    spacerCommonNode->write("mass", dspins[SPC_MAS]->value());
    write(spacerCommonNode, "inertia", calcCylinderInertia(dspins[SPC_MAS]->value(), dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[SPC_RAD]->value());
    geometryNode->write("height", dspins[SPC_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[SPC_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        spacerCommonNode->insert("elements", elementsNode);
    }

    if(checks[FFL_CHK]->isChecked()) {
        MappingPtr spacerLFNode = new Mapping;

        spacerLFNode->write("name", "SPACER_LF");
        write(spacerLFNode, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0));
        spacerLFNode->write("jointId", 0);
        spacerLFNode->insert(spacerCommonNode);

        linksNode->append(spacerLFNode);

        MappingPtr spacerRFNode = new Mapping;

        spacerRFNode->write("name", "SPACER_RF");
        write(spacerRFNode, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0));
        spacerRFNode->write("jointId", 1);
        spacerRFNode->insert(spacerCommonNode);

        linksNode->append(spacerRFNode);
    }

    if(checks[RFL_CHK]->isChecked()) {
        MappingPtr spacerLRNode = new Mapping;

        spacerLRNode->write("name", "SPACER_LR");
        write(spacerLRNode, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0, (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0));
        spacerLRNode->write("jointId", 2);
        spacerLRNode->insert(spacerCommonNode);

        linksNode->append(spacerLRNode);

        MappingPtr spacerRRNode = new Mapping;

        spacerRRNode->write("name", "SPACER_RR");
        write(spacerRRNode, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0));
        spacerRRNode->write("jointId", 3);
        spacerRRNode->insert(spacerCommonNode);

        linksNode->append(spacerRRNode);
    }
}


void CrawlerGeneratorImpl::writeAISTTrack(Listing* linksNode)
{
    MappingPtr trackCommonNode = new Mapping;
    trackCommonNode->write("parent", "CHASSIS");
    trackCommonNode->write("jointType", "pseudo_continuous_track");
    trackCommonNode->write("jointAxis", "Y");
    write(trackCommonNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackCommonNode->write("mass", dspins[TRK_MAS]->value());
    write(trackCommonNode, "inertia", calcBoxInertia(dspins[TRK_MAS]->value(), dspins[TRK_WBS]->value(), dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");

    int numPoints = 5;
    int pitch = numPoints - 1;
    double pitchAngle = 180.0 / (double)pitch;
    int n = numPoints * 4 + 2;
    for(int i = 0; i < numPoints; ++i) {
        crossSectionList.append(dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[TRK_RAD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; ++i) {
        crossSectionList.append(-dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[TRK_RAD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; ++i) {
        crossSectionList.append(dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[TRK_RAD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -dspins[TRK_WDT]->value() / 2.0, 0.0, 0.0, dspins[TRK_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[TRK_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackCommonNode->insert("elements", elementsNode);
    }

    MappingPtr trackLNode = new Mapping;

    trackLNode->write("name", "TRACK_L");
    write(trackLNode, "translation", Vector3(0.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0));
    trackLNode->insert(trackCommonNode);

    linksNode->append(trackLNode);

    MappingPtr trackRNode = new Mapping;

    trackRNode->write("name", "TRACK_R");
    write(trackRNode, "translation", Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0));
    trackRNode->insert(trackCommonNode);

    linksNode->append(trackRNode);
}


void CrawlerGeneratorImpl::writeAISTTrackF(Listing* linksNode)
{
    MappingPtr trackCommonNode = new Mapping;

    trackCommonNode->write("jointType", "pseudo_continuous_track");
    trackCommonNode->write("jointAxis", "Y");
    write(trackCommonNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackCommonNode->write("mass", dspins[FFL_MAS]->value());
    double radius = std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value());
    write(trackCommonNode, "inertia", calcBoxInertia(dspins[FFL_MAS]->value(), dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), radius * 2.0));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");

    int numPoints = 5;
    int pitch = numPoints - 1;
    double pitchAngle = 180.0 / (double)pitch;
    int n = numPoints * 4 + 2;
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[FFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(-dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_RRD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[FFL_RRD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; i++) {
        crossSectionList.append(dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[FFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -dspins[FFL_WDT]->value() / 2.0, 0.0, 0.0, dspins[FFL_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[FFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackCommonNode->insert("elements", elementsNode);
    }

    MappingPtr trackLFNode = new Mapping;

    trackLFNode->write("name", "TRACK_LF");
    trackLFNode->write("parent", "SPACER_LF");
    write(trackLFNode, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0));
    trackLFNode->insert(trackCommonNode);

    linksNode->append(trackLFNode);

    MappingPtr trackRFNode = new Mapping;

    trackRFNode->write("name", "TRACK_RF");
    trackRFNode->write("parent", "SPACER_RF");
    write(trackRFNode, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0));
    trackRFNode->insert(trackCommonNode);

    linksNode->append(trackRFNode);
}


void CrawlerGeneratorImpl::writeAISTTrackR(Listing* linksNode)
{
    MappingPtr trackCommonNode = new Mapping;

    trackCommonNode->write("jointType", "pseudo_continuous_track");
    trackCommonNode->write("jointAxis", "Y");
    write(trackCommonNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackCommonNode->write("mass", dspins[RFL_MAS]->value());
    double radius = std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value());
    write(trackCommonNode, "inertia", calcBoxInertia(dspins[RFL_MAS]->value(), dspins[RFL_WBS]->value(), dspins[RFL_WDT]->value(), radius * 2.0));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = new Mapping;
    geometryNode->write("type", "Extrusion");
    Listing& crossSectionList = *geometryNode->createFlowStyleListing("crossSection");

    int numPoints = 5;
    int pitch = numPoints - 1;
    double pitchAngle = 180.0 / (double)pitch;
    int n = numPoints * 4 + 2;
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[RFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(-dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_RRD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[RFL_RRD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; i++) {
        crossSectionList.append(dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(dspins[RFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -dspins[RFL_WDT]->value() / 2.0, 0.0, 0.0, dspins[RFL_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[RFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackCommonNode->insert("elements", elementsNode);
    }

    MappingPtr trackLRNode = new Mapping;

    trackLRNode->write("name", "TRACK_LR");
    trackLRNode->write("parent", "SPACER_LR");
    write(trackLRNode, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0, (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0));
    trackLRNode->insert(trackCommonNode);

    linksNode->append(trackLRNode);

    MappingPtr trackRRNode = new Mapping;

    trackRRNode->write("name", "TRACK_RR");
    trackRRNode->write("parent", "SPACER_RR");
    write(trackRRNode, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0));
    trackRRNode->insert(trackCommonNode);

    linksNode->append(trackRRNode);
}


MappingPtr CrawlerGeneratorImpl::writeConfig(const string& filename)
{
    MappingPtr node = new Mapping;

    filesystem::path path(filename);
    string name = path.stem().string();

    node->write("format", "CrawlerRobotBuilderYaml");
    node->write("formatVersion", "1.0");
    node->write("name", name);

    ListingPtr configsNode = new Listing;
    {
        MappingPtr node = new Mapping;

        Listing& doubleSpinList = *node->createFlowStyleListing("doubleSpin");
        int n = NUM_DSPINS;
        for(int i = 0; i < NUM_DSPINS; ++i) {
            doubleSpinList.append(dspins[i]->value()), n, n;
        }

        Listing& spinList = *node->createFlowStyleListing("spin");
        int n1 = NUM_SPINS;
        for(int i = 0; i < NUM_SPINS; ++i) {
            spinList.append(agxspins[i]->value(), n1, n1);
        }

        int n2 = NUM_BUTTONS;
        for(int i = 0; i < NUM_BUTTONS; ++i) {
            string key = "button" + to_string(i);
            write(node, key, extractColor(buttons[i]));
        }

        Listing& checkList = *node->createFlowStyleListing("check");
        int n3 = NUM_CHECKS;
        for(int i = 0; i < NUM_CHECKS; ++i) {
            checkList.append(checks[i]->isChecked() ? 1 : 0, n3, n3);
        }

        configsNode->append(node);
    }

    if(!configsNode->empty()) {
        node->insert("configs", configsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXTrack()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[TRK_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[TRK_MAS]->value() / 3.0, dspins[TRK_WBS]->value(), dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0));

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxspins[TRK_BNN]->value());
    node->write("nodeThickness", agxdspins[TRK_BNT]->value());
    node->write("nodeWidth", agxdspins[TRK_BNW]->value());
    node->write("nodeThickerThickness", agxdspins[TRK_BNTT]->value());
    node->write("useThickerNodeEvery", agxspins[TRK_BUTNE]->value());
    node->write("material", "robotTracks");
    node->write("nodeDistanceTension", agxdspins[TRK_BNDTM]->value() * exp10(-agxspins[TRK_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxdspins[TRK_BSHFPM]->value() * exp10(-agxspins[TRK_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxspins[TRK_BMSHNF]->value());
    node->write("hingeCompliance", agxdspins[TRK_BHCM]->value() * exp10(-agxspins[TRK_BHCE]->value()));
    node->write("hingeSpookDamping", agxdspins[TRK_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxdspins[TRK_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxdspins[TRK_BNWST]->value());

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXSprocket()
{
    MappingPtr sprocketNode = new Mapping;

    sprocketNode->write("parent", "CHASSIS");
    sprocketNode->insert(writeAGXWheel());
    sprocketNode->write("mass", dspins[TRK_MAS]->value() * 2.0 / 9.0);
    write(sprocketNode, "inertia", calcCylinderInertia(dspins[TRK_MAS]->value(), dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[TRK_RAD]->value());
    geometryNode->write("height", dspins[TRK_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[TRK_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        sprocketNode->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRoller()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerGeneratorImpl::writeAGXIdler()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerGeneratorImpl::writeAGXSubTrackF()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[FFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[FFL_MAS]->value() / 3.0, dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXSubTrackR()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[RFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[RFL_MAS]->value() / 3.0, dspins[RFL_WBS]->value(), dspins[RFL_WDT]->value(), std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXSubTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxspins[FLP_BNN]->value());
    node->write("nodeThickness", agxdspins[FLP_BNT]->value());
    node->write("nodeWidth", agxdspins[FLP_BNW]->value());
    node->write("nodeThickerThickness", agxdspins[FLP_BNTT]->value());
    node->write("useThickerNodeEvery", agxspins[FLP_BUTNE]->value());
    node->write("material", "robotTracks");
    node->write("nodeDistanceTension", agxdspins[FLP_BNDTM]->value() * exp10(-agxspins[FLP_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxdspins[FLP_BSHFPM]->value() * exp10(-agxspins[FLP_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxspins[FLP_BMSHNF]->value());
    node->write("hingeCompliance", agxdspins[FLP_BHCM]->value() * exp10(-agxspins[FLP_BHCE]->value()));
    node->write("hingeSpookDamping", agxdspins[FLP_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxdspins[FLP_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxdspins[FLP_BNWST]->value());

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXSprocketF()
{
    MappingPtr sprocketNode = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2spf * r2spf / totalf * dspins[FFL_MAS]->value();

    sprocketNode->insert(writeAGXWheel());
    sprocketNode->write("mass", mass);
    write(sprocketNode, "inertia", calcCylinderInertia(mass, dspins[FFL_RRD]->value(), dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[FFL_RRD]->value());
    geometryNode->write("height", dspins[FFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[FFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        sprocketNode->insert("elements", elementsNode);
    }

    return sprocketNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRollerF()
{
    MappingPtr rollerNode = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2rof * r2rof / totalf * dspins[FFL_MAS]->value();

    rollerNode->insert(writeAGXWheel());
    rollerNode->write("mass", mass);
    write(rollerNode, "inertia", calcCylinderInertia(mass, (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0, dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    geometryNode->write("height", dspins[FFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[FFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        rollerNode->insert("elements", elementsNode);
    }

    return rollerNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXIdlerF()
{
    MappingPtr idlerNode = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2idf * r2idf / totalf * dspins[FFL_MAS]->value();

    idlerNode->insert(writeAGXWheel());
    idlerNode->write("mass", mass);
    write(idlerNode, "inertia", calcCylinderInertia(mass, dspins[FFL_FRD]->value(), dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[FFL_FRD]->value());
    geometryNode->write("height", dspins[FFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[FFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        idlerNode->insert("elements", elementsNode);
    }

    return idlerNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXSprocketR()
{
    MappingPtr sprocketNode = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2spr * r2spr / totalr * dspins[RFL_MAS]->value();

    sprocketNode->insert(writeAGXWheel());
    sprocketNode->write("mass", mass);
    write(sprocketNode, "inertia", calcCylinderInertia(mass, dspins[RFL_FRD]->value(), dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[RFL_FRD]->value());
    geometryNode->write("height", dspins[RFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[RFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        sprocketNode->insert("elements", elementsNode);
    }

    return sprocketNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRollerR()
{
    MappingPtr rollerNode = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2ror * r2ror / totalr * dspins[RFL_MAS]->value();

    rollerNode->insert(writeAGXWheel());
    rollerNode->write("mass", mass);
    write(rollerNode, "inertia", calcCylinderInertia(mass, (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0, dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    geometryNode->write("height", dspins[RFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[RFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        rollerNode->insert("elements", elementsNode);
    }

    return rollerNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXIdlerR()
{
    MappingPtr idlerNode = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2idr * r2idr / totalr * dspins[RFL_MAS]->value();

    idlerNode->insert(writeAGXWheel());
    idlerNode->write("mass", mass);
    write(idlerNode, "inertia", calcCylinderInertia(mass, dspins[RFL_RRD]->value(), dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", dspins[RFL_RRD]->value());
    geometryNode->write("height", dspins[RFL_WDT]->value());

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", extractColor(buttons[RFL_CLR]));
    appearanceNode->insert("material", materialNode);

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        idlerNode->insert("elements", elementsNode);
    }

    return idlerNode;
}


MappingPtr CrawlerGeneratorImpl::writeAGXWheel()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "revolute");
    node->write("jointAxis", "Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("material", "robotWheel");

    return node;
}


bool CrawlerGeneratorImpl::writeAGX(const string& filename)
{
    YAMLWriter writer(filename);
    int jointId = 0;
    writer.startMapping(); {
        writer.putKeyValue("format", "ChoreonoidBody");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("angleUnit", "degree");
        writer.putKeyValue("name", bodyname);
        writer.putKey("links");
        writer.startListing(); {
            writer.startMapping(); {
                writer.putKeyValue("name", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0, 0.0, 0.0));
                writer.putKeyValue("jointType", "free");
                putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
                writer.putKeyValue("mass", dspins[CHS_MAS]->value());
                writer.putKey("inertia");
                writer.startFlowStyleListing(); {
                    VectorXd inertia = calcBoxInertia(dspins[CHS_MAS]->value(),
                                                      dspins[CHS_XSZ]->value(),
                                                      dspins[CHS_YSZ]->value(),
                                                      dspins[CHS_ZSZ]->value());
                    for(int i = 0; i < 9; ++i) {
                        writer.putScalar(inertia[i]);
                    }
                } writer.endListing(); // end inertia listing
                writer.putKey("elements");
                writer.startMapping(); {
                    writer.putKey("Shape");
                    writer.startMapping(); {
                        writer.putKey("geometry");
                        writer.startFlowStyleMapping(); {
                            writer.putKeyValue("type", "Box");
                            putKeyVector3(writer, "size", Vector3(dspins[CHS_XSZ]->value(),
                                                                   dspins[CHS_YSZ]->value(),
                                                                   dspins[CHS_ZSZ]->value()));
                        } writer.endMapping(); // end geometry mapping
                        writer.putKey("appearance");
                        writer.startFlowStyleMapping(); {
                            writer.putKey("material");
                            writer.startFlowStyleMapping(); {
                                putKeyVector3(writer, "diffuseColor", extractColor(buttons[CHS_CLR]));
                                putKeyVector3(writer, "specularColor", extractColor(buttons[CHS_CLR]));
                                writer.putKeyValue("shininess", 0.6);
                            } writer.endMapping(); // end material mapping
                        } writer.endMapping(); // end appearance mapping
                    } writer.endMapping(); // end shape mapping
                } writer.endMapping(); // end elements mapping
            } writer.endMapping(); // end chassis mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "TRACK_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writeAGXTrack(writer);
                writer.putKey("elements");
                writer.startListing(); {
                    writer.startMapping(); {
                        writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                        writer.putKeyValue("name", "TRACK_L");
                        writer.putKey("sprocketNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("SPROCKET_L");
                        } writer.endListing(); // end sprocketnames listing
                        writer.putKey("rollerNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("ROLLER_L");
                        } writer.endListing(); // end rollernames listing
                        writer.putKey("idlerNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("IDLER_L");
                        } writer.endListing(); // end idlernames listing
                        writeAGXTrackBelt(writer);
                    } writer.endMapping(); // end elements mapping
                } writer.endListing(); // end elements listing
            } writer.endMapping(); // end trackl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "TRACK_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writeAGXTrack(writer);
                writer.putKey("elements");
                writer.startListing(); {
                    writer.startMapping(); {
                        writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                        writer.putKeyValue("name", "TRACK_L");
                        writer.putKey("sprocketNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("SPROCKET_R");
                        } writer.endListing(); // end sprocketnames listing
                        writer.putKey("rollerNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("ROLLER_R");
                        } writer.endListing(); // end rollernames listing
                        writer.putKey("idlerNames");
                        writer.startFlowStyleListing(); {
                            writer.putString("IDLER_R");
                        } writer.endListing(); // end idlernames listing
                        writeAGXTrackBelt(writer);
                    } writer.endMapping(); // end elements mapping
                } writer.endListing(); // end elements listing
            } writer.endMapping(); // end trackr mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "SPROCKET_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXSprocket(writer);
            } writer.endMapping(); // end sprocketl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "ROLLER_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXRoller(writer);
            } writer.endMapping(); // end rollerl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "IDLER_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXIdler(writer);
            } writer.endMapping(); // end idlerl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "SPROCKET_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXSprocket(writer);
            } writer.endMapping(); // end sprocketr mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "ROLLER_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXRoller(writer);
            } writer.endMapping(); // end rollerr mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "IDLER_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAGXIdler(writer);
            } writer.endMapping(); // end idlerr mapping

            if(checks[FFL_CHK]->isChecked()) {
                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_LF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                 (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSpacer(writer);
                } writer.endMapping(); // end spacerlf

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                 -((dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value()),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSpacer(writer);
                } writer.endMapping(); // end spacerrf

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAGXSubTrackF(writer);
                    writer.putKey("elements");
                    writer.startListing(); {
                        writer.startMapping(); {
                            writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                            writer.putKeyValue("name", "TRACK_LF");
                            writer.putKey("sprocketNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("SPROCKET_LF");
                            } writer.endListing(); // end sprocketnames listing
                            writer.putKey("rollerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("ROLLER_LF");
                            } writer.endListing(); // end rollernames listing
                            writer.putKey("idlerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("IDLER_LF");
                            } writer.endListing(); // end idlernames listing
                            writeAGXSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAGXSubTrackF(writer);
                    writer.putKey("elements");
                    writer.startListing(); {
                        writer.startMapping(); {
                            writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                            writer.putKeyValue("name", "TRACK_RF");
                            writer.putKey("sprocketNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("SPROCKET_RF");
                            } writer.endListing(); // end sprocketnames listing
                            writer.putKey("rollerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("ROLLER_RF");
                            } writer.endListing(); // end rollernames listing
                            writer.putKey("idlerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("IDLER_RF");
                            } writer.endListing(); // end idlernames listing
                            writeAGXSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSprocketF(writer);
                } writer.endMapping(); // end sprocketl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXRollerF(writer);
                } writer.endMapping(); // end rollerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value(),
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXIdlerF(writer);
                } writer.endMapping(); // end idlerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSprocketF(writer);
                } writer.endMapping(); // end sprocketr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXRollerF(writer);
                } writer.endMapping(); // end rollerr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value(),
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXIdlerF(writer);
                } writer.endMapping(); // end idlerr mapping
            }
            if(checks[RFL_CHK]->isChecked()) {
                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_LR");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                                 (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSpacer(writer);
                } writer.endMapping(); // end spacerlf

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RR");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                                 -((dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value()),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSpacer(writer);
                } writer.endMapping(); // end spacerrf

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAGXSubTrackF(writer);
                    writer.putKey("elements");
                    writer.startListing(); {
                        writer.startMapping(); {
                            writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                            writer.putKeyValue("name", "TRACK_LR");
                            writer.putKey("sprocketNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("SPROCKET_LR");
                            } writer.endListing(); // end sprocketnames listing
                            writer.putKey("rollerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("ROLLER_LR");
                            } writer.endListing(); // end rollernames listing
                            writer.putKey("idlerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("IDLER_LR");
                            } writer.endListing(); // end idlernames listing
                            writeAGXSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAGXSubTrackF(writer);
                    writer.putKey("elements");
                    writer.startListing(); {
                        writer.startMapping(); {
                            writer.putKeyValue("type", "AGXVehicleContinuousTrackDevice");
                            writer.putKeyValue("name", "TRACK_RR");
                            writer.putKey("sprocketNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("SPROCKET_RR");
                            } writer.endListing(); // end sprocketnames listing
                            writer.putKey("rollerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("ROLLER_RR");
                            } writer.endListing(); // end rollernames listing
                            writer.putKey("idlerNames");
                            writer.startFlowStyleListing(); {
                                writer.putString("IDLER_RR");
                            } writer.endListing(); // end idlernames listing
                            writeAGXSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSprocketR(writer);
                } writer.endMapping(); // end sprocketl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXRollerR(writer);
                } writer.endMapping(); // end rollerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value(),
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXIdlerR(writer);
                } writer.endMapping(); // end idlerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXSprocketR(writer);
                } writer.endMapping(); // end sprocketr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXRollerR(writer);
                } writer.endMapping(); // end rollerr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value(),
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAGXIdlerR(writer);
                } writer.endMapping(); // end idlerr mapping
            }
        } writer.endListing(); // end links listing
    } writer.endMapping(); // end body mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXTrack(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "fixed");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[TRK_MAS]->value() / 3.0);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[TRK_MAS]->value() / 3.0,
                                          dspins[TRK_WBS]->value(),
                                          dspins[TRK_WDT]->value(),
                                          dspins[TRK_RAD]->value() * 2.0);
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    return true;
}


bool CrawlerGeneratorImpl::writeAGXTrackBelt(YAMLWriter& writer)
{
    putKeyVector3(writer, "upAxis", Vector3(0.0, 0.0, 1.0));
    writer.putKeyValue("numberOfNodes", agxspins[TRK_BNN]->value());
    writer.putKeyValue("nodeThickness", agxdspins[TRK_BNT]->value());
    writer.putKeyValue("nodeWidth", agxdspins[TRK_BNW]->value());
    writer.putKeyValue("nodeThickerThickness", agxdspins[TRK_BNTT]->value());
    writer.putKeyValue("useThickerNodeEvery", agxspins[TRK_BUTNE]->value());
    writer.putKeyValue("material", bodyname + "Tracks");
    writer.putKeyValue("nodeDistanceTension", agxdspins[TRK_BNDTM]->value() * exp10(-agxspins[TRK_BNDTE]->value()));
    writer.putKeyValue("stabilizingHingeFrictionParameter", agxdspins[TRK_BSHFPM]->value() * exp10(-agxspins[TRK_BSHFPE]->value()));
    writer.putKeyValue("minStabilizingHingeNormalForce", agxspins[TRK_BMSHNF]->value());
    writer.putKeyValue("hingeCompliance", agxdspins[TRK_BHCM]->value() * exp10(-agxspins[TRK_BHCE]->value()));
    writer.putKeyValue("hingeSpookDamping", agxdspins[TRK_BHSD]->value());
    writer.putKeyValue("nodesToWheelsMergeThreshold", agxdspins[TRK_BNWMT]->value());
    writer.putKeyValue("nodesToWheelsSplitThreshold", agxdspins[TRK_BNWST]->value());
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSprocket(YAMLWriter& writer)
{
    writeAGXWheel(writer);
    writer.putKeyValue("mass", dspins[TRK_MAS]->value() * 2.0 / 9.0);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(dspins[TRK_MAS]->value(),
                                               dspins[TRK_RAD]->value(),
                                               dspins[TRK_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[TRK_RAD]->value());
                writer.putKeyValue("height", dspins[TRK_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[TRK_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXRoller(YAMLWriter& writer)
{
    return writeAGXSprocket(writer);
}


bool CrawlerGeneratorImpl::writeAGXIdler(YAMLWriter& writer)
{
    return writeAGXSprocket(writer);
}


bool CrawlerGeneratorImpl::writeAGXSpacer(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "revolute");
    writer.putKeyValue("jointAxis", "-Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[SPC_MAS]->value());
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(dspins[SPC_MAS]->value(),
                                               dspins[SPC_RAD]->value(),
                                               dspins[SPC_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[SPC_RAD]->value());
                writer.putKeyValue("height", dspins[SPC_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[SPC_CLR]));
                    putKeyVector3(writer, "specularColor", extractColor(buttons[SPC_CLR]));
                    writer.putKeyValue("shininess", 0.6);
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSubTrackF(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "fixed");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[FFL_MAS]->value() / 3.0);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[FFL_MAS]->value() / 3.0,
                                          dspins[FFL_WBS]->value(),
                                          dspins[FFL_WDT]->value(),
                                          std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value()));
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSubTrackR(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "fixed");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[RFL_MAS]->value() / 3.0);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[RFL_MAS]->value() / 3.0,
                                          dspins[RFL_WBS]->value(),
                                          dspins[RFL_WDT]->value(),
                                          std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value()));
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSubTrackBelt(YAMLWriter& writer)
{
    putKeyVector3(writer, "upAxis", Vector3(0.0, 0.0, 1.0));
    writer.putKeyValue("numberOfNodes", agxspins[FLP_BNN]->value());
    writer.putKeyValue("nodeThickness", agxdspins[FLP_BNT]->value());
    writer.putKeyValue("nodeWidth", agxdspins[FLP_BNW]->value());
    writer.putKeyValue("nodeThickerThickness", agxdspins[FLP_BNTT]->value());
    writer.putKeyValue("useThickerNodeEvery", agxspins[FLP_BUTNE]->value());
    writer.putKeyValue("material", bodyname + "Tracks");
    writer.putKeyValue("nodeDistanceTension", agxdspins[FLP_BNDTM]->value() * exp10(-agxspins[FLP_BNDTE]->value()));
    writer.putKeyValue("stabilizingHingeFrictionParameter", agxdspins[FLP_BSHFPM]->value() * exp10(-agxspins[FLP_BSHFPE]->value()));
    writer.putKeyValue("minStabilizingHingeNormalForce", agxspins[FLP_BMSHNF]->value());
    writer.putKeyValue("hingeCompliance", agxdspins[FLP_BHCM]->value() * exp10(-agxspins[FLP_BHCE]->value()));
    writer.putKeyValue("hingeSpookDamping", agxdspins[FLP_BHSD]->value());
    writer.putKeyValue("nodesToWheelsMergeThreshold", agxdspins[FLP_BNWMT]->value());
    writer.putKeyValue("nodesToWheelsSplitThreshold", agxdspins[FLP_BNWST]->value());
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSprocketF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2spf * r2spf / totalf * dspins[FFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               dspins[FFL_RRD]->value(),
                                               dspins[FFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[FFL_RRD]->value());
                writer.putKeyValue("height", dspins[FFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[FFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXRollerF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2rof * r2rof / totalf * dspins[FFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0,
                                               dspins[FFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
                writer.putKeyValue("height", dspins[FFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[FFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXIdlerF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2idf * r2idf / totalf * dspins[FFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               dspins[FFL_FRD]->value(),
                                               dspins[FFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[FFL_FRD]->value());
                writer.putKeyValue("height", dspins[FFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[FFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXSprocketR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2spr * r2spr / totalr * dspins[RFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               dspins[RFL_FRD]->value(),
                                               dspins[RFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[RFL_FRD]->value());
                writer.putKeyValue("height", dspins[RFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[RFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXRollerR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2ror * r2ror / totalr * dspins[RFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0,
                                               dspins[RFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
                writer.putKeyValue("height", dspins[RFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[RFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXIdlerR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2idr * r2idr / totalr * dspins[RFL_MAS]->value();

    writeAGXWheel(writer);
    writer.putKeyValue("mass", mass);
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcCylinderInertia(mass,
                                               dspins[RFL_RRD]->value(),
                                               dspins[RFL_WDT]->value());
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startFlowStyleMapping(); {
                writer.putKeyValue("type", "Cylinder");
                writer.putKeyValue("radius", dspins[RFL_RRD]->value());
                writer.putKeyValue("height", dspins[RFL_WDT]->value());
            } writer.endMapping(); // end geometry mapping
            writer.putKey("appearance");
            writer.startFlowStyleMapping(); {
                writer.putKey("material");
                writer.startFlowStyleMapping(); {
                    putKeyVector3(writer, "diffuseColor", extractColor(buttons[RFL_CLR]));
                } writer.endMapping(); // end material mapping
            } writer.endMapping(); // end appearance mapping
        } writer.endMapping(); // end shape mapping
    } writer.endMapping(); // end elements mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAGXWheel(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "revolute");
    writer.putKeyValue("jointAxis", "Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("material", bodyname + "Wheel");
    return true;
}