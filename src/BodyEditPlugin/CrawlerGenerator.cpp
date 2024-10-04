/**
   @author Kenta Suzuki
*/

#include "CrawlerGenerator.h"
#include <cnoid/Button>
#include <cnoid/ButtonGroup>
#include <cnoid/CheckBox>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include <cnoid/MathUtil>
#include <cnoid/MenuManager>
#include <cnoid/NullOut>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/UTF8>
#include <cnoid/Widget>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <cnoid/stdx/filesystem>
#include <cnoid/HamburgerMenu>
#include <QBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include "ColorButton.h"
#include "FileFormWidget.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

CrawlerGenerator* crawlerInstance = nullptr;

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
    int page;
};

ButtonInfo buttonInfo[] = {
    { 1, 3,   0.0 / 255.0, 153.0 / 255.0,  0.0 / 255.0, 0 },
    { 4, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 0 },
    { 1, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 1 },
    { 5, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 1 },
    { 8, 3, 255.0 / 255.0,   0.0 / 255.0,  0.0 / 255.0, 0 }
};

struct DoubleSpinInfo {
    int row;
    int column;
    double value;
    double min;
    double max;
    int decimals;
    bool enabled;
    int page;
};

DoubleSpinInfo doubleSpinInfo[] = {
    { 1, 1,  8.000, 0.0, 1000.000, 3,  true, 0 }, { 2, 1,  0.450, 0.0, 1000.000, 3,  true, 0 }, {  2, 2,  0.300, 0.0, 1000.000, 3,  true, 0 }, { 2, 3,  0.100, 0.0, 1000.000, 3,  true, 0 },
    { 4, 1,  1.000, 0.0, 1000.000, 3,  true, 0 }, { 5, 1,  0.080, 0.0, 1000.000, 3,  true, 0 }, {  6, 1,  0.100, 0.0, 1000.000, 3,  true, 0 }, { 6, 3,  0.420, 0.0, 1000.000, 3,  true, 0 },
    { 1, 1,  0.250, 0.0, 1000.000, 3,  true, 1 }, { 2, 1,  0.080, 0.0, 1000.000, 3,  true, 1 }, {  2, 2,  0.080, 0.0, 1000.000, 3,  true, 1 }, { 3, 1,  0.080, 0.0, 1000.000, 3,  true, 1 }, { 3, 3,  0.130, 0.0, 1000.000, 3,  true, 1 },
    { 5, 1,  0.250, 0.0, 1000.000, 3,  true, 1 }, { 6, 1,  0.080, 0.0, 1000.000, 3,  true, 1 }, {  6, 2,  0.080, 0.0, 1000.000, 3,  true, 1 }, { 7, 1,  0.080, 0.0, 1000.000, 3,  true, 1 }, { 7, 3,  0.130, 0.0, 1000.000, 3,  true, 1 },
    { 8, 1,  0.200, 0.0, 1000.000, 3,  true, 0 }, { 9, 1,  0.060, 0.0, 1000.000, 3,  true, 0 }, { 10, 1,  0.013, 0.0, 1000.000, 3,  true, 0 }
};

//DoubleSpinInfo agxdoubleSpinInfo[] = {
//    {  1, 4,  0.010, 0.0, 1000.000, 3, false }, {  2, 1,  0.090, 0.0, 1000.000, 3, false }, {  2, 4,  0.020,       0.0, 1000.000, 3, false }, {  3, 4,  2.000,       0.0, 1000.000, 3, false }, {  4, 1,  1.000, 0.0, 1000.000, 3, false },
//    {  5, 1,  9.000, 0.0, 1000.000, 3, false }, {  5, 4,  0.010, 0.0, 1000.000, 3, false }, {  6, 1, -0.001, -1000.000, 1000.000, 3, false }, {  6, 4, -0.009, -1000.000, 1000.000, 3, false },
//    {  8, 4,  0.010, 0.0, 1000.000, 3, false }, {  9, 1,  0.090, 0.0, 1000.000, 3, false }, {  9, 4,  0.020,       0.0, 1000.000, 3, false }, { 10, 4,  2.000,       0.0, 1000.000, 3, false }, { 11, 1,  1.000, 0.0, 1000.000, 3, false },
//    { 12, 1,  9.000, 0.0, 1000.000, 3, false }, { 12, 4,  0.010, 0.0, 1000.000, 3, false }, { 13, 1, -0.001, -1000.000, 1000.000, 3, false }, { 13, 4, -0.009, -1000.000, 1000.000, 3, false }
//};

