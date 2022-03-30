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

    bool save(const string& filename);
    void initialize();
    void onResetButtonClicked();
    void onExportYamlButtonClicked();
    void onImportYamlButtonClicked();
    bool loadConfig(const string& filename, ostream& os = nullout());
    void onEnableAgxCheckToggled(const bool& on);
    void onColorChanged(PushButton* pushbutton);
    void setColor(PushButton* pushbutton, const Vector3& color);
    Vector3 extractColor(PushButton* colorButton);
    bool writeYaml(const string& filename);

    bool write(const string& filename);
    bool writeTrack(YAMLWriter& writer);
    bool writeSpacer(YAMLWriter& writer);
    bool writeSubTrackF(YAMLWriter& writer);
    bool writeSubTrackR(YAMLWriter& writer);

    bool writeAgx(const string& filename);
    bool writeAgxTrack(YAMLWriter& writer);
    bool writeAgxTrackBelt(YAMLWriter& writer);
    bool writeAgxSprocket(YAMLWriter& writer);
    bool writeAgxRoller(YAMLWriter& writer);
    bool writeAgxIdler(YAMLWriter& writer);
    bool writeAgxSpacer(YAMLWriter& writer);
    bool writeAgxSubTrackF(YAMLWriter& writer);
    bool writeAgxSubTrackR(YAMLWriter& writer);
    bool writeAgxSubTrackBelt(YAMLWriter& writer);
    bool writeAgxSprocketF(YAMLWriter& writer);
    bool writeAgxRollerF(YAMLWriter& writer);
    bool writeAgxIdlerF(YAMLWriter& writer);
    bool writeAgxSprocketR(YAMLWriter& writer);
    bool writeAgxRollerR(YAMLWriter& writer);
    bool writeAgxIdlerR(YAMLWriter& writer);
    bool writeAgxWheel(YAMLWriter& writer);
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
    toolButtons[IMPORT]->sigClicked().connect([&](){ onImportYamlButtonClicked(); });
    toolButtons[EXPORT]->sigClicked().connect([&](){ onExportYamlButtonClicked(); });
    checks[AGX_CHK]->sigToggled().connect([&](bool on){ onEnableAgxCheckToggled(on); });
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
        if(!checks[AGX_CHK]->isChecked()) {
            write(filename);
        } else {
            writeAgx(filename);
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


void CrawlerGeneratorImpl::onImportYamlButtonClicked()
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
        loadConfig(filename);
    }
}


bool CrawlerGeneratorImpl::loadConfig(const string& filename,std::ostream& os )
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
                            bool checked = checkList[j].toBool() ? true : false;
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


void CrawlerGeneratorImpl::onExportYamlButtonClicked()
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
        writeYaml(filename);
    }
}


void CrawlerGeneratorImpl::onEnableAgxCheckToggled(const bool& on)
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


bool CrawlerGeneratorImpl::writeYaml(const string& filename)
{
    if(filename.empty()) {
        return false;
    }
    filesystem::path path(filename);
    string name = path.stem().string();

    YAMLWriter writer(filename);
    writer.startMapping(); {
        writer.putKeyValue("format", "CrawlerRobotBuilderYaml");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("name", name);
        writer.putKey("configs");
        writer.startListing(); {
            writer.startMapping();
            writer.putKey("doubleSpin");
            writer.startFlowStyleListing(); {
                for(int i = 0; i < NUM_DSPINS; ++i) {
                    writer.putScalar(dspins[i]->value());
                }
            } writer.endListing();

            writer.putKey("spin");
            writer.startFlowStyleListing(); {
                for(int i = 0; i < NUM_SPINS; ++i) {
                    writer.putScalar(agxspins[i]->value());
                }
            } writer.endListing();

            for(int i = 0; i < NUM_BUTTONS; ++i) {
                string key = "button" + to_string(i);
                putKeyVector3(writer, key, extractColor(buttons[i]));
            }

            writer.putKey("check");
            writer.startFlowStyleListing(); {
                for(int i = 0; i < NUM_CHECKS; ++i) {
                    writer.putScalar(checks[i]->isChecked());
                }
            } writer.endListing();
        } writer.endListing();
    } writer.endMapping();
    return true;
}


