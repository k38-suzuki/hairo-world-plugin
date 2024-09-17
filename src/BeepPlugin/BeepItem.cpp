/**
   @author Kenta Suzuki
*/

#include "BeepItem.h"
#include <cnoid/Archive>
#include <cnoid/Buttons>
#include <cnoid/ComboBox>
#include <cnoid/CollisionLinkPair>
#include <cnoid/Dialog>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/LazyCaller>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Separator>
#include <cnoid/SimulatorItem>
#include <cnoid/WorldItem>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include "Beeper.h"
#include "BeepEventReader.h"
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BeepPlayer : public Dialog
{
public:
    BeepPlayer();

    void load(const string& filename);

private:
    void play();

    ComboBox* eventCombo;
    QDialogButtonBox* buttonBox;
    Beeper beeper;
    vector<BeepEvent> events;
};

class BeepItem::Impl
{
public:
    BeepItem* self;

    Impl(BeepItem* self);
    Impl(BeepItem* self, const Impl& org);

    bool initializeSimulation(SimulatorItem* simulatorItem);
    void onPostDynamics();

    typedef shared_ptr<CollisionLinkPair> CollisionLinkPairPtr;

    SimulatorItem* simulatorItem;
    WorldItem* worldItem;
    Beeper beeper;
    bool is_played;
    string beep_event_file_path;
    vector<BeepEvent> events;
};

}


void BeepItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<BeepItem, SubSimulatorItem>(N_("BeepItem"))
            .addCreationPanel<BeepItem>();

    ItemTreeView::customizeContextMenu<BeepItem>(
        [&](BeepItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction){
            menuManager.addItem(_("Play beep"))->sigTriggered().connect(
                [&, item](){
                    string filename = item->impl->beep_event_file_path;
                    BeepPlayer* player = new BeepPlayer;
                    player->load(filename);
                    player->show();
                });
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


BeepItem::BeepItem()
    : SubSimulatorItem()
{
    impl = new Impl(this);
}


BeepItem::Impl::Impl(BeepItem* self)
    : self(self),
      is_played(false),
      beep_event_file_path("")
{
    simulatorItem = nullptr;
    worldItem = nullptr;
    events.clear();
}


BeepItem::BeepItem(const BeepItem& org)
    : SubSimulatorItem(org)
{
    impl = new Impl(this, *org.impl);
}


BeepItem::Impl::Impl(BeepItem* self, const Impl& org)
    : self(self)
{
    is_played = org.is_played;
    beep_event_file_path = org.beep_event_file_path;
}


BeepItem::~BeepItem()
{
    delete impl;
}


bool BeepItem::initializeSimulation(SimulatorItem* simulatorItem)
{
    return impl->initializeSimulation(simulatorItem);
}


bool BeepItem::Impl::initializeSimulation(SimulatorItem* simulatorItem)
{
    this->simulatorItem = simulatorItem;
    worldItem = this->simulatorItem->findOwnerItem<WorldItem>();
    is_played = false;
    events.clear();

    if(!beep_event_file_path.empty()) {
        BeepEventReader reader;
        if(reader.load(beep_event_file_path)) {
            events = reader.events();
        }
    }

    if(worldItem) {
        worldItem->setCollisionDetectionEnabled(true);
        this->simulatorItem->addPostDynamicsFunction([&](){ onPostDynamics(); });
    }

    return true;
}


void BeepItem::Impl::onPostDynamics()
{
    double currentTime = simulatorItem->currentTime();
    static double startTime = 0.0;

    int playID = 999;
    for(size_t i = 0; i < events.size(); ++i) {
        string link0 = events[i].link1();
        string link1 = events[i].link2();

        bool contacted = false;
        vector<CollisionLinkPairPtr>& collisions = worldItem->collisions();
        for(auto& collision : collisions) {
            LinkPtr links[2] = { collision->link(0), collision->link(1) };
            if((links[0]->name() == link0 && links[1]->name() == link1)
                    || (links[0]->name() == link1 && links[1]->name() == link0)) {
                contacted = true;
            } else if(link0 == "ALL") {
                if(links[0]->name() == link1 || links[1]->name() == link1) {
                    contacted = true;
                }
            } else if(link1 == "ALL") {
                if(links[0]->name() == link0 || links[1]->name() == link0) {
                    contacted = true;
                }
            }
            if(contacted && !events[i].isEnabled()) {
                playID = i;
            }
        }

        events[i].setEnabled(contacted);
    }

    if(playID != 999 && !is_played) {
        startTime = currentTime;
    }

    if(currentTime < startTime + 0.2) {
        callLater([this, playID](){
            int frequency = events[playID].frequency();
            int length = 200;
            if(!beeper.isActive()) {
                beeper.start(frequency, length);
            }
        });
        is_played = true;
    } else {
        is_played = false;
    }
}


Item* BeepItem::doCloneItem(CloneMap* cloneMap) const
{
    return new BeepItem(*this);
}


void BeepItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SubSimulatorItem::doPutProperties(putProperty);
    putProperty(_("Beep event file"), FilePathProperty(impl->beep_event_file_path),
                [this](const std::string& value){
                    impl->beep_event_file_path = value;
                    return true;
                });
}


bool BeepItem::store(Archive& archive)
{
    SubSimulatorItem::store(archive);
    archive.writeRelocatablePath("beep_event_file_path", impl->beep_event_file_path);
    return true;
}


bool BeepItem::restore(const Archive& archive)
{
    SubSimulatorItem::restore(archive);
    string symbol;
    if(archive.read("beep_event_file_path", symbol)) {
        symbol = archive.resolveRelocatablePath(symbol);
        if(!symbol.empty()) {
            impl->beep_event_file_path = symbol;
        }
    }
    return true;
}


BeepPlayer::BeepPlayer()
    : Dialog()
{
    events.clear();

    eventCombo = new ComboBox;

    auto playButton = new PushButton(QIcon::fromTheme("media-playback-start"), _("Play"));
    playButton->sigClicked().connect([this](){ play(); });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto layout = new QHBoxLayout;
    layout->addWidget(eventCombo);
    layout->addWidget(playButton);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(_("Beep Player"));
}


void BeepPlayer::load(const string& filename)
{
    events.clear();

    if(!filename.empty()) {
        BeepEventReader reader;
        if(reader.load(filename)) {
            events = reader.events();
        }
    }

    for(auto& event : events) {
        MessageView::instance()->putln(formatR(_("name: {0}, link1: {1}, link2: {2}, frequency: {3}"),
                event.name(), event.link1(), event.link2(), event.frequency()));
        eventCombo->addItem(event.name().c_str());
    }
}


void BeepPlayer::play()
{
    int index =  eventCombo->currentIndex();
    int frequency = events[index].frequency();
    int length = 200;
    if(!beeper.isActive()) {
        beeper.start(frequency, length);
    }
}