DoubleSpinInfo agxdoubleSpinInfo[] = {
    {  1, 4,  0.010, 0.0, 1000.000, 3, false, 0 }, {  2, 1,  0.100, 0.0, 1000.000, 3, false, 0 }, {  2, 4,  0.020,       0.0, 1000.000, 3, false, 0 }, {  3, 4,  9.000,       0.0, 1000.000, 3, false, 0 }, {  4, 1,  4.000, 0.0, 1000.000, 3, false, 0 },
    {  5, 1,  9.000, 0.0, 1000.000, 3, false, 0 }, {  5, 4,  0.010, 0.0, 1000.000, 3, false, 0 }, {  6, 1, -0.001, -1000.000, 1000.000, 3, false, 0 }, {  6, 4, -0.009, -1000.000, 1000.000, 3, false, 0 },
    {  8, 4,  0.010, 0.0, 1000.000, 3, false, 0 }, {  9, 1,  0.080, 0.0, 1000.000, 3, false, 0 }, {  9, 4,  0.020,       0.0, 1000.000, 3, false, 0 }, { 10, 4,  9.000,       0.0, 1000.000, 3, false, 0 }, { 11, 1,  4.000, 0.0, 1000.000, 3, false, 0 },
    { 12, 1,  9.000, 0.0, 1000.000, 3, false, 0 }, { 12, 4,  0.010, 0.0, 1000.000, 3, false, 0 }, { 13, 1, -0.001, -1000.000, 1000.000, 3, false, 0 }, { 13, 4, -0.009, -1000.000, 1000.000, 3, false, 0 }
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
    int page;
};

LabelInfo labelInfo[] = {
    { 1, 0, 0 }, { 1, 2, 0 }, { 2, 0, 0 },
    { 4, 0, 0 }, { 4, 2, 0 }, { 5, 0, 0 }, {  6, 0, 0 }, { 6, 2, 0 },
    { 1, 0, 1 }, { 1, 2, 1 }, { 2, 0, 1 }, {  3, 0, 1 }, { 3, 2, 1 },
    { 5, 0, 1 }, { 5, 2, 1 }, { 6, 0, 1 }, {  7, 0, 1 }, { 7, 2, 1 },
    { 8, 0, 0 }, { 8, 2, 0 }, { 9, 0, 0 }, { 10, 0, 0 }
};

LabelInfo agxlabelInfo[] = {
    {  1, 0, 0 }, {  1, 3, 0 },
    {  2, 0, 0 }, {  2, 3, 0 },
    {  3, 0, 0 }, {  3, 3, 0 },
    {  4, 0, 0 }, {  4, 3, 0 },
    {  5, 0, 0 }, {  5, 3, 0 },
    {  6, 0, 0 }, {  6, 3, 0 },
    {  8, 0, 0 }, {  8, 3, 0 },
    {  9, 0, 0 }, {  9, 3, 0 },
    { 10, 0, 0 }, { 10, 3, 0 },
    { 11, 0, 0 }, { 11, 3, 0 },
    { 12, 0, 0 }, { 12, 3, 0 },
    { 13, 0, 0 }, { 13, 3, 0 }
};

struct Info {
    int row;
    int column;
    int page;
};

Info separatorInfo[] = {
    { 0, 0, 0 },
    { 3, 0, 0 },
    { 0, 0, 1 },
    { 4, 0, 1 },
    { 7, 0, 0 }
};

