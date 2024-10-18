/**
   @author Kenta Suzuki
*/

#include "EnergyFilter.h"
#include <cnoid/Button>
#include <cnoid/ButtonGroup>
#include <cnoid/Dialog>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/ExecutablePath>
#include <cnoid/Separator>
#include <cnoid/SpinBox>
#include <cnoid/stdx/filesystem>
#include <cnoid/TreeWidget>
#include <cnoid/UTF8>
#include <cnoid/YAMLReader>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace {

class EnergyFilterDialog : public QDialog
{
public:
    EnergyFilterDialog(QWidget* parent = nullptr);

    enum ItemId { CHECK, SPECIES, MIN, MAX };

    RadioButton noFilterRadio;
    RadioButton rangeFilterRadio;
    RadioButton nuclideFilterRadio;
    SpinBox* minChSpinBox;
    SpinBox* maxChSpinBox;
    TreeWidget* nuclideTree;
    QDialogButtonBox* buttonBox;

    void storeState(Archive& archive);
    void restoreState(const Archive& archive);
};

}

namespace cnoid {

class EnergyFilter::Impl
{
public:
    EnergyFilter* self;

    Impl(EnergyFilter* self);

    EnergyFilterDialog* config;
};

}


EnergyFilter::EnergyFilter()
{
    impl = new Impl(this);
}


EnergyFilter::Impl::Impl(EnergyFilter* self)
    : self(self),
      config(new EnergyFilterDialog())
{

}


EnergyFilter::~EnergyFilter()
{
    delete impl;
}


void EnergyFilter::showConfigDialog()
{
    impl->config->show();
}


int EnergyFilter::mode() const
{
    int mode = NO_FILTER;
    if(impl->config->noFilterRadio.isChecked()) {
        mode = NO_FILTER;
    } else if(impl->config->rangeFilterRadio.isChecked()) {
        mode = RANGE_FILTER;
    } else if(impl->config->nuclideFilterRadio.isChecked()) {
        mode = NUCLIDE_FILTER;
    }
    return mode;
}


int EnergyFilter::min() const
{
    return impl->config->minChSpinBox->value();
}


int EnergyFilter::max() const
{
    return impl->config->maxChSpinBox->value();
}


vector<EnergyFilter::NuclideFilterInfo> EnergyFilter::nuclideFilterInfo() const
{
    vector<EnergyFilter::NuclideFilterInfo> nuclide_filter_info;

    for(int i = 0; i < impl->config->nuclideTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = impl->config->nuclideTree->topLevelItem(i);
        CheckBox* check = dynamic_cast<CheckBox*>(impl->config->nuclideTree->itemWidget(item, EnergyFilterDialog::CHECK));
        if(check) {
            if(check->isChecked()) {
                EnergyFilter::NuclideFilterInfo info;
                info.species = item->text(EnergyFilterDialog::SPECIES).toStdString();
                info.min = atoi(item->text(EnergyFilterDialog::MIN).toStdString().c_str());
                info.max = atoi(item->text(EnergyFilterDialog::MAX).toStdString().c_str());
                nuclide_filter_info.push_back(info);
            }
        }
    }
    return nuclide_filter_info;
}


