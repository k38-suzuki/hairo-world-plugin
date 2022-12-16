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

struct AGXVehicleContinuousTrackDeviceInfo {
    const char* name;
    const char* sprocketName;
    const char* rollerName;
    const char* idlerName;
    bool isSubTrack;
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
    MappingPtr writeConfig(const string& filename);

    void writeBody(Listing* linksNode);
    ListingPtr writeBody();
    MappingPtr writeChassis();
    MappingPtr writeSpacer();
    MappingPtr writeTrack();
    MappingPtr writeFrontTrack();
    MappingPtr writeRearTrack();

    void writeAGXBody(Listing* linksNode);
    MappingPtr writeAGXTrack();
    MappingPtr writeAGXTrackBelt();
    MappingPtr writeAGXSprocket();
    MappingPtr writeAGXRoller();
    MappingPtr writeAGXIdler();
    MappingPtr writeAGXFrontTrack();
    MappingPtr writeAGXRearTrack();
    MappingPtr writeAGXSubTrackBelt();
    MappingPtr writeAGXFrontSprocket();
    MappingPtr writeAGXFrontRoller();
    MappingPtr writeAGXFrontIdler();
    MappingPtr writeAGXRearSprocket();
    MappingPtr writeAGXRearRoller();
    MappingPtr writeAGXRearIdler();
    MappingPtr writeAGXWheel();
    MappingPtr writeCylinderShape(const double& radius, const double& height, const Vector3& color);
    MappingPtr writeAGXVehicleContinuousTrackDevice(const char* name, const char* sprocketName, const char* rollerName, const char* idlerName, const bool isSubTrack);
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
        auto topNode = writeBody(filename);
        if(yamlWriter.openFile(filename)) {
            yamlWriter.putNode(topNode);
            yamlWriter.closeFile();
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
    bool isAGXChecked = checks[AGX_CHK]->isChecked();

    node->write("format", "ChoreonoidBody");
    node->write("formatVersion", "1.0");
    node->write("angleUnit", "degree");
    node->write("name", name);

    ListingPtr linksNode = new Listing;

    linksNode->append(writeChassis());
    if(!isAGXChecked) {
        writeBody(linksNode);
    } else {
        writeAGXBody(linksNode);
    }

    if(!linksNode->empty()) {
        node->insert("links", linksNode);
    }

    return node;
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


void CrawlerGeneratorImpl::writeBody(Listing* linksNode)
{
    int jointID = 0;

    MappingPtr trackNode = writeTrack();
    MappingPtr spacerNode = writeSpacer();
    MappingPtr frontTrackNode = writeFrontTrack();
    MappingPtr rearTrackNode = writeRearTrack();

    {
        static const char* name[] = { "TRACK_L", "TRACK_R" };
        Vector3 translation[2];
        translation[0] = Vector3(0.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);

        for(int i = 0; i < 2; ++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->insert(trackNode);

            linksNode->append(node);   
        }
    }

    if(checks[FFL_CHK]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(spacerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "TRACK_LF", "TRACK_RF" };
            static const char* parent[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(dspins[FFL_WBS]->value() / 2.0,  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(dspins[FFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->insert(frontTrackNode);

                linksNode->append(node);
            }
        }
    }

    if(checks[RFL_CHK]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(spacerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "TRACK_LR", "TRACK_RR" };
            static const char* parent[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(-dspins[RFL_WBS]->value() / 2.0,  (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-dspins[RFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->insert(rearTrackNode);

                linksNode->append(node);
            }
        }
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


MappingPtr CrawlerGeneratorImpl::writeSpacer()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "revolute");
    node->write("jointAxis", "-Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[SPC_MAS]->value());
    write(node, "inertia", calcCylinderInertia(dspins[SPC_MAS]->value(), dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value(), extractColor(buttons[SPC_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeTrack()
{
    MappingPtr trackNode = new Mapping;
    trackNode->write("parent", "CHASSIS");
    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", dspins[TRK_MAS]->value());
    write(trackNode, "inertia", calcBoxInertia(dspins[TRK_MAS]->value(), dspins[TRK_WBS]->value(), dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0));

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
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerGeneratorImpl::writeFrontTrack()
{
    MappingPtr trackNode = new Mapping;

    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", dspins[FFL_MAS]->value());
    double radius = std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value());
    write(trackNode, "inertia", calcBoxInertia(dspins[FFL_MAS]->value(), dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), radius * 2.0));

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
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerGeneratorImpl::writeRearTrack()
{
    MappingPtr trackNode = new Mapping;

    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", dspins[RFL_MAS]->value());
    double radius = std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value());
    write(trackNode, "inertia", calcBoxInertia(dspins[RFL_MAS]->value(), dspins[RFL_WBS]->value(), dspins[RFL_WDT]->value(), radius * 2.0));

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
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


void CrawlerGeneratorImpl::writeAGXBody(Listing* linksNode)
{
    int jointID = 0;

    MappingPtr agxTrackNode = writeAGXTrack();
    MappingPtr agxSprocketNode = writeAGXSprocket();
    MappingPtr agxRollerNode = writeAGXRoller();
    MappingPtr agxIdlerNode = writeAGXIdler();
    MappingPtr spacerNode = writeSpacer();
    MappingPtr agxFrontTrackNode = writeAGXFrontTrack();
    MappingPtr agxFrontSprocketNode = writeAGXFrontSprocket();
    MappingPtr agxFrontRollerNode = writeAGXFrontRoller();
    MappingPtr agxFrontIdlerNode = writeAGXFrontIdler();
    MappingPtr agxRearTrackNode = writeAGXRearTrack();
    MappingPtr agxRearSprocketNode = writeAGXRearSprocket();
    MappingPtr agxRearRollerNode = writeAGXRearRoller();
    MappingPtr agxRearIdlerNode = writeAGXRearIdler();

    {
        static const char* name[] = { "TRACK_L", "TRACK_R" };
        static const char* parent[] = { "CHASSIS", "CHASSIS" };
        Vector3 translation[2];
        translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);

        for(int i = 0; i < 2; ++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            node->write("parent", parent[i]);
            write(node, "translation", translation[i]);
            node->insert(agxTrackNode);

            ListingPtr elementsNode = new Listing;
            static const AGXVehicleContinuousTrackDeviceInfo info[] = {
                { "TRACK_L", "SPROCKET_L", "ROLLER_L", "IDLER_L", false },
                { "TRACK_R", "SPROCKET_R", "ROLLER_R", "IDLER_R", false }
            };

            elementsNode->append(writeAGXVehicleContinuousTrackDevice(info[i].name, info[i].sprocketName, info[i].rollerName, info[i].idlerName, info[i].isSubTrack));
            if(!elementsNode->empty()) {
                node->insert("elements", elementsNode);
            }

            linksNode->append(node);
        }
    }

    {
        static const char* name[] = { "SPROCKET_L", "SPROCKET_R" };
        Vector3 translation[2];
        translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);

        for(int i = 0; i < 2;++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->write("jointId", jointID++);
            node->insert(agxSprocketNode);

            linksNode->append(node);
        }
    }

    {
        static const char* name[] = { "ROLLER_L", "ROLLER_R" };
        Vector3 translation[2];
        translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);

        for(int i = 0; i < 2;++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->write("jointId", jointID++);
            node->insert(agxRollerNode);

            linksNode->append(node);
        }
    }

    {
        static const char* name[] = { "IDLER_L", "IDLER_R" };
        Vector3 translation[2];
        translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);

        for(int i = 0; i < 2;++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->write("jointId", jointID++);
            node->insert(agxIdlerNode);

            linksNode->append(node);
        }
    }

    if(checks[FFL_CHK]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(spacerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "TRACK_LF", "TRACK_RF" };
            static const char* parent[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(0.0,  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->insert(agxFrontTrackNode);

                ListingPtr elementsNode = new Listing;
                static const AGXVehicleContinuousTrackDeviceInfo info[] = {
                    { "TRACK_LF", "SPROCKET_LF", "ROLLER_LF", "IDLER_LF", true },
                    { "TRACK_RF", "SPROCKET_RF", "ROLLER_RF", "IDLER_RF", true }
                };

                elementsNode->append(writeAGXVehicleContinuousTrackDevice(info[i].name, info[i].sprocketName, info[i].rollerName, info[i].idlerName, info[i].isSubTrack));
                if(!elementsNode->empty()) {
                    node->insert("elements", elementsNode);
                }

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "SPROCKET_LF", "SPROCKET_RF" };
            static const char* parent[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(0.0,  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxFrontSprocketNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "ROLLER_LF", "ROLLER_RF" };
            static const char* parent[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(dspins[FFL_WBS]->value() / 2.0,  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(dspins[FFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxFrontRollerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "IDLER_LF", "IDLER_RF" };
            static const char* parent[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            translation[0] = Vector3(dspins[FFL_WBS]->value(), (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(dspins[FFL_WBS]->value(), -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxFrontIdlerNode);

                linksNode->append(node);
            }
        }
    }


    if(checks[RFL_CHK]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(spacerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "TRACK_LR", "TRACK_RR" };
            static const char* parent[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(0.0, (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->insert(agxRearTrackNode);

                ListingPtr elementsNode = new Listing;
                static const AGXVehicleContinuousTrackDeviceInfo info[] = {
                    { "TRACK_LR", "SPROCKET_LR", "ROLLER_LR", "IDLER_LR", true },
                    { "TRACK_RR", "SPROCKET_RR", "ROLLER_RR", "IDLER_RR", true }
                };

                elementsNode->append(writeAGXVehicleContinuousTrackDevice(info[i].name, info[i].sprocketName, info[i].rollerName, info[i].idlerName, info[i].isSubTrack));
                if(!elementsNode->empty()) {
                    node->insert("elements", elementsNode);
                }

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "SPROCKET_LR", "SPROCKET_RR" };
            static const char* parent[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(0.0,  (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxRearSprocketNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "ROLLER_LR", "ROLLER_RR" };
            static const char* parent[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(-dspins[RFL_WBS]->value() / 2.0,  (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-dspins[RFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxRearRollerNode);

                linksNode->append(node);
            }
        }

        {
            static const char* name[] = { "IDLER_LR", "IDLER_RR" };
            static const char* parent[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            translation[0] = Vector3(-dspins[RFL_WBS]->value(),  (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-dspins[RFL_WBS]->value(), -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0, 0.0);

            for(int i = 0; i < 2; ++i) {
                MappingPtr node = new Mapping;

                node->write("name", name[i]);
                node->write("parent", parent[i]);
                write(node, "translation", translation[i]);
                node->write("jointId", jointID++);
                node->insert(agxRearIdlerNode);

                linksNode->append(node);
            }
        }
    }
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
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->insert(writeAGXWheel());
    node->write("mass", dspins[TRK_MAS]->value() * 2.0 / 9.0);
    write(node, "inertia", calcCylinderInertia(dspins[TRK_MAS]->value(), dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value(), extractColor(buttons[TRK_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
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


MappingPtr CrawlerGeneratorImpl::writeAGXFrontTrack()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[FFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[FFL_MAS]->value() / 3.0, dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRearTrack()
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


MappingPtr CrawlerGeneratorImpl::writeAGXFrontSprocket()
{
    MappingPtr node = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2spf * r2spf / totalf * dspins[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, dspins[FFL_RRD]->value(), dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[FFL_RRD]->value(), dspins[FFL_WDT]->value(), extractColor(buttons[FFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXFrontRoller()
{
    MappingPtr node = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2rof * r2rof / totalf * dspins[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, (dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0, dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0, dspins[FFL_WDT]->value(), extractColor(buttons[FFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXFrontIdler()
{
    MappingPtr node = new Mapping;

    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2idf * r2idf / totalf * dspins[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, dspins[FFL_FRD]->value(), dspins[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[FFL_FRD]->value(), dspins[FFL_WDT]->value(), extractColor(buttons[FFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRearSprocket()
{
    MappingPtr node = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2spr * r2spr / totalr * dspins[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, dspins[RFL_FRD]->value(), dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[RFL_FRD]->value(), dspins[RFL_WDT]->value(), extractColor(buttons[RFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRearRoller()
{
    MappingPtr node = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2ror * r2ror / totalr * dspins[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, (dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0, dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0, dspins[RFL_WDT]->value(), extractColor(buttons[RFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXRearIdler()
{
    MappingPtr node = new Mapping;

    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2idr * r2idr / totalr * dspins[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, dspins[RFL_RRD]->value(), dspins[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[RFL_RRD]->value(), dspins[RFL_WDT]->value(), extractColor(buttons[RFL_CLR])));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
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


MappingPtr CrawlerGeneratorImpl::writeCylinderShape(const double& radius, const double& height, const Vector3& color)
{
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", radius);
    geometryNode->write("height", height);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = new Mapping;
    write(materialNode, "diffuseColor", color);
    appearanceNode->insert("material", materialNode);

    return node;
}


MappingPtr CrawlerGeneratorImpl::writeAGXVehicleContinuousTrackDevice(const char* name, const char* sprocketName, const char* rollerName, const char* idlerName, const bool isSubTrack)
{
    MappingPtr node = new Mapping;

    node->write("type", "AGXVehicleContinuousTrackDevice");
    node->write("name", name);

    Listing& sprocketNamesNode = *node->createFlowStyleListing("sprocketNames");
    sprocketNamesNode.append(sprocketName);

    Listing& rollerNamesNode = *node->createFlowStyleListing("rollerNames");
    rollerNamesNode.append(rollerName);

    Listing& idlerNamesNode = *node->createFlowStyleListing("idlerNames");
    idlerNamesNode.append(idlerName);

    if(!isSubTrack) {
        node->insert(writeAGXTrackBelt());
    } else {
        node->insert(writeAGXSubTrackBelt());
    }

    return node;
}