bool CrawlerGeneratorImpl::write(const string& filename)
{
    if(filename.empty()) {
        return false;
    }

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
                writeTrack(writer);
            } writer.endMapping(); // end trackl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "TRACK_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                              -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                              -dspins[CHS_ZSZ]->value() / 2.0));
                writeTrack(writer);
            } writer.endMapping(); // end trackr mapping

            if(checks[FFL_CHK]->isChecked()) {
                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_LF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                                                                  -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeSpacer(writer);
                } writer.endMapping(); // end spacerlf mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                  -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(),
                                                                  -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeSpacer(writer);
                } writer.endMapping(); // end spacerrf mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                  0.0));
                    writeSubTrackF(writer);
                } writer.endMapping(); // end tracklf mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                  -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                  0.0));
                    writeSubTrackF(writer);
                } writer.endMapping(); // end trackrf mapping
            }
            if(checks[RFL_CHK]->isChecked()) {
                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_LR");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                                  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                                                                  -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeSpacer(writer);
                } writer.endMapping(); // end spacerlr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RR");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                                  -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(),
                                                                  -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeSpacer(writer);
                } writer.endMapping(); // end spacerrr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                  (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                  0.0));
                    writeSubTrackR(writer);
                } writer.endMapping(); // end tracklr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                  -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                  0.0));
                    writeSubTrackR(writer);
                } writer.endMapping(); // end trackrr mapping
            }
        } writer.endListing(); // end links listing
    } writer.endMapping(); // end body mapping
    return true;
}