bool EnergyFilter::load(const string& filename, ostream& os)
{
    TreeWidget* nuclideTree = impl->config->nuclideTree;
    nuclideTree->clear();
    try {
        YAMLReader reader;
        MappingPtr node = reader.loadDocument(filename)->toMapping();
        if(node) {
            auto& filterList = *node->findListing("filters");
            if(filterList.isValid()) {
                for(int i = 0; i < filterList.size(); ++i) {
                    Mapping* info = filterList[i].toMapping();
                    string name;
                    info->read("name", name);
                    Vector2 band;
                    read(info, "band", band);

                    QTreeWidgetItem* item = new QTreeWidgetItem(nuclideTree);
                    CheckBox* check = new CheckBox();
                    nuclideTree->setItemWidget(item, EnergyFilterDialog::CHECK, check);
                    item->setText(EnergyFilterDialog::SPECIES, QString::fromStdString(name));
                    item->setText(EnergyFilterDialog::MIN, QString::fromStdString(to_string((int)band[0])));
                    item->setText(EnergyFilterDialog::MAX, QString::fromStdString(to_string((int)band[1])));
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }
    return true;
}


void EnergyFilter::storeState(Archive& archive)
{
    impl->config->storeState(archive);
}


void EnergyFilter::restoreState(const Archive& archive)
{
    impl->config->restoreState(archive);
}


EnergyFilterDialog::EnergyFilterDialog(QWidget* parent)
    : QDialog(parent)
{
    ButtonGroup group;
    group.addButton(&noFilterRadio);
    group.addButton(&rangeFilterRadio);
    group.addButton(&nuclideFilterRadio);
    noFilterRadio.setText(_("No filter"));
    rangeFilterRadio.setText(_("Range filter"));
    nuclideFilterRadio.setText(_("Nuclide filter"));

    auto rangeHbox = new QHBoxLayout();
    rangeHbox->addWidget(&rangeFilterRadio);
    rangeHbox->addStretch();
    rangeHbox->addWidget(new QLabel(_("Min [Ch]")));
    minChSpinBox = new SpinBox();
    minChSpinBox->setSingleStep(1);
    minChSpinBox->setValue(0);
    minChSpinBox->setMinimum(0);
    minChSpinBox->setMaximum(100000);
    rangeHbox->addWidget(minChSpinBox);
    rangeHbox->addSpacing(10);
    rangeHbox->addWidget(new QLabel(_("Max [Ch]")));
    maxChSpinBox = new SpinBox();
    maxChSpinBox->setSingleStep(1);
    maxChSpinBox->setValue(1);
    maxChSpinBox->setMinimum(0);
    maxChSpinBox->setMaximum(100000);
    rangeHbox->addWidget(maxChSpinBox);

    auto nuclideHbox = new QHBoxLayout();
    nuclideHbox->addWidget(&nuclideFilterRadio);
    nuclideHbox->addStretch();
    nuclideTree = new TreeWidget();
    const QStringList headers = { " ", _("Nuclide"), _("Min [Ch]"), _("Max [Ch]") };
    nuclideTree->setHeaderLabels(headers);

    noFilterRadio.setChecked(true);
    minChSpinBox->setEnabled(false);
    maxChSpinBox->setEnabled(false);
    nuclideTree->setEnabled(false);
    rangeFilterRadio.sigToggled().connect(
        [&](bool checked){
            minChSpinBox->setEnabled(checked);
            maxChSpinBox->setEnabled(checked);
        });
    nuclideFilterRadio.sigToggled().connect(
        [&](bool checked){
            nuclideTree->setEnabled(checked);
        });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(&noFilterRadio);
    mainLayout->addLayout(rangeHbox);
    mainLayout->addLayout(nuclideHbox);
    mainLayout->addWidget(nuclideTree);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator());
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Energy Filter Config"));
}


void EnergyFilterDialog::storeState(Archive& archive)
{
    int mode = 0;
    if(noFilterRadio.isChecked()) {
        mode = EnergyFilter::NO_FILTER;
    } else if(rangeFilterRadio.isChecked()) {
        mode = EnergyFilter::RANGE_FILTER;
    } else if(nuclideFilterRadio.isChecked()) {
        mode = EnergyFilter::NUCLIDE_FILTER;
    }
    archive.write("mode", mode);
    archive.write("min", minChSpinBox->value());
    archive.write("max", maxChSpinBox->value());

    ListingPtr filterListing = new Listing;

    for(int i = 0; i < nuclideTree->topLevelItemCount(); ++i) {
        bool checked = false;
        QTreeWidgetItem* item = nuclideTree->topLevelItem(i);
        if(item) {
            ArchivePtr subArchive = new Archive;
            CheckBox* check = dynamic_cast<CheckBox*>(nuclideTree->itemWidget(item, CHECK));
            checked = check->isChecked();
            subArchive->write("filter", checked);

            filterListing->append(subArchive);
        }
    }

    archive.insert("filters", filterListing);
}


void EnergyFilterDialog::restoreState(const Archive& archive)
{
    int mode = 0;
    archive.read("mode", mode);
    if(mode == EnergyFilter::NO_FILTER) {
        noFilterRadio.setChecked(true);
    } else if(mode == EnergyFilter::RANGE_FILTER) {
        rangeFilterRadio.setChecked(true);
    } else if(mode == EnergyFilter::NUCLIDE_FILTER) {
        nuclideFilterRadio.setChecked(true);
    }

    minChSpinBox->setValue(archive.get("min", 0));
    maxChSpinBox->setValue(archive.get("max", 0));

    ListingPtr filterListing = archive.findListing("filters");
    if(filterListing->isValid()) {
        for(int i = 0; i < filterListing->size(); ++i) {
            bool checked = false;
            QTreeWidgetItem* item = nuclideTree->topLevelItem(i);
            if(item) {
                auto subArchive = filterListing->at(i)->toMapping();
                subArchive->read("filter", checked);
                CheckBox* check = dynamic_cast<CheckBox*>(nuclideTree->itemWidget(item, CHECK));
                if(check) {
                    check->setChecked(checked);
                }
            }
        }
    }
}
