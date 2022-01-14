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
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <cmath>
#include <stdio.h>
#include "FileFormWidget.h"
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

void putKeyVector3(YAMLWriter& writer, const string key, const Vector3 value)
{
    writer.putKey(key);
    writer.startFlowStyleListing();
    for(int i = 0; i < 3; ++i) {
        writer.putScalar(value[i]);
    }
    writer.endListing();
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


DoubleSpinInfo agxdoubleSpinInfo[] = {
    {  1, 4,  0.010, 0.0, 1000.000, 3, false }, {  2, 1,  0.090, 0.0, 1000.000, 3, false }, {  2, 4,  0.020,       0.0, 1000.000, 3, false }, {  3, 4,  2.000,       0.0, 1000.000, 3, false }, {  4, 1,  1.000, 0.0, 1000.000, 3, false },
    {  5, 1,  9.000, 0.0, 1000.000, 3, false }, {  5, 4,  0.010, 0.0, 1000.000, 3, false }, {  6, 1, -0.001, -1000.000, 1000.000, 3, false }, {  6, 4, -0.009, -1000.000, 1000.000, 3, false },
    {  8, 4,  0.010, 0.0, 1000.000, 3, false }, {  9, 1,  0.090, 0.0, 1000.000, 3, false }, {  9, 4,  0.020,       0.0, 1000.000, 3, false }, { 10, 4,  2.000,       0.0, 1000.000, 3, false }, { 11, 1,  1.000, 0.0, 1000.000, 3, false },
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


SpinInfo agxspinInfo[] = {
    {  1, 1, 42, 0, 9999, false }, {  3, 1,   3, 0, 9999, false }, {  3, 5,  4, 0, 9999, false },
    {  4, 2,  6, 0, 9999, false }, {  4, 4, 100, 0, 9999, false }, {  5, 2, 10, 0, 9999, false },
    {  8, 1, 42, 0, 9999, false }, { 10, 1,   3, 0, 9999, false }, { 10, 5,  4, 0, 9999, false },
    { 11, 2,  6, 0, 9999, false }, { 11, 4, 100, 0, 9999, false }, { 12, 2, 10, 0, 9999, false }
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

class CrawlerConfigDialog : public Dialog
{
public:
    CrawlerConfigDialog();

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

    bool save(const string& filename);
    void initialize();
    void onResetButtonClicked();
    void onExportYamlButtonClicked();
    void onImportYamlButtonClicked();
    bool loadConfig(const string& filename, ostream& os = nullout());
    void onEnableAgxCheckToggled(const bool& on);
    void onExportAGXBody(const string& fileName);
    void onColorChanged(PushButton* pushbutton);
    void setColor(PushButton* pushbutton, const Vector3& color);
    Vector3 extractColor(PushButton* colorButton);
    bool writeYaml(const string& filename);
    bool write(const string& filename);
    bool writeTrack(YAMLWriter& writer);
    bool writeSpacer(YAMLWriter& writer);
    bool writeSubTrackF(YAMLWriter& writer);
    bool writeSubTrackR(YAMLWriter& writer);
};


class CrawlerGeneratorImpl
{
public:
    CrawlerGeneratorImpl(CrawlerGenerator* self, ExtensionManager* ext);
    CrawlerGenerator* self;

    CrawlerConfigDialog* dialog;
};

}


CrawlerGenerator::CrawlerGenerator(ExtensionManager* ext)
{
    impl = new CrawlerGeneratorImpl(this, ext);
}


CrawlerGeneratorImpl::CrawlerGeneratorImpl(CrawlerGenerator* self, ExtensionManager* ext)
    : self(self)
{
    dialog = new CrawlerConfigDialog();

    MenuManager& mm = ext->menuManager().setPath("/Tools").setPath(_("BodyGenerator"));
    mm.addItem(_("CrawlerRobot"))->sigTriggered().connect([&](){ dialog->show(); });
}


CrawlerGenerator::~CrawlerGenerator()
{
    delete impl;
}


void CrawlerGenerator::initialize(ExtensionManager* ext)
{
    ext->manage(new CrawlerGenerator(ext));
}


CrawlerConfigDialog::CrawlerConfigDialog()
{
    setWindowTitle(_("CrawlerRobot Builder"));
    QGridLayout* gbox = new QGridLayout();
    QGridLayout* agbox = new QGridLayout();

    for(int i = 0; i < NUM_DSPINS; ++i) {
        DoubleSpinInfo info = doubleSpinInfo[i];
        dspins[i] = new DoubleSpinBox();
        DoubleSpinBox* dspin = dspins[i];
        gbox->addWidget(dspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_AGXDSPINS; ++i) {
        DoubleSpinInfo info = agxdoubleSpinInfo[i];
        agxdspins[i] = new DoubleSpinBox();
        DoubleSpinBox* agxdspin = agxdspins[i];
        agbox->addWidget(agxdspin, info.row, info.column);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        agxspins[i] = new SpinBox();
        SpinInfo info = agxspinInfo[i];
        SpinBox* spin = agxspins[i];
        agbox->addWidget(spin, info.row, info.column);
    }

    for(int i = 0; i < NUM_BUTTONS; ++i) {
        ButtonInfo info = buttonInfo[i];
        buttons[i] = new PushButton();
        PushButton* button = buttons[i];
        gbox->addWidget(button, info.row, info.column);
        button->sigClicked().connect([&, button](){ onColorChanged(button); });
    }

    static const char* clabels[] = { _("Front SubTrack"), _("Rear SubTrack"), _("AGX") };

    QHBoxLayout* chbox = new QHBoxLayout();
    for(int i = 0; i < NUM_CHECKS; ++i) {
        CheckInfo info = checkInfo[i];
        checks[i] = new CheckBox();
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

    QHBoxLayout* thbox = new QHBoxLayout();
    thbox->addStretch();
    for(int i = 0; i < NUMTBUTTONS; ++i) {
        toolButtons[i] = new PushButton(tlabels[i]);
        PushButton* button = toolButtons[i];
        thbox->addWidget(button);
    }

    formWidget = new FileFormWidget();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    PushButton* okButton = new PushButton(_("&Ok"));
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);

    initialize();

    QVBoxLayout* vbox = new QVBoxLayout();
    QHBoxLayout* hbox = new QHBoxLayout();
    vbox->addLayout(thbox);
    hbox->addLayout(gbox);
//    hbox->addLayout(agbox);
    vbox->addLayout(hbox);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(formWidget);
    vbox->addWidget(buttonBox);
    setLayout(vbox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    toolButtons[RESET]->sigClicked().connect([&](){ onResetButtonClicked(); });
    toolButtons[IMPORT]->sigClicked().connect([&](){ onImportYamlButtonClicked(); });
    toolButtons[EXPORT]->sigClicked().connect([&](){ onExportYamlButtonClicked(); });
    checks[AGX_CHK]->sigToggled().connect([&](bool on){ onEnableAgxCheckToggled(on); });
    formWidget->sigClicked().connect([&](string filename){ save(filename); });
}


bool CrawlerConfigDialog::save(const string& filename)
{
    if(!filename.empty()) {
        if(!checks[AGX_CHK]->isChecked()) {
            write(filename);
        } else {
            onExportAGXBody(filename);
        }
    }
    return true;
}


void CrawlerConfigDialog::initialize()
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


void CrawlerConfigDialog::onResetButtonClicked()
{
    initialize();
}


void CrawlerConfigDialog::onImportYamlButtonClicked()
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


bool CrawlerConfigDialog::loadConfig(const string& filename,std::ostream& os )
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


void CrawlerConfigDialog::onExportYamlButtonClicked()
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
        if(ext.empty()){
            filename += ".yaml";
        }
        writeYaml(filename);
    }
}


void CrawlerConfigDialog::onEnableAgxCheckToggled(const bool& on)
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


void CrawlerConfigDialog::onExportAGXBody(const string& fileName)
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
    Vector3 spacerColor = extractColor(buttons[SPC_CLR]);
    fprintf(fp, "          diffuseColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          specularColor: [ %3.2lf, %3.2lf, %3.2lf ]\n", spacerColor[0], spacerColor[1], spacerColor[2]);
    fprintf(fp, "          shininess: 0.6\n");
    fprintf(fp, "\n");

    fprintf(fp, "TRACKBELT_COMMON: &TrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", agxspins[TRK_BNN]->value());
    fprintf(fp, "  nodeThickness: %lf\n", agxdspins[TRK_BNT]->value());
    fprintf(fp, "  nodeWidth:  %lf\n", agxdspins[TRK_BNW]->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", agxdspins[TRK_BNTT]->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", agxspins[TRK_BUTNE]->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "  nodeDistanceTension: %2.1lfe-%d\n", agxdspins[TRK_BNDTM]->value(), agxspins[TRK_BNDTE]->value());
    fprintf(fp, "  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", agxdspins[TRK_BSHFPM]->value(), agxspins[TRK_BSHFPE]->value());
    fprintf(fp, "  minStabilizingHingeNormalForce: %d\n", agxspins[TRK_BMSHNF]->value());
    fprintf(fp, "  hingeCompliance: %2.1lfe-%d\n", agxdspins[TRK_BHCM]->value(), agxspins[TRK_BHCE]->value());
    fprintf(fp, "  hingeSpookDamping: %lf\n", agxdspins[TRK_BHSD]->value());
    fprintf(fp, "  nodesToWheelsMergeThreshold: %lf\n", agxdspins[TRK_BNWMT]->value());
    fprintf(fp, "  nodesToWheelsSplitThreshold: %lf\n", agxdspins[TRK_BNWST]->value());
    fprintf(fp, "\n");

    fprintf(fp, "SUBTRACKBELT_COMMON: &SubTrackBeltCommon\n");
    fprintf(fp, "  upAxis: [ 0, 0, 1 ]\n");
    fprintf(fp, "  numberOfNodes: %d\n", agxspins[FLP_BNN]->value());
    fprintf(fp, "  nodeThickness: %lf\n", agxdspins[FLP_BNT]->value());
    fprintf(fp, "  nodeWidth: %lf\n", agxdspins[FLP_BNW]->value());
    fprintf(fp, "  nodeThickerThickness: %lf\n", agxdspins[FLP_BNTT]->value());
    fprintf(fp, "  useThickerNodeEvery: %d\n", agxspins[FLP_BUTNE]->value());
    fprintf(fp, "  material: %sTracks\n", bodyName.c_str());
    fprintf(fp, "  nodeDistanceTension: %2.1lfe-%d\n", agxdspins[FLP_BNDTM]->value(), agxspins[FLP_BNDTE]->value());
    fprintf(fp, "  stabilizingHingeFrictionParameter: %2.1lfe-%d\n", agxdspins[FLP_BSHFPM]->value(), agxspins[FLP_BSHFPE]->value());
    fprintf(fp, "  minStabilizingHingeNormalForce: %d\n", agxspins[FLP_BMSHNF]->value());
    fprintf(fp, "  hingeCompliance: %2.1lfe-%d\n", agxdspins[FLP_BHCM]->value(), agxspins[FLP_BHCE]->value());
    fprintf(fp, "  hingeSpookDamping: %lf\n", agxdspins[FLP_BHSD]->value());
    fprintf(fp, "  nodesToWheelsMergeThreshold: %lf\n", agxdspins[FLP_BNWMT]->value());
    fprintf(fp, "  nodesToWheelsSplitThreshold: %lf\n", agxdspins[FLP_BNWST]->value());
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
    Vector3 trackColor = extractColor(buttons[TRK_CLR]);
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
    Vector3 subtrackfColor = extractColor(buttons[FFL_CLR]);
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
    Vector3 subtrackrColor = extractColor(buttons[RFL_CLR]);
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
    Vector3 chassisColor = extractColor(buttons[CHS_CLR]);
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


void CrawlerConfigDialog::onColorChanged(PushButton* pushbutton)
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


void CrawlerConfigDialog::setColor(PushButton* pushbutton, const Vector3& color)
{
    QColor selectedColor;
    selectedColor.setRed(color[0] * 255.0);
    selectedColor.setGreen(color[1] * 255.0);
    selectedColor.setBlue(color[2] * 255.0);
    QPalette palette;
    palette.setColor(QPalette::Button, selectedColor);
    pushbutton->setPalette(palette);
}


Vector3 CrawlerConfigDialog::extractColor(PushButton* colorButton)
{
    QColor selectedColor = colorButton->palette().color(QPalette::Button);
    return Vector3(selectedColor.red() / 255.0, selectedColor.green() / 255.0, selectedColor.blue() / 255.0);
}


bool CrawlerConfigDialog::writeYaml(const string& filename)
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


bool CrawlerConfigDialog::write(const string& filename)
{
    if(filename.empty()) {
        return false;
    }

    filesystem::path path(filename);
    string name = path.stem().string();

    YAMLWriter writer(filename);
    int jointId = 0;
    writer.startMapping(); {
        writer.putKeyValue("format", "ChoreonoidBody");
        writer.putKeyValue("formatVersion", "1.0");
        writer.putKeyValue("angleUnit", "degree");
        writer.putKeyValue("name", name);
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


bool CrawlerConfigDialog::writeTrack(YAMLWriter& writer)
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


bool CrawlerConfigDialog::writeSpacer(YAMLWriter& writer)
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


bool CrawlerConfigDialog::writeSubTrackF(YAMLWriter& writer)
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


bool CrawlerConfigDialog::writeSubTrackR(YAMLWriter& writer)
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