bool CrawlerGeneratorImpl::writeTrack(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "pseudo_continuous_track");
    writer.putKeyValue("jointAxis", "Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[TRK_MAS]->value());
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[TRK_MAS]->value(),
                                          dspins[TRK_WBS]->value(),
                                          dspins[TRK_WDT]->value(),
                                          dspins[TRK_RAD]->value() * 2.0);
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startMapping(); {
                writer.putKeyValue("type", "Extrusion");
                writer.putKey("crossSection");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 9; ++i) {
                        writer.putScalar(dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[TRK_RAD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 9; ++i) {
                        writer.putScalar(-dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[TRK_RAD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 1; ++i) {
                        writer.putScalar(dspins[TRK_WBS]->value() / 2.0 + dspins[TRK_RAD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[TRK_RAD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                } writer.endListing(); // end crosssection listing;
                Vector6 spine;
                spine << 0.0, -dspins[TRK_WDT]->value() / 2.0, 0.0, 0.0, dspins[TRK_WDT]->value() / 2.0, 0.0;
                writer.putKey("spine");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 6; ++i) {
                        writer.putScalar(spine[i]);
                    }
                } writer.endListing(); // end spine listing
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


bool CrawlerGeneratorImpl::writeSpacer(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeSubTrackF(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "pseudo_continuous_track");
    writer.putKeyValue("jointAxis", "Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[FFL_MAS]->value());
    double frontSubTrackRadius = std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value());
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[FFL_MAS]->value(),
                                          dspins[FFL_WBS]->value(),
                                          dspins[FFL_WDT]->value(),
                                          frontSubTrackRadius * 2.0);
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startMapping(); {
                writer.putKeyValue("type", "Extrusion");
                writer.putKey("crossSection");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 9; i++) {
                        writer.putScalar(dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[FFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 9; i++) {
                        writer.putScalar(-dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_RRD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[FFL_RRD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 1; i++) {
                        writer.putScalar(dspins[FFL_WBS]->value() / 2.0 + dspins[FFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[FFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                } writer.endListing(); // end crosssection listing;
                Vector6 spine;
                spine << 0.0, -dspins[FFL_WDT]->value() / 2.0, 0.0, 0.0, dspins[FFL_WDT]->value() / 2.0, 0.0;
                writer.putKey("spine");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 6; ++i) {
                        writer.putScalar(spine[i]);
                    }
                } writer.endListing(); // end spine listing
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


bool CrawlerGeneratorImpl::writeSubTrackR(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "pseudo_continuous_track");
    writer.putKeyValue("jointAxis", "Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("mass", dspins[RFL_MAS]->value());
    double rearSubTrackRadius = std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value());
    writer.putKey("inertia");
    writer.startFlowStyleListing(); {
        VectorXd inertia = calcBoxInertia(dspins[RFL_MAS]->value(),
                                          dspins[RFL_WBS]->value(),
                                          dspins[RFL_WDT]->value(),
                                          rearSubTrackRadius * 2.0);
        for(int i = 0; i < 9; ++i) {
            writer.putScalar(inertia[i]);
        }
    } writer.endListing(); // end inertia listing
    writer.putKey("elements");
    writer.startMapping(); {
        writer.putKey("Shape");
        writer.startMapping(); {
            writer.putKey("geometry");
            writer.startMapping(); {
                writer.putKeyValue("type", "Extrusion");
                writer.putKey("crossSection");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 9; i++) {
                        writer.putScalar(dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[RFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 9; i++) {
                        writer.putScalar(-dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_RRD]->value() * cos((90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[RFL_RRD]->value() * sin((90.0 + 22.5 * i) * TO_RADIAN));
                    }
                    for(int i = 0; i < 1; i++) {
                        writer.putScalar(dspins[RFL_WBS]->value() / 2.0 + dspins[RFL_FRD]->value() * cos((-90.0 + 22.5 * i) * TO_RADIAN));
                        writer.putScalar(dspins[RFL_FRD]->value() * sin((-90.0 + 22.5 * i) * TO_RADIAN));
                    }
                } writer.endListing(); // end crosssection listing;
                Vector6 spine;
                spine << 0.0, -dspins[RFL_WDT]->value() / 2.0, 0.0, 0.0, dspins[RFL_WDT]->value() / 2.0, 0.0;
                writer.putKey("spine");
                writer.startFlowStyleListing(); {
                    for(int i = 0; i < 6; ++i) {
                        writer.putScalar(spine[i]);
                    }
                } writer.endListing(); // end spine listing
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


bool CrawlerGeneratorImpl::writeAgx(const string& filename)
{
    if(filename.empty()) {
        return false;
    }

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
                writeAgxTrack(writer);
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
                        writeAgxTrackBelt(writer);
                    } writer.endMapping(); // end elements mapping
                } writer.endListing(); // end elements listing
            } writer.endMapping(); // end trackl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "TRACK_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writeAgxTrack(writer);
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
                        writeAgxTrackBelt(writer);
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
                writeAgxSprocket(writer);
            } writer.endMapping(); // end sprocketl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "ROLLER_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAgxRoller(writer);
            } writer.endMapping(); // end rollerl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "IDLER_L");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                             (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAgxIdler(writer);
            } writer.endMapping(); // end idlerl mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "SPROCKET_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAgxSprocket(writer);
            } writer.endMapping(); // end sprocketr mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "ROLLER_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(0.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAgxRoller(writer);
            } writer.endMapping(); // end rollerr mapping

            writer.startMapping(); {
                writer.putKeyValue("name", "IDLER_R");
                writer.putKeyValue("parent", "CHASSIS");
                putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                             -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0,
                                                             -dspins[CHS_ZSZ]->value() / 2.0));
                writer.putKeyValue("jointId", jointId++);
                writeAgxIdler(writer);
            } writer.endMapping(); // end idlerr mapping

            if(checks[FFL_CHK]->isChecked()) {
                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_LF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                 (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxSpacer(writer);
                } writer.endMapping(); // end spacerlf

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RF");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(dspins[TRK_WBS]->value() / 2.0,
                                                                 -((dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value()),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxSpacer(writer);
                } writer.endMapping(); // end spacerrf

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAgxSubTrackF(writer);
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
                            writeAgxSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAgxSubTrackF(writer);
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
                            writeAgxSubTrackBelt(writer);
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
                    writeAgxSprocketF(writer);
                } writer.endMapping(); // end sprocketl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxRollerF(writer);
                } writer.endMapping(); // end rollerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_LF");
                    writer.putKeyValue("parent", "SPACER_LF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value(),
                                                                 (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxIdlerF(writer);
                } writer.endMapping(); // end idlerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxSprocketF(writer);
                } writer.endMapping(); // end sprocketr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value() / 2.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxRollerF(writer);
                } writer.endMapping(); // end rollerr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_RF");
                    writer.putKeyValue("parent", "SPACER_RF");
                    putKeyVector3(writer, "translation", Vector3(dspins[FFL_WBS]->value(),
                                                                 -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxIdlerF(writer);
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
                    writeAgxSpacer(writer);
                } writer.endMapping(); // end spacerlf

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPACER_RR");
                    writer.putKeyValue("parent", "CHASSIS");
                    putKeyVector3(writer, "translation", Vector3(-dspins[TRK_WBS]->value() / 2.0,
                                                                 -((dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value()),
                                                                 -dspins[CHS_ZSZ]->value() / 2.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxSpacer(writer);
                } writer.endMapping(); // end spacerrf

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAgxSubTrackF(writer);
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
                            writeAgxSubTrackBelt(writer);
                        } writer.endMapping(); // end elements mapping
                    } writer.endListing(); // end elements listing
                } writer.endMapping(); // end trackl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "TRACK_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writeAgxSubTrackF(writer);
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
                            writeAgxSubTrackBelt(writer);
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
                    writeAgxSprocketR(writer);
                } writer.endMapping(); // end sprocketl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxRollerR(writer);
                } writer.endMapping(); // end rollerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_LR");
                    writer.putKeyValue("parent", "SPACER_LR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value(),
                                                                 (dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxIdlerR(writer);
                } writer.endMapping(); // end idlerl mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "SPROCKET_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(0.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxSprocketR(writer);
                } writer.endMapping(); // end sprocketr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "ROLLER_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value() / 2.0,
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxRollerR(writer);
                } writer.endMapping(); // end rollerr mapping

                writer.startMapping(); {
                    writer.putKeyValue("name", "IDLER_RR");
                    writer.putKeyValue("parent", "SPACER_RR");
                    putKeyVector3(writer, "translation", Vector3(-dspins[RFL_WBS]->value(),
                                                                 -(dspins[SPC_WDT]->value() + dspins[RFL_WDT]->value()) / 2.0,
                                                                 0.0));
                    writer.putKeyValue("jointId", jointId++);
                    writeAgxIdlerR(writer);
                } writer.endMapping(); // end idlerr mapping
            }
        } writer.endListing(); // end links listing
    } writer.endMapping(); // end body mapping
    return true;
}


