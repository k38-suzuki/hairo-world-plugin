/**
    @author Kenta Suzuki
*/

#include "BodyCreator.h"
#include <cnoid/Button>
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
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QWidget>
#include "BodyCreatorDialog.h"
#include "ColorButton.h"
#include "CreatorToolBar.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

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
    int group;
};

ButtonInfo buttonInfo[] = {
    { 0, 3,   0.0 / 255.0, 153.0 / 255.0,  0.0 / 255.0, 0 },
    { 0, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 1 },
    { 0, 3, 255.0 / 255.0,   0.0 / 255.0,  0.0 / 255.0, 2 },
    { 0, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 3 },
    { 0, 3,  51.0 / 255.0,  51.0 / 255.0, 51.0 / 255.0, 4 }
};

struct DoubleSpinInfo {
    int row;
    int column;
    double value;
    double min;
    double max;
    int decimals;
    int group;
};

DoubleSpinInfo doubleSpinInfo[] = {
    { 0, 1, 8.000, 0.0, 1000.000, 3, 0 }, { 1, 1, 0.450, 0.0, 1000.000, 3, 0 }, { 1, 2, 0.300, 0.0, 1000.000, 3, 0 }, { 1, 3, 0.100, 0.0, 1000.000, 3, 0 },
    { 0, 1, 1.000, 0.0, 1000.000, 3, 1 }, { 1, 1, 0.080, 0.0, 1000.000, 3, 1 }, { 2, 1, 0.100, 0.0, 1000.000, 3, 1 }, { 2, 3, 0.420, 0.0, 1000.000, 3, 1 },
    { 0, 1, 0.200, 0.0, 1000.000, 3, 2 }, { 1, 1, 0.060, 0.0, 1000.000, 3, 2 }, { 2, 1, 0.013, 0.0, 1000.000, 3, 2 },
    { 0, 1, 0.250, 0.0, 1000.000, 3, 3 }, { 1, 1, 0.080, 0.0, 1000.000, 3, 2 }, { 1, 2, 0.080, 0.0, 1000.000, 3, 3 }, { 2, 1, 0.080, 0.0, 1000.000, 3, 3 }, { 2, 3, 0.130, 0.0, 1000.000, 3, 3 },
    { 0, 1, 0.250, 0.0, 1000.000, 3, 4 }, { 1, 1, 0.080, 0.0, 1000.000, 3, 3 }, { 1, 2, 0.080, 0.0, 1000.000, 3, 4 }, { 2, 1, 0.080, 0.0, 1000.000, 3, 4 }, { 2, 3, 0.130, 0.0, 1000.000, 3, 4 }
};

DoubleSpinInfo agxdoubleSpinInfo[] = {
    { 0, 4, 0.010, 0.0, 1000.000, 3, 0 }, { 1, 1, 0.100, 0.0, 1000.000, 3, 0 }, { 1, 4,  0.020,       0.0, 1000.000, 3, 0 }, { 2, 4,  9.000,       0.0, 1000.000, 3, 0 }, { 3, 1, 4.000, 0.0, 1000.000, 3, 0 },
    { 4, 1, 9.000, 0.0, 1000.000, 3, 0 }, { 4, 4, 0.010, 0.0, 1000.000, 3, 0 }, { 5, 1, -0.001, -1000.000, 1000.000, 3, 0 }, { 5, 4, -0.009, -1000.000, 1000.000, 3, 0 },
    { 0, 4, 0.010, 0.0, 1000.000, 3, 1 }, { 1, 1, 0.080, 0.0, 1000.000, 3, 1 }, { 1, 4,  0.020,       0.0, 1000.000, 3, 1 }, { 2, 4,  9.000,       0.0, 1000.000, 3, 1 }, { 3, 1, 4.000, 0.0, 1000.000, 3, 1 },
    { 4, 1, 9.000, 0.0, 1000.000, 3, 1 }, { 4, 4, 0.010, 0.0, 1000.000, 3, 1 }, { 5, 1, -0.001, -1000.000, 1000.000, 3, 1 }, { 5, 4, -0.009, -1000.000, 1000.000, 3, 1 }
};

struct SpinInfo {
    int row;
    int column;
    double value;
    double min;
    double max;
    int group;
};

SpinInfo agxspinInfo[] = {
    { 0, 1, 42, 0, 9999, 0 }, { 2, 1,   3, 0, 9999, 0 }, { 2, 5, 4, 0, 9999, 0 },
    { 3, 2,  7, 0, 9999, 0 }, { 3, 4, 100, 0, 9999, 0 }, { 4, 2, 7, 0, 9999, 0 },
    { 0, 1, 42, 0, 9999, 1 }, { 2, 1,   3, 0, 9999, 1 }, { 2, 5, 4, 0, 9999, 1 },
    { 3, 2,  7, 0, 9999, 1 }, { 3, 4, 100, 0, 9999, 1 }, { 4, 2, 7, 0, 9999, 1 }
};