Info agxseparatorInfo[] = {
    { 0, 0, 0 },
    { 7, 0, 0 }
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

class CrawlerGenerator::Impl : public Dialog
{
public:

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
    enum DialogButtonId { RESET, IMPORT, EXPORT, NUM_TBUTTONS };

    CheckBox* checks[NUM_CHECKS];
    ColorButton* buttons[NUM_BUTTONS];
    DoubleSpinBox* dspins[NUM_DSPINS];
    DoubleSpinBox* agxdspins[NUM_AGXDSPINS];
    SpinBox* agxspins[NUM_SPINS];
    PushButton* toolButtons[NUM_TBUTTONS];
    ButtonGroup settingGroup;
    RadioButton setting1Radio;
    RadioButton setting2Radio;
    QStackedWidget* topWidget;
    FileFormWidget* formWidget;
    YAMLWriter yamlWriter;
    YAMLWriter yamlWriter2;

    Impl();

    bool save(const string& filename);
    bool save2(const string& filename);

    void initialize();
    void onResetButtonClicked();
    void onExportButtonClicked();
    void onImportButtonClicked();

    bool load2(const string& filename, ostream& os = nullout());
    void onEnableAGXCheckToggled(bool checked);
    void onButtonToggled(const int& id, bool checked);

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


void CrawlerGenerator::initializeClass(ExtensionManager* ext)
{
    if(!crawlerInstance) {
        crawlerInstance = ext->manage(new CrawlerGenerator);

        MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("Make a body file"));
        mm.addItem(_("CrawlerRobot"))->sigTriggered().connect(
                    [&](){ crawlerInstance->impl->show(); });
    }
}


CrawlerGenerator::CrawlerGenerator()
{
    impl = new Impl;
}


CrawlerGenerator::Impl::Impl()
    : Dialog()
{
    setWindowTitle(_("CrawlerRobot Generator"));

    yamlWriter.setKeyOrderPreservationMode(true);
    yamlWriter2.setKeyOrderPreservationMode(true);

    QGridLayout* gbox[2];
    gbox[0] = new QGridLayout;
    gbox[1] = new QGridLayout;
    QGridLayout* agbox = new QGridLayout;

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox;
        DoubleSpinBox* dspin = dspins[i];
        gbox[info.page]->addWidget(dspin, info.row, info.column);
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
        buttons[i] = new ColorButton;
        PushButton* button = buttons[i];
        gbox[info.page]->addWidget(button, info.row, info.column);
    }

    static const char* label0[] = {
        _("Chassis"), _("Track"), _("Front SubTrack"), _("Rear SubTrack"), _("Spacer")
    };

    for(int i = 0; i < 5; ++i) {
        Info info = separatorInfo[i];
        gbox[info.page]->addLayout(new HSeparatorBox(new QLabel(label0[i])), info.row, info.column, 1, 4);
    }

    static const char* label1[] = {
        _("mass [kg]"), _("color"), _("size(x-y-z) [m, m, m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]")
    };

    for(int i = 0; i < 22; ++i) {
        LabelInfo info = labelInfo[i];
        gbox[info.page]->addWidget(new QLabel(label1[i]), info.row, info.column);
    }

    static const char* label2[] = { _("Track Belt"), _("SubTrack Belt") };

    for(int i = 0; i < 2; ++i) {
        Info info = agxseparatorInfo[i];
        agbox->addLayout(new HSeparatorBox(new QLabel(label2[i])), info.row, info.column, 1, 6);
    }

    static const char* label3[] = {
        _("number of nodes [-]"), _("node thickness [m]"),
        _("node width [m]"), _("node thickerthickness [m]"),
        _("use thicker node every [-]"), _("node distance tension [m, e-]"),
        _("stabilizing hinge friction parameter [-, e-]"), _("min stabilizing hinge normal force [N]"),
        _("hinge compliance [rad/Nm, e-]"), _("hinge spook damping [s]"),
        _("nodes to wheels merge threshold [-]"), _("nodes to wheels split threshold [-]")
    };

    for(int i = 0; i < 24; ++i) {
        LabelInfo info = agxlabelInfo[i];
        agbox->addWidget(new QLabel(label3[i % 12]), info.row, info.column);
    }

    static const char* label4[] = { _("Front SubTrack"), _("Rear SubTrack"), _("AGX") };

    auto hbox1 = new QHBoxLayout;
    for(int i = 0; i < NUM_CHECKS; ++i) {
        CheckInfo info = checkInfo[i];
        checks[i] = new CheckBox;
        CheckBox* check = checks[i];
        check->setText(label4[i]);
        hbox1->addWidget(check);
    }
    hbox1->addStretch();

    auto hbox = new QHBoxLayout;
    setting1Radio.setText(_("Setting 1"));
    setting1Radio.setChecked(true);
    hbox->addWidget(&setting1Radio);
    settingGroup.addButton(&setting1Radio, 0);

    setting2Radio.setText(_("Setting 2"));
    hbox->addWidget(&setting2Radio);
    settingGroup.addButton(&setting2Radio, 1);
    hbox->addStretch();

    settingGroup.sigButtonToggled().connect(
        [&](int id, bool checked){ onButtonToggled(id, checked); });

    hbox->addStretch();
    static const char* tlabel[] = { _("&New"), _("&Import"), _("&Export") };
    static const char* name[] = { "document-new", "document-open", "document-save" };
    for(int i = 0; i < NUM_TBUTTONS; ++i) {
        toolButtons[i] = new PushButton;
        PushButton* button = toolButtons[i];
        const QIcon icon = QIcon::fromTheme(name[i]);
        if(icon.isNull()) {
            button->setText(tlabel[i]);
        } else {
            button->setIcon(icon);
        }
        hbox->addWidget(button);
    }

    formWidget = new FileFormWidget;

    initialize();

    Widget* page1Widget = new Widget;
    auto vbox1 = new QVBoxLayout;
    vbox1->addLayout(gbox[0]);
    vbox1->addStretch();
    page1Widget->setLayout(vbox1);

    Widget* page2Widget = new Widget;
    auto vbox2 = new QVBoxLayout;
    vbox2->addLayout(hbox1);
    vbox2->addLayout(gbox[1]);
    vbox2->addStretch();
    page2Widget->setLayout(vbox2);

    Widget* page3Widget = new Widget;
    auto vbox3 = new QVBoxLayout;
    vbox3->addLayout(agbox);
    vbox3->addStretch();
    page3Widget->setLayout(vbox3);

    topWidget = new QStackedWidget;
    topWidget->addWidget(page1Widget);
    topWidget->addWidget(page2Widget);
    // topWidget->addWidget(page3Widget);

    auto vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(topWidget);
    vbox->addWidget(new HSeparator);
    vbox->addWidget(formWidget);
    setLayout(vbox);

    toolButtons[RESET]->sigClicked().connect([&](){ onResetButtonClicked(); });
    toolButtons[IMPORT]->sigClicked().connect([&](){ onImportButtonClicked(); });
    toolButtons[EXPORT]->sigClicked().connect([&](){ onExportButtonClicked(); });
    checks[AGX_CHK]->sigToggled().connect([&](bool checked){ onEnableAGXCheckToggled(checked); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


CrawlerGenerator::~CrawlerGenerator()
{
    delete impl;
}


bool CrawlerGenerator::Impl::save(const string& filename)
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


bool CrawlerGenerator::Impl::save2(const string& filename)
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


void CrawlerGenerator::Impl::initialize()
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
        ColorButton* button = buttons[i];
        button->setColor(Vector3(info.red, info.green, info.blue));
    }

    for(int i = 0; i < NUM_CHECKS; ++i) {
        CheckInfo info = checkInfo[i];
        checks[i]->setChecked(info.checked);
    }
}


void CrawlerGenerator::Impl::onResetButtonClicked()
{
    initialize();
}


void CrawlerGenerator::Impl::onImportButtonClicked()
{
    string filename = getOpenFileName(_("Load a configuration file"), "yaml;yml");

    if(!filename.empty()) {
        load2(filename);
    }
}


bool CrawlerGenerator::Impl::load2(const string& filename, std::ostream& os)
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

                    auto& buttonList = *info->findListing("button");
                    if(buttonList.isValid()) {
                        for(int j = 0; j < buttonList.size(); ++j) {
                            Listing& colorList = *buttonList[j].toListing();
                            if(colorList.isValid()) {
                                Vector3 color;
                                for(int k = 0; k < colorList.size(); ++k) {
                                    color[k] = colorList[k].toDouble();
                                }
                                buttons[j]->setColor(color);
                            }
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


void CrawlerGenerator::Impl::onExportButtonClicked()
{
    string filename = getSaveFileName(_("Save a configuration file"), "yaml;yml");

    if(!filename.empty()) {
       filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".yaml") {
            filename += ".yaml";
        }
        save2(filename);
    }
}


void CrawlerGenerator::Impl::onEnableAGXCheckToggled(bool checked)
{
    agxdspins[TRK_BNT]->setEnabled(checked);
    agxdspins[TRK_BNW]->setEnabled(checked);
    agxdspins[TRK_BNTT]->setEnabled(checked);
    agxdspins[TRK_BNDTM]->setEnabled(checked);
    agxdspins[TRK_BSHFPM]->setEnabled(checked);

    agxdspins[TRK_BHCM]->setEnabled(checked);
    agxdspins[TRK_BHSD]->setEnabled(checked);
    agxdspins[TRK_BNWMT]->setEnabled(checked);
    agxdspins[TRK_BNWST]->setEnabled(checked);

    agxdspins[FLP_BNT]->setEnabled(checked);
    agxdspins[FLP_BNW]->setEnabled(checked);
    agxdspins[FLP_BNTT]->setEnabled(checked);
    agxdspins[FLP_BNDTM]->setEnabled(checked);
    agxdspins[FLP_BSHFPM]->setEnabled(checked);

    agxdspins[FLP_BHCM]->setEnabled(checked);
    agxdspins[FLP_BHSD]->setEnabled(checked);
    agxdspins[FLP_BNWMT]->setEnabled(checked);
    agxdspins[FLP_BNWST]->setEnabled(checked);

    agxspins[TRK_BNN]->setEnabled(checked);
    agxspins[TRK_BUTNE]->setEnabled(checked);
    agxspins[TRK_BNDTE]->setEnabled(checked);
    agxspins[TRK_BSHFPE]->setEnabled(checked);
    agxspins[TRK_BMSHNF]->setEnabled(checked);
    agxspins[TRK_BHCE]->setEnabled(checked);

    agxspins[FLP_BNN]->setEnabled(checked);
    agxspins[FLP_BUTNE]->setEnabled(checked);
    agxspins[FLP_BNDTE]->setEnabled(checked);
    agxspins[FLP_BSHFPE]->setEnabled(checked);
    agxspins[FLP_BMSHNF]->setEnabled(checked);
    agxspins[FLP_BHCE]->setEnabled(checked);
}


void CrawlerGenerator::Impl::onButtonToggled(const int& id, bool checked)
{
    if(checked) {
        topWidget->setCurrentIndex(id);
    }
}


MappingPtr CrawlerGenerator::Impl::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    string name = filesystem::path(fromUTF8(filename)).stem().string();
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


MappingPtr CrawlerGenerator::Impl::writeConfig(const string& filename)
{
    MappingPtr node = new Mapping;

    string name = filesystem::path(fromUTF8(filename)).stem().string();

    node->write("format", "CrawlerRobotGeneratorYaml");
    node->write("formatVersion", "1.0");
    node->write("name", name);

    ListingPtr configsNode = new Listing;
    {
        MappingPtr node = new Mapping;

        auto doubleSpinList = node->createFlowStyleListing("doubleSpin");
        int n = NUM_DSPINS;
        for(int i = 0; i < NUM_DSPINS; ++i) {
            doubleSpinList->append(dspins[i]->value()), n, n;
        }

        auto spinList = node->createFlowStyleListing("spin");
        int n1 = NUM_SPINS;
        for(int i = 0; i < NUM_SPINS; ++i) {
            spinList->append(agxspins[i]->value(), n1, n1);
        }

        int n2 = NUM_BUTTONS;
        auto buttonList = node->createFlowStyleListing("button");
        for(int i = 0; i < NUM_BUTTONS; ++i) {
            ListingPtr colorList = new Listing;
            for(int j = 0; j < 3; ++j) {
                colorList->append(buttons[i]->color()[j], 5, 5);
            }
            buttonList->append(colorList);
        }

        auto checkList = node->createFlowStyleListing("check");
        int n3 = NUM_CHECKS;
        for(int i = 0; i < NUM_CHECKS; ++i) {
            checkList->append(checks[i]->isChecked() ? 1 : 0, n3, n3);
        }

        configsNode->append(node);
    }

    if(!configsNode->empty()) {
        node->insert("configs", configsNode);
    }

    return node;
}


void CrawlerGenerator::Impl::writeBody(Listing* linksNode)
{
    int jointID = 0;

    MappingPtr trackNode = writeTrack();
    MappingPtr spacerNode = writeSpacer();
    MappingPtr frontTrackNode = writeFrontTrack();
    MappingPtr rearTrackNode = writeRearTrack();

    {
        static const char* name[] = { "TRACK_L", "TRACK_R" };
        Vector3 translation[2];
        // translation[0] = Vector3(0.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0, (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);

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
            // translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), 0.0);

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
            // translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), 0.0);

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


MappingPtr CrawlerGenerator::Impl::writeChassis()
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
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", buttons[CHS_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        chassisNode->insert("elements", elementsNode);
    }

    return chassisNode;
}


MappingPtr CrawlerGenerator::Impl::writeSpacer()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "revolute");
    node->write("jointAxis", "-Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[SPC_MAS]->value());
    write(node, "inertia", calcCylinderInertia(dspins[SPC_MAS]->value(), dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[SPC_RAD]->value(), dspins[SPC_WDT]->value(), buttons[SPC_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeTrack()
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
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", buttons[TRK_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerGenerator::Impl::writeFrontTrack()
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
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", buttons[FFL_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerGenerator::Impl::writeRearTrack()
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
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", buttons[RFL_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


void CrawlerGenerator::Impl::writeAGXBody(Listing* linksNode)
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
        // translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, -dspins[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[TRK_WDT]->value()) / 2.0, 0.0);

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
            // translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), 0.0);

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
            // translation[0] = Vector3(dspins[FFL_WBS]->value() / 2.0,  (dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
            // translation[1] = Vector3(dspins[FFL_WBS]->value() / 2.0, -(dspins[SPC_WDT]->value() + dspins[FFL_WDT]->value()) / 2.0, 0.0);
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
            // translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), -dspins[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(-dspins[TRK_WBS]->value() / 2.0,  (dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 + dspins[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(-dspins[TRK_WBS]->value() / 2.0, -(dspins[CHS_YSZ]->value() + dspins[SPC_WDT]->value()) / 2.0 - dspins[TRK_WDT]->value(), 0.0);

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


MappingPtr CrawlerGenerator::Impl::writeAGXTrack()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[TRK_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[TRK_MAS]->value() / 3.0, dspins[TRK_WBS]->value(), dspins[TRK_WDT]->value(), dspins[TRK_RAD]->value() * 2.0));

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxspins[TRK_BNN]->value());
    node->write("nodeThickness", agxdspins[TRK_BNT]->value());
    node->write("nodeWidth", agxdspins[TRK_BNW]->value());
    node->write("nodeThickerThickness", agxdspins[TRK_BNTT]->value());
    node->write("useThickerNodeEvery", agxspins[TRK_BUTNE]->value());
    node->write("material", "CrawlerTracks");
    node->write("nodeDistanceTension", agxdspins[TRK_BNDTM]->value() * exp10(-agxspins[TRK_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxdspins[TRK_BSHFPM]->value() * exp10(-agxspins[TRK_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxspins[TRK_BMSHNF]->value());
    node->write("hingeCompliance", agxdspins[TRK_BHCM]->value() * exp10(-agxspins[TRK_BHCE]->value()));
    node->write("hingeSpookDamping", agxdspins[TRK_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxdspins[TRK_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxdspins[TRK_BNWST]->value());

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXSprocket()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->insert(writeAGXWheel());
    node->write("mass", dspins[TRK_MAS]->value() * 2.0 / 9.0);
    write(node, "inertia", calcCylinderInertia(dspins[TRK_MAS]->value(), dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(dspins[TRK_RAD]->value(), dspins[TRK_WDT]->value(), buttons[TRK_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXRoller()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerGenerator::Impl::writeAGXIdler()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerGenerator::Impl::writeAGXFrontTrack()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[FFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[FFL_MAS]->value() / 3.0, dspins[FFL_WBS]->value(), dspins[FFL_WDT]->value(), std::max(dspins[FFL_FRD]->value(), dspins[FFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXRearTrack()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", dspins[RFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(dspins[RFL_MAS]->value() / 3.0, dspins[RFL_WBS]->value(), dspins[RFL_WDT]->value(), std::max(dspins[RFL_FRD]->value(), dspins[RFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXSubTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxspins[FLP_BNN]->value());
    node->write("nodeThickness", agxdspins[FLP_BNT]->value());
    node->write("nodeWidth", agxdspins[FLP_BNW]->value());
    node->write("nodeThickerThickness", agxdspins[FLP_BNTT]->value());
    node->write("useThickerNodeEvery", agxspins[FLP_BUTNE]->value());
    node->write("material", "CrawlerTracks");
    node->write("nodeDistanceTension", agxdspins[FLP_BNDTM]->value() * exp10(-agxspins[FLP_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxdspins[FLP_BSHFPM]->value() * exp10(-agxspins[FLP_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxspins[FLP_BMSHNF]->value());
    node->write("hingeCompliance", agxdspins[FLP_BHCM]->value() * exp10(-agxspins[FLP_BHCE]->value()));
    node->write("hingeSpookDamping", agxdspins[FLP_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxdspins[FLP_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxdspins[FLP_BNWST]->value());

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXFrontSprocket()
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

    elementsNode->append(writeCylinderShape(dspins[FFL_RRD]->value(), dspins[FFL_WDT]->value(), buttons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXFrontRoller()
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

    elementsNode->append(writeCylinderShape((dspins[FFL_RRD]->value() + dspins[FFL_FRD]->value()) / 2.0, dspins[FFL_WDT]->value(), buttons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXFrontIdler()
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

    elementsNode->append(writeCylinderShape(dspins[FFL_FRD]->value(), dspins[FFL_WDT]->value(), buttons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXRearSprocket()
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

    elementsNode->append(writeCylinderShape(dspins[RFL_FRD]->value(), dspins[RFL_WDT]->value(), buttons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXRearRoller()
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

    elementsNode->append(writeCylinderShape((dspins[RFL_RRD]->value() + dspins[RFL_FRD]->value()) / 2.0, dspins[RFL_WDT]->value(), buttons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXRearIdler()
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

    elementsNode->append(writeCylinderShape(dspins[RFL_RRD]->value(), dspins[RFL_WDT]->value(), buttons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXWheel()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "revolute");
    node->write("jointAxis", "Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("material", "CrawlerWheel");

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeCylinderShape(const double& radius, const double& height, const Vector3& color)
{
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Cylinder");
    geometryNode->write("radius", radius);
    geometryNode->write("height", height);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", color);

    return node;
}


MappingPtr CrawlerGenerator::Impl::writeAGXVehicleContinuousTrackDevice(const char* name, const char* sprocketName, const char* rollerName, const char* idlerName, const bool isSubTrack)
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
