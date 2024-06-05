/**
   @author Kenta Suzuki
*/

#include "BeepWidget.h"
#include <cnoid/Archive>
#include <cnoid/Button>
#include <QBoxLayout>
#include <QTreeWidgetItem>
#include "Beeper.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BeepWidget::Impl
{
public:
    BeepWidget* self;

    Impl(BeepWidget* self);

    enum ButtonID { ADD_BUTTON, REMOVE_BUTTON, PLAY_BUTTON, NUM_BUTTONS };
    enum ColumnID { NO, LINK0, LINK1, FREQUENCY, NUM_COLUMNS };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];
    Beeper* beeper;

    void addItem(const string& link0, const string& link1, const int& frequency);
    void removeCurrentItem();
    void clearItems();
    void onButtonClicked(const int& id);
    void onPlayButtonClicked();
    void play(QTreeWidgetItem* item);
    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);
};

}


BeepWidget::BeepWidget()
{
    impl = new Impl(this);
}


BeepWidget::Impl::Impl(BeepWidget* self)
    : self(self)
{
    beeper = new Beeper;

    const QStringList label0 = { _("No"), _("Link0"), _("Link1"), _("Frequency") };
    treeWidget = new TreeWidget;
    treeWidget->setHeaderLabels(label0);

    static const char* label1[] = { _("+"), _("-"), _("Play") };
    auto vbox = new QVBoxLayout;
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton(label1[i]);
        vbox->addWidget(buttons[i]);
        buttons[i]->sigClicked().connect([&, i](){ onButtonClicked(i); });
    }
    vbox->addStretch();

    auto hbox = new QHBoxLayout;
    hbox->addWidget(treeWidget);
    hbox->addLayout(vbox);

    auto topVbox = new QVBoxLayout;
    topVbox->addLayout(hbox);
    self->setLayout(topVbox);
}


BeepWidget::~BeepWidget()
{
    delete impl;
}


TreeWidget* BeepWidget::treeWidget()
{
    return impl->treeWidget;
}


void BeepWidget::Impl::addItem(const string& link0, const string& link1, const int& frequency)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    int count = treeWidget->topLevelItemCount();
    item->setText(NO, to_string(count).c_str());
    item->setText(LINK0, link0.c_str());
    item->setText(LINK1, link1.c_str());
    item->setText(FREQUENCY, to_string(frequency).c_str());
    treeWidget->addTopLevelItem(item);
    treeWidget->setCurrentItem(item);
}


void BeepWidget::Impl::removeCurrentItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void BeepWidget::Impl::clearItems()
{
    int numChildren = treeWidget->topLevelItemCount();
    for(int i = 0; i < numChildren; ++i) {
        QTreeWidgetItem* item = treeWidget->currentItem();
        if(item) {
            int index = treeWidget->indexOfTopLevelItem(item);
            treeWidget->takeTopLevelItem(index);
        }
    }
}


void BeepWidget::Impl::onButtonClicked(const int& id)
{
    if(id == ADD_BUTTON) {
        addItem("", "", 440);
    } else if(id == REMOVE_BUTTON) {
        removeCurrentItem();
    } else if(id == PLAY_BUTTON) {
        onPlayButtonClicked();
    }
}


void BeepWidget::Impl::onPlayButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    play(item);
}


int BeepWidget::numItems() const
{
    return impl->treeWidget->topLevelItemCount();
}


void BeepWidget::play(const int& index)
{
    QTreeWidgetItem* item = impl->treeWidget->topLevelItem(index);
    impl->play(item);
}


void BeepWidget::Impl::play(QTreeWidgetItem* item)
{
    if(item) {
        int frequency = item->text(FREQUENCY).toInt();
        int length = 200;

        if(!beeper->isActive()) {
            beeper->start(frequency, length);
        }
    }
}


bool BeepWidget::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool BeepWidget::Impl::storeState(Archive& archive)
{
    ListingPtr itemListing = new Listing;

    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            ArchivePtr subArchive = new Archive;
            subArchive->write("link0", item->text(LINK0).toStdString());
            subArchive->write("link1", item->text(LINK1).toStdString());
            subArchive->write("frequency", item->text(FREQUENCY).toStdString());

            itemListing->append(subArchive);
        }
    }

    archive.insert("items", itemListing);

    return true;
}


bool BeepWidget::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool BeepWidget::Impl::restoreState(const Archive& archive)
{
    clearItems();

    ListingPtr itemListing = archive.findListing("items");
    if(itemListing->isValid()) {
        for(int i = 0; i < itemListing->size(); ++i) {
            auto subArchive = archive.subArchive(itemListing->at(i)->toMapping());
            string link0, link1;
            int frequency;
            subArchive->read("link0", link0);
            subArchive->read("link1", link1);
            subArchive->read("frequency", frequency);
            addItem(link0, link1, frequency);
        }
    }
    return true;
}