struct LabelInfo {
    int row;
    int column;
    int group;
};

LabelInfo labelInfo[] = {
    { 0, 0, 0 }, { 0, 2, 0 }, { 1, 0, 0 },
    { 0, 0, 1 }, { 0, 2, 1 }, { 1, 0, 1 }, { 2, 0, 1 }, { 2, 2, 1 },
    { 0, 0, 2 }, { 0, 2, 2 }, { 1, 0, 2 }, { 2, 0, 2 },
    { 0, 0, 3 }, { 0, 2, 3 }, { 1, 0, 3 }, { 2, 0, 3 }, { 2, 2, 3 },
    { 0, 0, 4 }, { 0, 2, 4 }, { 1, 0, 4 }, { 2, 0, 4 }, { 2, 2, 4 }
};

LabelInfo agxlabelInfo[] = {
    { 0, 0, 0 }, { 0, 3, 0 },
    { 1, 0, 0 }, { 1, 3, 0 },
    { 2, 0, 0 }, { 2, 3, 0 },
    { 3, 0, 0 }, { 3, 3, 0 },
    { 4, 0, 0 }, { 4, 3, 0 },
    { 5, 0, 0 }, { 5, 3, 0 },
    { 0, 0, 1 }, { 0, 3, 1 },
    { 1, 0, 1 }, { 1, 3, 1 },
    { 2, 0, 1 }, { 2, 3, 1 },
    { 3, 0, 1 }, { 3, 3, 1 },
    { 4, 0, 1 }, { 4, 3, 1 },
    { 5, 0, 1 }, { 5, 3, 1 }
};

struct AGXVehicleContinuousTrackDeviceInfo {
    const char* name;
    const char* sprocketName;
    const char* rollerName;
    const char* idlerName;
    bool isSubTrack;
};

class CrawlerCreatorWidget : public QWidget
{
public:
    CrawlerCreatorWidget(QWidget* parent = nullptr);

private:
    bool save(const string& filename);
    bool save2(const string& filename);

    void initialize();
    void on_exportButton_clicked();
    void on_importButton_clicked();

    bool load2(const string& filename, ostream& os = nullout());

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

    enum {
        CHS_MAS, CHS_XSZ, CHS_YSZ, CHS_ZSZ,
        TRK_MAS, TRK_RAD, TRK_WDT, TRK_WBS,
        SPC_MAS, SPC_RAD, SPC_WDT,
        FFL_MAS, FFL_FRD, FFL_RRD, FFL_WDT, FFL_WBS,
        RFL_MAS, RFL_FRD, RFL_RRD, RFL_WDT, RFL_WBS,
        NumDoubleSpinBoxes
    };
    enum {
        CHS_CLR, TRK_CLR, SPC_CLR,
        FFL_CLR, RFL_CLR, NumColorButtons
    };
    enum {
        TRK_BNT, TRK_BNW, TRK_BNTT, TRK_BNDTM, TRK_BSHFPM,
        TRK_BHCM, TRK_BHSD, TRK_BNWMT, TRK_BNWST,
        FLP_BNT, FLP_BNW, FLP_BNTT, FLP_BNDTM, FLP_BSHFPM,
        FLP_BHCM, FLP_BHSD, FLP_BNWMT, FLP_BNWST,
        NumAGXDoubleSpinBoxes
    };
    enum {
        TRK_BNN, TRK_BUTNE, TRK_BNDTE,
        TRK_BSHFPE, TRK_BMSHNF, TRK_BHCE,
        FLP_BNN, FLP_BUTNE, FLP_BNDTE,
        FLP_BSHFPE, FLP_BMSHNF, FLP_BHCE,
        NumAGXSpinBoxes
    };

    ColorButton* colorButtons[NumColorButtons];
    DoubleSpinBox* doubleSpinBoxes[NumDoubleSpinBoxes];
    DoubleSpinBox* agxDoubleSpinBoxes[NumAGXDoubleSpinBoxes];
    SpinBox* agxSpinBoxes[NumAGXSpinBoxes];
    QGridLayout* gridLayouts[6];
    QGroupBox* groupBoxes[6];
    YAMLWriter yamlWriter;
    YAMLWriter yamlWriter2;
};

}


void CrawlerCreator::initializeClass(ExtensionManager* ext)
{
    static CrawlerCreatorWidget* widget = nullptr;

    if(!widget) {
        widget = ext->manage(new CrawlerCreatorWidget);

        BodyCreatorDialog::instance()->listedWidget()->addWidget(_("Crawler"), widget);
    }
}


CrawlerCreator::CrawlerCreator()
{

}


CrawlerCreator::~CrawlerCreator()
{

}


