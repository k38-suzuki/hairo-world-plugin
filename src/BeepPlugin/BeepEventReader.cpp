/**
    @author Kenta Suzuki
*/

#include "BeepEventReader.h"
#include <cnoid/Format>
#include <cnoid/NullOut>
#include <cnoid/MessageView>
#include <cnoid/YAMLReader>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BeepEventReader::Impl
{
public:
    BeepEventReader* self;

    Impl(BeepEventReader* self);
    ~Impl();

    bool load(const string& filename, ostream& os = nullout());

    vector<BeepEvent> events;
};

}


BeepEvent::BeepEvent()
{
    name_.clear();
    link1_.clear();
    link2_.clear();
    frequency_ = 440;
    length_ = 200;
    is_enabled_ = false;
}


BeepEvent::BeepEvent(const BeepEvent& org)
{
    name_ = org.name_;
    link1_ = org.link1_;
    link2_ = org.link2_;
    frequency_ = org.frequency_;
    length_ = org.length_;
    is_enabled_ = org.is_enabled_;
}


BeepEventReader::BeepEventReader()
{
    impl = new Impl(this);
}


BeepEventReader::Impl::Impl(BeepEventReader* self)
    : self(self)
{
    events.clear();
}


BeepEventReader::~BeepEventReader()
{
    delete impl;
}


BeepEventReader::Impl::~Impl()
{

}


vector<BeepEvent> BeepEventReader::events()
{
    return impl->events;
}


bool BeepEventReader::load(const string& filename)
{
    if(!filename.empty()) {
        return impl->load(filename);
    }
    return false;
}


bool BeepEventReader::Impl::load(const string& filename, ostream& os)
{
    events.clear();

    try {
        YAMLReader reader;
        auto archive = reader.loadDocument(filename)->toMapping();
        if(archive) {
            auto& eventList = *archive->findListing("events");
            if(eventList.isValid()) {
                for(int i = 0; i < eventList.size(); ++i) {
                    Mapping* node = eventList[i].toMapping();
                    BeepEvent event;

                    event.setName(node->get("name", ""));
                    event.setFrequency(node->get("frequency", 0));
                    event.setLength(node->get("length", 0));

                    auto& pairList = *node->findListing("pair");
                    if(pairList.isValid() && pairList.size() == 2) {
                        string link1 = pairList[0].toString();
                        string link2 = pairList[1].toString();
                        event.setLinkPairs(link1, link2);
                    }

                    events.push_back(event);
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    MessageView::instance()->putln(formatR(_("Beep events were loaded.")));

    return true;
}