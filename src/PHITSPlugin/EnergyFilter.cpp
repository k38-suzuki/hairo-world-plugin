/**
   \file
   \author Kenta Suzuki
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
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;
namespace filesystem = stdx::filesystem;

namespace {

class EnergyFilterDialog : public Dialog
{
public:
    EnergyFilterDialog();

    enum ItemID { CHECK, SPECIES, MIN, MAX, NUM_ITEMIDS };

    RadioButton noFilterRadio;
    RadioButton rangeFilterRadio;
    RadioButton nuclideFilterRadio;
    SpinBox* minChSpin;
    SpinBox* maxChSpin;
    TreeWidget* nuclideTree;

    void store(Mapping* archive);
    void restore(const Mapping* archive);
};

}


namespace cnoid {

class EnergyFilterImpl
{
public:
    EnergyFilterImpl(EnergyFilter* self);
    EnergyFilter* self;

    EnergyFilterDialog* config;
};

}


EnergyFilter::EnergyFilter()
{
    impl = new EnergyFilterImpl(this);
}


EnergyFilterImpl::EnergyFilterImpl(EnergyFilter* self)
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
    return impl->config->minChSpin->value();
}


int EnergyFilter::max() const
{
    return impl->config->maxChSpin->value();
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


void EnergyFilter::store(Mapping* archive)
{
    impl->config->store(archive);
}


void EnergyFilter::restore(const Mapping* archive)
{
    impl->config->restore(archive);
}


EnergyFilterDialog::EnergyFilterDialog()
{
    setWindowTitle(_("Energy Filter Config"));

    ButtonGroup group;
    group.addButton(&noFilterRadio);
    group.addButton(&rangeFilterRadio);
    group.addButton(&nuclideFilterRadio);
    noFilterRadio.setText(_("No filter"));
    rangeFilterRadio.setText(_("Range filter"));
    nuclideFilterRadio.setText(_("Nuclide filter"));

    QHBoxLayout* rangeHbox = new QHBoxLayout();
    rangeHbox->addWidget(&rangeFilterRadio);
    rangeHbox->addStretch();
    rangeHbox->addWidget(new QLabel(_("Min [Ch]")));
    minChSpin = new SpinBox();
    minChSpin->setSingleStep(1);
    minChSpin->setValue(0);
    minChSpin->setMinimum(0);
    minChSpin->setMaximum(100000);
    rangeHbox->addWidget(minChSpin);
    rangeHbox->addSpacing(10);
    rangeHbox->addWidget(new QLabel(_("Max [Ch]")));
    maxChSpin = new SpinBox();
    maxChSpin->setSingleStep(1);
    maxChSpin->setValue(1);
    maxChSpin->setMinimum(0);
    maxChSpin->setMaximum(100000);
    rangeHbox->addWidget(maxChSpin);

    QHBoxLayout* nuclideHbox = new QHBoxLayout();
    nuclideHbox->addWidget(&nuclideFilterRadio);
    nuclideHbox->addStretch();
    nuclideTree = new TreeWidget();
    const QStringList headers = { " ", _("Nuclide"), _("Min [Ch]"), _("Max [Ch]") };
    nuclideTree->setHeaderLabels(headers);

    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));

    noFilterRadio.setChecked(true);
    minChSpin->setEnabled(false);
    maxChSpin->setEnabled(false);
    nuclideTree->setEnabled(false);
    rangeFilterRadio.sigToggled().connect([&](bool on){
        minChSpin->setEnabled(on);
        maxChSpin->setEnabled(on);
    });
    nuclideFilterRadio.sigToggled().connect([&](bool on){
        nuclideTree->setEnabled(on);
    });

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(&noFilterRadio);
    vbox->addLayout(rangeHbox);
    vbox->addLayout(nuclideHbox);
    vbox->addWidget(nuclideTree);
    vbox->addWidget(new HSeparator());
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


void EnergyFilterDialog::store(Mapping* archive)
{
    int mode = 0;
    if(noFilterRadio.isChecked()) {
        mode = EnergyFilter::NO_FILTER;
    } else if(rangeFilterRadio.isChecked()) {
        mode = EnergyFilter::RANGE_FILTER;
    } else if(nuclideFilterRadio.isChecked()) {
        mode = EnergyFilter::NUCLIDE_FILTER;
    }
    archive->write("mode", mode);
    archive->write("min", minChSpin->value());
    archive->write("max", maxChSpin->value());

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

    archive->insert("filters", filterListing);
}


void EnergyFilterDialog::restore(const Mapping* archive)
{
    int mode = 0;
    archive->read("mode", mode);
    if(mode == EnergyFilter::NO_FILTER) {
        noFilterRadio.setChecked(true);
    } else if(mode == EnergyFilter::RANGE_FILTER) {
        rangeFilterRadio.setChecked(true);
    } else if(mode == EnergyFilter::NUCLIDE_FILTER) {
        nuclideFilterRadio.setChecked(true);
    }
    
    minChSpin->setValue(archive->get("min", 0));
    maxChSpin->setValue(archive->get("max", 0));

    ListingPtr filterListing = archive->findListing("filters");
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