CrawlerCreatorWidget::CrawlerCreatorWidget(QWidget* parent)
    : QWidget(parent)
{
    yamlWriter.setKeyOrderPreservationMode(true);
    yamlWriter2.setKeyOrderPreservationMode(true);

    const QStringList list = {
        _("Chassis"), _("Track"), _("Spacer"), _("Front SubTrack"), _("Rear SubTrack"), _("AGX")
    };

    for(int i = 0; i < 6; ++i) {
        gridLayouts[i] = new QGridLayout;
        groupBoxes[i] = new QGroupBox(list.at(i));
        if(i > 2) {
            groupBoxes[i]->setCheckable(true);
        }
        auto layout = new QVBoxLayout;
        layout->addLayout(gridLayouts[i]);
        if(i != 5) {
            groupBoxes[i]->setLayout(layout);
        }
    }

    const QStringList list2 = {
        _("mass [kg]"), _("color"), _("size(x-y-z) [m, m, m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius [m]"), _("width [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]"),
        _("mass [kg]"), _("color"), _("radius(forward-backward) [m]"), _("width [m]"), _("wheelbase [m]")
    };

    for(int i = 0; i < 22; ++i) {
        LabelInfo& info = labelInfo[i];
        gridLayouts[info.group]->addWidget(new QLabel(list2[i]), info.row, info.column);
    }

    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = doubleSpinInfo[i];
        doubleSpinBoxes[i] = new DoubleSpinBox;
        gridLayouts[info.group]->addWidget(doubleSpinBoxes[i], info.row, info.column);
    }

    for(int i = 0; i < NumColorButtons; ++i) {
        ButtonInfo& info = buttonInfo[i];
        colorButtons[i] = new ColorButton;
        gridLayouts[info.group]->addWidget(colorButtons[i], info.row, info.column);
    }

    // for AGX configuration
    const QStringList list3 = { _("Track Belt"), _("SubTrack Belt") };

    QGroupBox* groupBoxes2[2];
    QGridLayout* gridLayouts2[2];

    for(int i = 0; i < 2; ++i) {
        gridLayouts2[i] = new QGridLayout;
        groupBoxes2[i] = new QGroupBox(list3.at(i));
        groupBoxes2[i]->setLayout(gridLayouts2[i]);
    }

    auto layout = new QVBoxLayout;
    layout->addWidget(groupBoxes2[0]);
    layout->addWidget(groupBoxes2[1]);
    layout->addStretch();
    groupBoxes[5]->setLayout(layout);

    const QStringList list4 = {
        _("number of nodes [-]"), _("node thickness [m]"),
        _("node width [m]"), _("node thickerthickness [m]"),
        _("use thicker node every [-]"), _("node distance tension [m, e-]"),
        _("stabilizing hinge friction parameter [-, e-]"), _("min stabilizing hinge normal force [N]"),
        _("hinge compliance [rad/Nm, e-]"), _("hinge spook damping [s]"),
        _("nodes to wheels merge threshold [-]"), _("nodes to wheels split threshold [-]")
    };

    for(int i = 0; i < 24; ++i) {
        LabelInfo& info = agxlabelInfo[i];
        gridLayouts2[info.group]->addWidget(new QLabel(list4[i % 12]), info.row, info.column);
    }

    for(int i = 0; i < NumAGXDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = agxdoubleSpinInfo[i];
        agxDoubleSpinBoxes[i] = new DoubleSpinBox;
        gridLayouts2[info.group]->addWidget(agxDoubleSpinBoxes[i], info.row, info.column);
    }

    for(int i = 0; i < NumAGXSpinBoxes; ++i) {
        SpinInfo& info = agxspinInfo[i];
        agxSpinBoxes[i] = new SpinBox;
        gridLayouts2[info.group]->addWidget(agxSpinBoxes[i], info.row, info.column);
    }

    initialize();

    auto page1 = new QWidget;
    layout = new QVBoxLayout;
    layout->addWidget(groupBoxes[0]);
    layout->addWidget(groupBoxes[1]);
    layout->addStretch();
    page1->setLayout(layout);

    auto page2 = new QWidget;
    layout = new QVBoxLayout;
    layout->addWidget(groupBoxes[2]);
    layout->addWidget(groupBoxes[3]);
    layout->addWidget(groupBoxes[4]);
    layout->addStretch();
    page2->setLayout(layout);

    auto page3 = new QWidget;
    layout = new QVBoxLayout;
    layout->addWidget(groupBoxes[5]);
    layout->addStretch();
    page3->setLayout(layout);

    auto tabWidget = new QTabWidget;
    tabWidget->addTab(page1, _("Setting 1"));
    tabWidget->addTab(page2, _("Setting 2"));
    tabWidget->addTab(page3, _("Setting 3"));

    auto toolBar = new CreatorToolBar;
    toolBar->sigNewRequested().connect([&](){ initialize(); });
    toolBar->sigSaveRequested().connect([&](const string& filename){ save(filename); });

    for(int i = 0; i < 2; ++i) {
        const QIcon documentIcon = QIcon::fromTheme(i == 0 ? "document-open" : "document-save");
        auto documentButton = toolBar->addButton(documentIcon, i == 0 ? _("&Import") : _("&Export"));
        connect(documentButton, &QPushButton::clicked, [&, i](){ i == 0 ? on_importButton_clicked() : on_exportButton_clicked(); });
    }

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle(_("CrawlerRobot Generator"));
}


