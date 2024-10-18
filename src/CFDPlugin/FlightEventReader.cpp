/**
    @author Kenta Suzuki
*/

#include "FlightEventReader.h"
#include <cnoid/Format>
#include <cnoid/NullOut>
#include <cnoid/MessageView>
#include <cnoid/YAMLReader>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

FlightEvent::FlightEvent()
{
    name_  = "";
    mass_ = 0.0;
    duration_ = 0.0;
}

FlightEvent::FlightEvent(const FlightEvent& org)
{
    name_ = org.name_;
    mass_ = org.mass_;
    duration_ = org.duration_;
}

class FlightEventReader::Impl
{
public:
    FlightEventReader* self;

    Impl(FlightEventReader* self);
    ~Impl();

    bool load(const string& filename, ostream& os = nullout());

    vector<FlightEvent> events;
};

}


FlightEventReader::FlightEventReader()
{
    impl = new Impl(this);
}


FlightEventReader::Impl::Impl(FlightEventReader* self)
    : self(self)
{
    events.clear();
}


FlightEventReader::~FlightEventReader()
{
    delete impl;
}


FlightEventReader::Impl::~Impl()
{

}


vector<FlightEvent> FlightEventReader::events()
{
    return impl->events;
}


bool FlightEventReader::load(const string& filename)
{
    if(!filename.empty()) {
        return impl->load(filename);
    }
    return false;
}


bool FlightEventReader::Impl::load(const string& filename, ostream& os)
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
                    FlightEvent event;

                    event.setMass(node->get("mass", 0.0));
                    event.setDuration(node->get("duration", 0.0));

                    event.setName(node->get("name", ""));

                    events.push_back(event);
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    MessageView::instance()->putln(formatR(_("Flight events were loaded.")));

    return true;
}