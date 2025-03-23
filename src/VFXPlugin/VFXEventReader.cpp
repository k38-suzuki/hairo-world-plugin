/**
    @author Kenta Suzuki
*/

#include "VFXEventReader.h"
#include <cnoid/EigenArchive>
#include <cnoid/Format>
#include <cnoid/NullOut>
#include <cnoid/MessageView>
#include <cnoid/YAMLReader>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

VFXEvent::VFXEvent()
    : VisualEffect()
{
    name_ = "";
    begin_time_ = 0.0;
    end_time_ = 0.0;
    duration_ = 0.0;
    cycle_ << 0.0, 0.0;
    target_colliders_.clear();
    is_enabled_ = false;
}


VFXEvent::VFXEvent(const VFXEvent& org)
    : VisualEffect(org)
{
    name_ = org.name_;
    begin_time_ = org.begin_time_;
    end_time_ = org.end_time_;
    duration_ = org.duration_;
    cycle_ = org.cycle_;
    target_colliders_ = org.target_colliders_;
    is_enabled_ = org.is_enabled_;
}


class VFXEventReader::Impl
{
public:
    VFXEventReader* self;

    Impl(VFXEventReader* self);
    ~Impl();

    bool load(const string& filename, ostream& os = nullout());

    vector<VFXEvent> events;
};

}


VFXEventReader::VFXEventReader()
{
    impl = new Impl(this);
}


VFXEventReader::Impl::Impl(VFXEventReader* self)
    : self(self)
{
    events.clear();
}


VFXEventReader::~VFXEventReader()
{
    delete impl;
}


VFXEventReader::Impl::~Impl()
{

}


vector<VFXEvent> VFXEventReader::events()
{
    return impl->events;
}


bool VFXEventReader::load(const string& filename)
{
    if(!filename.empty()) {
        return impl->load(filename);
    }
    return false;
}


bool VFXEventReader::Impl::load(const string& filename, ostream& os)
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
                    VFXEvent event;

                    Vector3 hsv;
                    if(read(node, "hsv", hsv)) {
                        event.setHsv(hsv);
                    }

                    Vector3 rgb;
                    if(read(node, "rgb", rgb)) {
                        event.setRgb(rgb);
                    }

                    event.setCoefB(node->get("coef_b", 0.0));
                    event.setCoefD(node->get("coef_d", 0.0));
                    event.setStdDev(node->get("std_dev", 0.0));
                    event.setSaltAmount(node->get("salt_amount", 0.0));
                    event.setSaltChance(node->get("salt_chance", 0.0));
                    event.setPepperAmount(node->get("pepper_amount", 0.0));
                    event.setPepperChance(node->get("pepper_chance", 0.0));
                    event.setMosaicChance(node->get("mosaic_chance", 0.0));
                    event.setKernel(node->get("kernel", 16));

                    event.setName(node->get("name", ""));
                    event.setBeginTime(node->get("begin_time", 0.0));
                    event.setEndTime(node->get("end_time", 0.0));
                    event.setDuration(node->get("duration", 0.0));

                    Vector2 cycle;
                    if(read(node, "cycle", cycle)) {
                        event.setCycle(cycle);
                    }

                    auto& colliderList = *node->findListing("target_collider");
                    if(colliderList.isValid()) {
                        for(int j = 0; j < colliderList.size(); ++j) {
                            string collider = colliderList[j].toString();
                            event.addTargetCollider(collider);
                        }
                    }

                    events.push_back(event);
                }
            }
        }
    }
    catch (const ValueNode::Exception& ex) {
        os << ex.message();
    }

    MessageView::instance()->putln(formatR(_("VFX events were loaded.")));

    return true;
}