bool CrawlerCreatorWidget::save(const string& filename)
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


bool CrawlerCreatorWidget::save2(const string& filename)
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


void CrawlerCreatorWidget::initialize()
{
    for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = doubleSpinInfo[i];
        DoubleSpinBox* spin = doubleSpinBoxes[i];
        spin->setDecimals(info.decimals);
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
    }

    for(int i = 0; i < NumAGXDoubleSpinBoxes; ++i) {
        DoubleSpinInfo& info = agxdoubleSpinInfo[i];
        DoubleSpinBox* spin = agxDoubleSpinBoxes[i];
        spin->setDecimals(info.decimals);
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
    }

    for(int i = 0; i < NumAGXSpinBoxes; ++i) {
        SpinInfo& info = agxspinInfo[i];
        SpinBox* spin = agxSpinBoxes[i];
        spin->setRange(info.min, info.max);
        spin->setValue(info.value);
    }

    for(int i = 0; i < NumColorButtons; ++i) {
        ButtonInfo& info = buttonInfo[i];
        ColorButton* button = colorButtons[i];
        button->setColor(Vector3(info.red, info.green, info.blue));
    }

    for(int i = 0; i < 3; ++i) {
        CheckInfo& info = checkInfo[i];
        groupBoxes[i + 3]->setChecked(info.checked);
    }
}


void CrawlerCreatorWidget::on_importButton_clicked()
{
    string filename = getOpenFileName(_("YAML File"), "yaml");

    if(!filename.empty()) {
        load2(filename);
    }
}


bool CrawlerCreatorWidget::load2(const string& filename, std::ostream& os)
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
                            doubleSpinBoxes[j]->setValue(value);
                        }
                    }

                    auto& spinList = *info->findListing("spin");
                    if(spinList.isValid()) {
                        for(int j = 0; j < spinList.size(); ++j) {
                            int value = spinList[j].toInt();
                            agxSpinBoxes[j]->setValue(value);
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
                                colorButtons[j]->setColor(color);
                            }
                        }
                    }

                    auto& checkList = *info->findListing("check");
                    if(checkList.isValid()) {
                        for(int j = 0; j < checkList.size(); ++j) {
                            bool checked = checkList[j].toInt() == 0 ? false : true;
                            groupBoxes[j + 3]->setChecked(checked);
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


void CrawlerCreatorWidget::on_exportButton_clicked()
{
    string filename = getSaveFileName(_("YAML File"), "yaml");

    if(!filename.empty()) {
       filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".yaml") {
            filename += ".yaml";
        }
        save2(filename);
    }
}


MappingPtr CrawlerCreatorWidget::writeBody(const string& filename)
{
    MappingPtr node = new Mapping;

    string name = filesystem::path(fromUTF8(filename)).stem().string();
    bool isAGXChecked = groupBoxes[5]->isChecked();

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


MappingPtr CrawlerCreatorWidget::writeConfig(const string& filename)
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
        int n = NumDoubleSpinBoxes;
        for(int i = 0; i < NumDoubleSpinBoxes; ++i) {
            doubleSpinList->append(doubleSpinBoxes[i]->value()), n, n;
        }

        auto spinList = node->createFlowStyleListing("spin");
        int n1 = NumAGXSpinBoxes;
        for(int i = 0; i < NumAGXSpinBoxes; ++i) {
            spinList->append(agxSpinBoxes[i]->value(), n1, n1);
        }

        int n2 = NumColorButtons;
        auto buttonList = node->createFlowStyleListing("button");
        for(int i = 0; i < NumColorButtons; ++i) {
            ListingPtr colorList = new Listing;
            for(int j = 0; j < 3; ++j) {
                colorList->append(colorButtons[i]->color()[j], 5, 5);
            }
            buttonList->append(colorList);
        }

        auto checkList = node->createFlowStyleListing("check");
        int n3 = 3;
        for(int i = 0; i < 3; ++i) {
            checkList->append(groupBoxes[i + 3]->isChecked() ? 1 : 0, n3, n3);
        }

        configsNode->append(node);
    }

    if(!configsNode->empty()) {
        node->insert("configs", configsNode);
    }

    return node;
}


