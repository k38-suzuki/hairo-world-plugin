/**
   \file
   \author Kenta Suzuki
*/

#include "BeepWidget.h"
#include <cnoid/Archive>
#include <cnoid/Button>
#include <cnoid/Process>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class BeepWidgetImpl
{
public:
    BeepWidgetImpl(BeepWidget* self);
    BeepWidget* self;

    enum ButtonID { ADD_BUTTON, REMOVE_BUTTON, PLAY_BUTTON, NUM_BUTTONS };
    enum ColumnID { NO, LINK0, LINK1, FREQUENCY, NUM_COLUMNS };

    TreeWidget* treeWidget;
    PushButton* buttons[NUM_BUTTONS];
    Process process;

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
    impl = new BeepWidgetImpl(this);
}


BeepWidgetImpl::BeepWidgetImpl(BeepWidget* self)
    : self(self)
{
    const QStringList labels0 = { _("No"), _("Link0"), _("Link1"), _("Frequency") };
    treeWidget = new TreeWidget;
    treeWidget->setHeaderLabels(labels0);

    static const char* labels1[] = { _("+"), _("-"), _("Play") };
    QVBoxLayout* vbox = new QVBoxLayout;
    for(int i = 0; i < NUM_BUTTONS; ++i) {
        buttons[i] = new PushButton(labels1[i]);
        PushButton* button = buttons[i];
        vbox->addWidget(button);
        button->sigClicked().connect([&, i](){ onButtonClicked(i); });
    }
    vbox->addStretch();

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(treeWidget);
    hbox->addLayout(vbox);

    QVBoxLayout* topVbox = new QVBoxLayout;
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


void BeepWidgetImpl::addItem(const string& link0, const string& link1, const int& frequency)
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


void BeepWidgetImpl::removeCurrentItem()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void BeepWidgetImpl::clearItems()
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


void BeepWidgetImpl::onButtonClicked(const int& id)
{
    if(id == ADD_BUTTON) {
        addItem("", "", 440);
    } else if(id == REMOVE_BUTTON) {
        removeCurrentItem();
    } else if(id == PLAY_BUTTON) {
        onPlayButtonClicked();
    }
}


void BeepWidgetImpl::onPlayButtonClicked()
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


void BeepWidgetImpl::play(QTreeWidgetItem* item)
{
    if(item) {
        int frequency = item->text(FREQUENCY).toInt();
        int length = 200;

        string actualCommand = "beep";
        QStringList arguments;

        if(frequency > 0) {
            string argument = "-f " + to_string(frequency);
            arguments << argument.c_str();
        }
        if(length > 0) {
            string argument = "-l " + to_string(length);
            arguments << argument.c_str();
        }

        if(process.state() != QProcess::NotRunning) {
            process.kill();
            process.waitForFinished(100);
        }
        process.start(actualCommand.c_str(), arguments);
        if(process.waitForStarted()) {

        }
    }
}


bool BeepWidget::storeState(Archive& archive)
{
    return impl->storeState(archive);
}


bool BeepWidgetImpl::storeState(Archive& archive)
{
    int numChildren = treeWidget->topLevelItemCount();
    archive.write("num_children", numChildren);
    for(int i = 0; i < numChildren; ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        string key0 = "link0_" + to_string(i);
        string key1 = "link1_" + to_string(i);
        string key2 = "frequency_" + to_string(i);
        archive.write(key0, item->text(LINK0).toStdString());
        archive.write(key1, item->text(LINK1).toStdString());
        archive.write(key2, item->text(FREQUENCY).toStdString());
    }
    return true;
}


bool BeepWidget::restoreState(const Archive& archive)
{
    return impl->restoreState(archive);
}


bool BeepWidgetImpl::restoreState(const Archive& archive)
{
    clearItems();
    int numChildren = archive.get("num_children", 0);
    for(int i = 0; i < numChildren; ++i) {
        string key0 = "link0_" + to_string(i);
        string key1 = "link1_" + to_string(i);
        string key2 = "frequency_" + to_string(i);
        string link0 = archive.get(key0, "");
        string link1 = archive.get(key1, "");
        int frequency = archive.get(key2, 440);
        addItem(link0, link1, frequency);
    }
    return true;
}