bool CrawlerGeneratorImpl::writeAgxTrack(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxTrackBelt(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxSprocket(YAMLWriter& writer)
{
    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxRoller(YAMLWriter& writer)
{
    return writeAgxSprocket(writer);
}


bool CrawlerGeneratorImpl::writeAgxIdler(YAMLWriter& writer)
{
    return writeAgxSprocket(writer);
}


bool CrawlerGeneratorImpl::writeAgxSpacer(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxSubTrackF(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxSubTrackR(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxSubTrackBelt(YAMLWriter& writer)
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


bool CrawlerGeneratorImpl::writeAgxSprocketF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2spf * r2spf / totalf * dspins[FFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxRollerF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2rof * r2rof / totalf * dspins[FFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxIdlerF(YAMLWriter& writer)
{
    double r2spf = dspins[FFL_RRD]->value() * dspins[FFL_RRD]->value();
    double r2rof = ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0) * ((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0);
    double r2idf = dspins[FFL_FRD]->value() * dspins[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2idf * r2idf / totalf * dspins[FFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxSprocketR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2spr * r2spr / totalr * dspins[RFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxRollerR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2ror * r2ror / totalr * dspins[RFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxIdlerR(YAMLWriter& writer)
{
    double r2spr = dspins[RFL_RRD]->value() * dspins[RFL_RRD]->value();
    double r2ror = ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0) * ((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0);
    double r2idr = dspins[RFL_FRD]->value() * dspins[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2idr * r2idr / totalr * dspins[RFL_MAS]->value();

    writeAgxWheel(writer);
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


bool CrawlerGeneratorImpl::writeAgxWheel(YAMLWriter& writer)
{
    writer.putKeyValue("jointType", "revolute");
    writer.putKeyValue("jointAxis", "Y");
    putKeyVector3(writer, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    writer.putKeyValue("material", bodyname + "Wheel");
    return true;
}