void CrawlerCreatorWidget::writeBody(Listing* linksNode)
{
    int jointID = 0;

    MappingPtr trackNode = writeTrack();
    MappingPtr spacerNode = writeSpacer();
    MappingPtr frontTrackNode = writeFrontTrack();
    MappingPtr rearTrackNode = writeRearTrack();

    {
        static const char* name[] = { "TRACK_L", "TRACK_R" };
        Vector3 translation[2];
        // translation[0] = Vector3(0.0, (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0, (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);

        for(int i = 0; i < 2; ++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->insert(trackNode);

            linksNode->append(node);
        }
    }

    if(groupBoxes[3]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            // translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), 0.0);

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
            translation[0] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);

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

    if(groupBoxes[4]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            // translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), 0.0);

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
            translation[0] = Vector3(-doubleSpinBoxes[RFL_WBS]->value() / 2.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-doubleSpinBoxes[RFL_WBS]->value() / 2.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);

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


MappingPtr CrawlerCreatorWidget::writeChassis()
{
    MappingPtr chassisNode = new Mapping;

    chassisNode->write("name", "CHASSIS");
    write(chassisNode, "translation", Vector3(0.0, 0.0, 0.0));
    chassisNode->write("jointType", "free");
    write(chassisNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    chassisNode->write("mass", doubleSpinBoxes[CHS_MAS]->value());
    write(chassisNode, "inertia", calcBoxInertia(doubleSpinBoxes[CHS_MAS]->value(), doubleSpinBoxes[CHS_XSZ]->value(), doubleSpinBoxes[CHS_YSZ]->value(), doubleSpinBoxes[CHS_ZSZ]->value()));

    ListingPtr elementsNode = new Listing;
    MappingPtr node = new Mapping;

    node->write("type", "Shape");

    MappingPtr geometryNode = node->createFlowStyleMapping("geometry");
    geometryNode->write("type", "Box");
    write(geometryNode, "size", Vector3(doubleSpinBoxes[CHS_XSZ]->value(), doubleSpinBoxes[CHS_YSZ]->value(), doubleSpinBoxes[CHS_ZSZ]->value()));

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", colorButtons[CHS_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        chassisNode->insert("elements", elementsNode);
    }

    return chassisNode;
}


MappingPtr CrawlerCreatorWidget::writeSpacer()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "revolute");
    node->write("jointAxis", "-Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", doubleSpinBoxes[SPC_MAS]->value());
    write(node, "inertia", calcCylinderInertia(doubleSpinBoxes[SPC_MAS]->value(), doubleSpinBoxes[SPC_RAD]->value(), doubleSpinBoxes[SPC_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[SPC_RAD]->value(), doubleSpinBoxes[SPC_WDT]->value(), colorButtons[SPC_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeTrack()
{
    MappingPtr trackNode = new Mapping;
    trackNode->write("parent", "CHASSIS");
    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", doubleSpinBoxes[TRK_MAS]->value());
    write(trackNode, "inertia", calcBoxInertia(doubleSpinBoxes[TRK_MAS]->value(), doubleSpinBoxes[TRK_WBS]->value(), doubleSpinBoxes[TRK_WDT]->value(), doubleSpinBoxes[TRK_RAD]->value() * 2.0));

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
        crossSectionList.append(doubleSpinBoxes[TRK_WBS]->value() / 2.0 + doubleSpinBoxes[TRK_RAD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[TRK_RAD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; ++i) {
        crossSectionList.append(-doubleSpinBoxes[TRK_WBS]->value() / 2.0 + doubleSpinBoxes[TRK_RAD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[TRK_RAD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; ++i) {
        crossSectionList.append(doubleSpinBoxes[TRK_WBS]->value() / 2.0 + doubleSpinBoxes[TRK_RAD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[TRK_RAD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -doubleSpinBoxes[TRK_WDT]->value() / 2.0, 0.0, 0.0, doubleSpinBoxes[TRK_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", colorButtons[TRK_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerCreatorWidget::writeFrontTrack()
{
    MappingPtr trackNode = new Mapping;

    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", doubleSpinBoxes[FFL_MAS]->value());
    double radius = std::max(doubleSpinBoxes[FFL_FRD]->value(), doubleSpinBoxes[FFL_RRD]->value());
    write(trackNode, "inertia", calcBoxInertia(doubleSpinBoxes[FFL_MAS]->value(), doubleSpinBoxes[FFL_WBS]->value(), doubleSpinBoxes[FFL_WDT]->value(), radius * 2.0));

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
        crossSectionList.append(doubleSpinBoxes[FFL_WBS]->value() / 2.0 + doubleSpinBoxes[FFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[FFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(-doubleSpinBoxes[FFL_WBS]->value() / 2.0 + doubleSpinBoxes[FFL_RRD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[FFL_RRD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; i++) {
        crossSectionList.append(doubleSpinBoxes[FFL_WBS]->value() / 2.0 + doubleSpinBoxes[FFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[FFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -doubleSpinBoxes[FFL_WDT]->value() / 2.0, 0.0, 0.0, doubleSpinBoxes[FFL_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", colorButtons[FFL_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


MappingPtr CrawlerCreatorWidget::writeRearTrack()
{
    MappingPtr trackNode = new Mapping;

    trackNode->write("jointType", "pseudo_continuous_track");
    trackNode->write("jointAxis", "Y");
    write(trackNode, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    trackNode->write("mass", doubleSpinBoxes[RFL_MAS]->value());
    double radius = std::max(doubleSpinBoxes[RFL_FRD]->value(), doubleSpinBoxes[RFL_RRD]->value());
    write(trackNode, "inertia", calcBoxInertia(doubleSpinBoxes[RFL_MAS]->value(), doubleSpinBoxes[RFL_WBS]->value(), doubleSpinBoxes[RFL_WDT]->value(), radius * 2.0));

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
        crossSectionList.append(doubleSpinBoxes[RFL_WBS]->value() / 2.0 + doubleSpinBoxes[RFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[RFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < numPoints; i++) {
        crossSectionList.append(-doubleSpinBoxes[RFL_WBS]->value() / 2.0 + doubleSpinBoxes[RFL_RRD]->value() * cos(radian(90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[RFL_RRD]->value() * sin(radian(90.0 + pitchAngle * i)), 2, n);
    }
    for(int i = 0; i < 1; i++) {
        crossSectionList.append(doubleSpinBoxes[RFL_WBS]->value() / 2.0 + doubleSpinBoxes[RFL_FRD]->value() * cos(radian(-90.0 + pitchAngle * i)), 2, n);
        crossSectionList.append(doubleSpinBoxes[RFL_FRD]->value() * sin(radian(-90.0 + pitchAngle * i)), 2, n);
    }

    VectorXd spine(6);
    spine << 0.0, -doubleSpinBoxes[RFL_WDT]->value() / 2.0, 0.0, 0.0, doubleSpinBoxes[RFL_WDT]->value() / 2.0, 0.0;
    write(geometryNode, "spine", spine);

    node->insert("geometry", geometryNode);

    MappingPtr appearanceNode = node->createFlowStyleMapping("appearance");
    MappingPtr materialNode = appearanceNode->createFlowStyleMapping("material");
    write(materialNode, "diffuseColor", colorButtons[RFL_CLR]->color());

    elementsNode->append(node);
    if(!elementsNode->empty()) {
        trackNode->insert("elements", elementsNode);
    }

    return trackNode;
}


void CrawlerCreatorWidget::writeAGXBody(Listing* linksNode)
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
        // translation[0] = Vector3(0.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(0.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(0.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(0.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);

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
        // translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        // translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
        translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);
        translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[TRK_WDT]->value()) / 2.0, 0.0);

        for(int i = 0; i < 2;++i) {
            MappingPtr node = new Mapping;

            node->write("name", name[i]);
            write(node, "translation", translation[i]);
            node->write("jointId", jointID++);
            node->insert(agxIdlerNode);

            linksNode->append(node);
        }
    }

    if(groupBoxes[3]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LF", "SPACER_RF" };
            Vector3 translation[2];
            // translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), 0.0);

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
            translation[0] = Vector3(0.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);

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
            translation[0] = Vector3(0.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);

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
            // translation[0] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            // translation[1] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[0] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(doubleSpinBoxes[FFL_WBS]->value() / 2.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);

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
            translation[0] = Vector3(doubleSpinBoxes[FFL_WBS]->value(), (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(doubleSpinBoxes[FFL_WBS]->value(), -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[FFL_WDT]->value()) / 2.0, 0.0);

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


    if(groupBoxes[4]->isChecked()) {
        {
            static const char* name[] = { "SPACER_LR", "SPACER_RR" };
            Vector3 translation[2];
            // translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            // translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), -doubleSpinBoxes[CHS_ZSZ]->value() / 2.0);
            translation[0] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0,  (doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 + doubleSpinBoxes[TRK_WDT]->value(), 0.0);
            translation[1] = Vector3(-doubleSpinBoxes[TRK_WBS]->value() / 2.0, -(doubleSpinBoxes[CHS_YSZ]->value() + doubleSpinBoxes[SPC_WDT]->value()) / 2.0 - doubleSpinBoxes[TRK_WDT]->value(), 0.0);

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
            translation[0] = Vector3(0.0, (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);

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
            translation[0] = Vector3(0.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(0.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);

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
            translation[0] = Vector3(-doubleSpinBoxes[RFL_WBS]->value() / 2.0,  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-doubleSpinBoxes[RFL_WBS]->value() / 2.0, -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);

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
            translation[0] = Vector3(-doubleSpinBoxes[RFL_WBS]->value(),  (doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);
            translation[1] = Vector3(-doubleSpinBoxes[RFL_WBS]->value(), -(doubleSpinBoxes[SPC_WDT]->value() + doubleSpinBoxes[RFL_WDT]->value()) / 2.0, 0.0);

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


MappingPtr CrawlerCreatorWidget::writeAGXTrack()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", doubleSpinBoxes[TRK_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(doubleSpinBoxes[TRK_MAS]->value() / 3.0, doubleSpinBoxes[TRK_WBS]->value(), doubleSpinBoxes[TRK_WDT]->value(), doubleSpinBoxes[TRK_RAD]->value() * 2.0));

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxSpinBoxes[TRK_BNN]->value());
    node->write("nodeThickness", agxDoubleSpinBoxes[TRK_BNT]->value());
    node->write("nodeWidth", agxDoubleSpinBoxes[TRK_BNW]->value());
    node->write("nodeThickerThickness", agxDoubleSpinBoxes[TRK_BNTT]->value());
    node->write("useThickerNodeEvery", agxSpinBoxes[TRK_BUTNE]->value());
    node->write("material", "CrawlerTracks");
    node->write("nodeDistanceTension", agxDoubleSpinBoxes[TRK_BNDTM]->value() * exp10(-agxSpinBoxes[TRK_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxDoubleSpinBoxes[TRK_BSHFPM]->value() * exp10(-agxSpinBoxes[TRK_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxSpinBoxes[TRK_BMSHNF]->value());
    node->write("hingeCompliance", agxDoubleSpinBoxes[TRK_BHCM]->value() * exp10(-agxSpinBoxes[TRK_BHCE]->value()));
    node->write("hingeSpookDamping", agxDoubleSpinBoxes[TRK_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxDoubleSpinBoxes[TRK_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxDoubleSpinBoxes[TRK_BNWST]->value());

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXSprocket()
{
    MappingPtr node = new Mapping;

    node->write("parent", "CHASSIS");
    node->insert(writeAGXWheel());
    node->write("mass", doubleSpinBoxes[TRK_MAS]->value() * 2.0 / 9.0);
    write(node, "inertia", calcCylinderInertia(doubleSpinBoxes[TRK_MAS]->value(), doubleSpinBoxes[TRK_RAD]->value(), doubleSpinBoxes[TRK_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[TRK_RAD]->value(), doubleSpinBoxes[TRK_WDT]->value(), colorButtons[TRK_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXRoller()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerCreatorWidget::writeAGXIdler()
{
    return writeAGXSprocket();
}


MappingPtr CrawlerCreatorWidget::writeAGXFrontTrack()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", doubleSpinBoxes[FFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(doubleSpinBoxes[FFL_MAS]->value() / 3.0, doubleSpinBoxes[FFL_WBS]->value(), doubleSpinBoxes[FFL_WDT]->value(), std::max(doubleSpinBoxes[FFL_FRD]->value(), doubleSpinBoxes[FFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXRearTrack()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "fixed");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("mass", doubleSpinBoxes[RFL_MAS]->value() / 3.0);
    write(node, "inertia", calcBoxInertia(doubleSpinBoxes[RFL_MAS]->value() / 3.0, doubleSpinBoxes[RFL_WBS]->value(), doubleSpinBoxes[RFL_WDT]->value(), std::max(doubleSpinBoxes[RFL_FRD]->value(), doubleSpinBoxes[RFL_RRD]->value())));

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXSubTrackBelt()
{
    MappingPtr node = new Mapping;

    write(node, "upAxis", Vector3(0.0, 0.0, 1.0));
    node->write("numberOfNodes", agxSpinBoxes[FLP_BNN]->value());
    node->write("nodeThickness", agxDoubleSpinBoxes[FLP_BNT]->value());
    node->write("nodeWidth", agxDoubleSpinBoxes[FLP_BNW]->value());
    node->write("nodeThickerThickness", agxDoubleSpinBoxes[FLP_BNTT]->value());
    node->write("useThickerNodeEvery", agxSpinBoxes[FLP_BUTNE]->value());
    node->write("material", "CrawlerTracks");
    node->write("nodeDistanceTension", agxDoubleSpinBoxes[FLP_BNDTM]->value() * exp10(-agxSpinBoxes[FLP_BNDTE]->value()));
    node->write("stabilizingHingeFrictionParameter", agxDoubleSpinBoxes[FLP_BSHFPM]->value() * exp10(-agxSpinBoxes[FLP_BSHFPE]->value()));
    node->write("minStabilizingHingeNormalForce", agxSpinBoxes[FLP_BMSHNF]->value());
    node->write("hingeCompliance", agxDoubleSpinBoxes[FLP_BHCM]->value() * exp10(-agxSpinBoxes[FLP_BHCE]->value()));
    node->write("hingeSpookDamping", agxDoubleSpinBoxes[FLP_BHSD]->value());
    node->write("nodesToWheelsMergeThreshold", agxDoubleSpinBoxes[FLP_BNWMT]->value());
    node->write("nodesToWheelsSplitThreshold", agxDoubleSpinBoxes[FLP_BNWST]->value());

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXFrontSprocket()
{
    MappingPtr node = new Mapping;

    double r2spf = doubleSpinBoxes[FFL_RRD]->value() * doubleSpinBoxes[FFL_RRD]->value();
    double r2rof = ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0);
    double r2idf = doubleSpinBoxes[FFL_FRD]->value() * doubleSpinBoxes[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2spf * r2spf / totalf * doubleSpinBoxes[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, doubleSpinBoxes[FFL_RRD]->value(), doubleSpinBoxes[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[FFL_RRD]->value(), doubleSpinBoxes[FFL_WDT]->value(), colorButtons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXFrontRoller()
{
    MappingPtr node = new Mapping;

    double r2spf = doubleSpinBoxes[FFL_RRD]->value() * doubleSpinBoxes[FFL_RRD]->value();
    double r2rof = ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0);
    double r2idf = doubleSpinBoxes[FFL_FRD]->value() * doubleSpinBoxes[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2rof * r2rof / totalf * doubleSpinBoxes[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, (doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0, doubleSpinBoxes[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0, doubleSpinBoxes[FFL_WDT]->value(), colorButtons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXFrontIdler()
{
    MappingPtr node = new Mapping;

    double r2spf = doubleSpinBoxes[FFL_RRD]->value() * doubleSpinBoxes[FFL_RRD]->value();
    double r2rof = ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[FFL_RRD]->value() + doubleSpinBoxes[FFL_FRD]->value()) / 2.0);
    double r2idf = doubleSpinBoxes[FFL_FRD]->value() * doubleSpinBoxes[FFL_FRD]->value();
    double totalf = r2spf + r2rof + r2idf;
    double mass = r2idf * r2idf / totalf * doubleSpinBoxes[FFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, doubleSpinBoxes[FFL_FRD]->value(), doubleSpinBoxes[FFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[FFL_FRD]->value(), doubleSpinBoxes[FFL_WDT]->value(), colorButtons[FFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXRearSprocket()
{
    MappingPtr node = new Mapping;

    double r2spr = doubleSpinBoxes[RFL_RRD]->value() * doubleSpinBoxes[RFL_RRD]->value();
    double r2ror = ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0);
    double r2idr = doubleSpinBoxes[RFL_FRD]->value() * doubleSpinBoxes[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2spr * r2spr / totalr * doubleSpinBoxes[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, doubleSpinBoxes[RFL_FRD]->value(), doubleSpinBoxes[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[RFL_FRD]->value(), doubleSpinBoxes[RFL_WDT]->value(), colorButtons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXRearRoller()
{
    MappingPtr node = new Mapping;

    double r2spr = doubleSpinBoxes[RFL_RRD]->value() * doubleSpinBoxes[RFL_RRD]->value();
    double r2ror = ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0);
    double r2idr = doubleSpinBoxes[RFL_FRD]->value() * doubleSpinBoxes[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2ror * r2ror / totalr * doubleSpinBoxes[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, (doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0, doubleSpinBoxes[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0, doubleSpinBoxes[RFL_WDT]->value(), colorButtons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXRearIdler()
{
    MappingPtr node = new Mapping;

    double r2spr = doubleSpinBoxes[RFL_RRD]->value() * doubleSpinBoxes[RFL_RRD]->value();
    double r2ror = ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0) * ((doubleSpinBoxes[RFL_RRD]->value() + doubleSpinBoxes[RFL_FRD]->value()) / 2.0);
    double r2idr = doubleSpinBoxes[RFL_FRD]->value() * doubleSpinBoxes[RFL_FRD]->value();
    double totalr = r2spr + r2ror + r2idr;
    double mass = r2idr * r2idr / totalr * doubleSpinBoxes[RFL_MAS]->value();

    node->insert(writeAGXWheel());
    node->write("mass", mass);
    write(node, "inertia", calcCylinderInertia(mass, doubleSpinBoxes[RFL_RRD]->value(), doubleSpinBoxes[RFL_WDT]->value()));

    ListingPtr elementsNode = new Listing;

    elementsNode->append(writeCylinderShape(doubleSpinBoxes[RFL_RRD]->value(), doubleSpinBoxes[RFL_WDT]->value(), colorButtons[RFL_CLR]->color()));
    if(!elementsNode->empty()) {
        node->insert("elements", elementsNode);
    }

    return node;
}


MappingPtr CrawlerCreatorWidget::writeAGXWheel()
{
    MappingPtr node = new Mapping;

    node->write("jointType", "revolute");
    node->write("jointAxis", "Y");
    write(node, "centerOfMass", Vector3(0.0, 0.0, 0.0));
    node->write("material", "CrawlerWheel");

    return node;
}


MappingPtr CrawlerCreatorWidget::writeCylinderShape(const double& radius, const double& height, const Vector3& color)
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


MappingPtr CrawlerCreatorWidget::writeAGXVehicleContinuousTrackDevice(const char* name, const char* sprocketName, const char* rollerName, const char* idlerName, const bool isSubTrack)
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