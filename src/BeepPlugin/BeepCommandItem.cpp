/**
    @author Kenta Suzuki
*/

#include "BeepCommandItem.h"
#include <cnoid/Archive>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/ItemManager>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/MessageView>
#include <cnoid/Sleep>
#include <cnoid/Process>
#include <cnoid/PutPropertyFunction>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class BeepCommandItem::Impl
{
public:
    BeepCommandItem* self;

    Impl(BeepCommandItem* self);
    Impl(BeepCommandItem* self, const Impl& org);
    ~Impl();

    bool execute();
    bool terminate();
    void onReadyReadServerProcessOutput();
        
    string command;
    int frequency;
    int length;
    Process process;
    double waiting_time_after_started;
    bool is_message_enabled;
};

}


void BeepCommandItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<BeepCommandItem>(N_("BeepCommandItem"));
    // im.addCreationPanel<BeepCommandItem>();

    ItemTreeView::customizeContextMenu<BeepCommandItem>(
        [](BeepCommandItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
            menuManager.addItem(_("Execute"))->sigTriggered().connect([item](){ item->execute(); });
            menuManager.addItem(_("Terminate"))->sigTriggered().connect([item](){ item->terminate(); });
            menuManager.addSeparator();
            menuFunction.dispatchAs<Item>(item);
        });
}


BeepCommandItem::BeepCommandItem()
    : Item()
{
    impl = new Impl(this);
}


BeepCommandItem::Impl::Impl(BeepCommandItem* self)
    : self(self)
{
    command = "beep";
    frequency = 440;
    length = 200;
    waiting_time_after_started = 0.0;
    is_message_enabled = true;

    process.sigReadyReadStandardOutput().connect(
        [&](){ onReadyReadServerProcessOutput(); });
}


BeepCommandItem::BeepCommandItem(const BeepCommandItem& org)
    : Item(org),
      impl(new Impl(this, *org.impl))
{

}


BeepCommandItem::Impl::Impl(BeepCommandItem* self, const Impl& org)
    : self(self)
{
    command = org.command;
    frequency = org.frequency;
    length = org.length;
    waiting_time_after_started = org.waiting_time_after_started;
    is_message_enabled = org.is_message_enabled;

    process.sigReadyReadStandardOutput().connect(
        [&](){ onReadyReadServerProcessOutput(); });
}


BeepCommandItem::~BeepCommandItem()
{
    delete impl;
}


BeepCommandItem::Impl::~Impl()
{

}

void BeepCommandItem::setFrequency(const int& frequency)
{
    impl->frequency = frequency;
}


int BeepCommandItem::frequency() const
{
    return impl->frequency;
}


void BeepCommandItem::setLength(const int& length)
{
    impl->length = length;
}


int BeepCommandItem::length() const
{
    return impl->length;
}


double BeepCommandItem::waitingTimeAfterStarted() const
{
    return impl->waiting_time_after_started;
}


void BeepCommandItem::setWaitingTimeAfterStarted(double time)
{
    impl->waiting_time_after_started = time;
}


void BeepCommandItem::showMessage(const bool checked)
{
    impl->is_message_enabled = checked;
}


bool BeepCommandItem::execute()
{
    return impl->execute();
}


bool BeepCommandItem::Impl::execute()
{
    bool result = false;
    
    MessageView* mv = MessageView::instance();
    
    if(!command.empty()) {
        terminate();
        string actual_command(command);
        string actual_arguments = formatR("-f {0} -l {1}", frequency, length).c_str();
        QStringList arguments = QStringList() << actual_arguments.c_str();
#ifdef _WIN32
        if(filesystem::path(fromUTF8(actual_command)).extension() != ".exe") {
            actual_command += ".exe";
        }
        // quote the command string to support a path including spaces
        process.start(QString("\"") + actual_command.c_str() + "\"", arguments);
#else
        process.start(actual_command.c_str(), arguments);
#endif

        if(process.waitForStarted()) {
            if(is_message_enabled) {
                mv->putln(
                    formatR(_("External command \"{0}\" has been executed by item \"{1}\"."),
                            actual_command + " " + actual_arguments, self->displayName()));
            }
            if(waiting_time_after_started > 0.0) {
                msleep(waiting_time_after_started * 1000.0);
            }
            
            result = true;

        } else {
            mv->put(formatR(_("External command \"{}\" cannot be executed."), actual_command));
            if(!filesystem::exists(fromUTF8(actual_command))) {
                if(is_message_enabled) {
                    mv->putln(_(" The command does not exist."));
                }
            } else {
                if(is_message_enabled) {
                    mv->putln("");
                }
            }
        }
    }
    return result;
}


bool BeepCommandItem::terminate()
{
    return impl->terminate();
}


bool BeepCommandItem::Impl::terminate()
{
    if(process.state() != QProcess::NotRunning) {
        process.kill();
        return process.waitForFinished(100);
    }
    return false;
}


void BeepCommandItem::Impl::onReadyReadServerProcessOutput()
{
    MessageView::instance()->put(QString(process.readAll()));
}


void BeepCommandItem::onDisconnectedFromRoot()
{
    terminate();
}


Item* BeepCommandItem::doCloneItem(CloneMap* cloneMap) const
{
    return new BeepCommandItem(*this);
}


void BeepCommandItem::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty.min(0)(_("Frequency"), impl->frequency, changeProperty(impl->frequency));
    putProperty.min(0)(_("Length"), impl->length, changeProperty(impl->length));
    putProperty(_("Waiting time after started"), impl->waiting_time_after_started,
                changeProperty(impl->waiting_time_after_started));
    putProperty(_("Show message"), impl->is_message_enabled,
                changeProperty(impl->is_message_enabled));
}


bool BeepCommandItem::store(Archive& archive)
{
    archive.write("frequency", impl->frequency);
    archive.write("length", impl->length);
    archive.write("waiting_time_after_started", impl->waiting_time_after_started);
    archive.write("is_message_enabled", impl->is_message_enabled);
    return true;
}


bool BeepCommandItem::restore(const Archive& archive)
{
    archive.read("frequency", impl->frequency);
    archive.read("length", impl->length);
    archive.read("waiting_time_after_started", impl->waiting_time_after_started);
    archive.read("is_message_enabled", impl->is_message_enabled);
    return true;
}