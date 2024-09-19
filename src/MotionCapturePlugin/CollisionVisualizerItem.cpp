/**
    @author Kenta Suzuki
*/

#include "CollisionVisualizerItem.h"
#include <cnoid/CollisionLinkPair>
#include <cnoid/ExtensionManager>
#include <cnoid/ItemManager>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class CollisionVisualizerItem::Impl
{
public:
    CollisionVisualizerItem* self;

    Impl(CollisionVisualizerItem* self);
    Impl(CollisionVisualizerItem* self, const Impl& org);
    ~Impl();
};

}


void CollisionVisualizerItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<CollisionVisualizerItem, CollisionDetectionControllerItem>(N_("CollisionVisualizerItem"));
    im.addCreationPanel<CollisionVisualizerItem>();
}


CollisionVisualizerItem::CollisionVisualizerItem()
    : CollisionDetectionControllerItem()
{
    impl = new Impl(this);
}


CollisionVisualizerItem::Impl::Impl(CollisionVisualizerItem* self)
    : self(self)
{

}


CollisionVisualizerItem::CollisionVisualizerItem(const CollisionVisualizerItem& org)
    : CollisionDetectionControllerItem(org),
      impl(new Impl(this, *org.impl))
{

}


CollisionVisualizerItem::Impl::Impl(CollisionVisualizerItem* self, const Impl& org)
    : self(self)
{

}


CollisionVisualizerItem::~CollisionVisualizerItem()
{
    delete impl;
}


CollisionVisualizerItem::Impl::~Impl()
{

}


bool CollisionVisualizerItem::initialize(ControllerIO* io)
{
    if(!CollisionDetectionControllerItem::initialize(io)) {
        return false;
    }
    return true;
}


bool CollisionVisualizerItem::start()
{
    if(!CollisionDetectionControllerItem::start()) {
        return false;
    }
    return true;
}


void CollisionVisualizerItem::input()
{
    CollisionDetectionControllerItem::input();
}


bool CollisionVisualizerItem::control()
{
    if(!CollisionDetectionControllerItem::control()) {
        return false;
    }
    return true;
}


void CollisionVisualizerItem::output()
{
    CollisionDetectionControllerItem::output();
}


void CollisionVisualizerItem::stop()
{
    CollisionDetectionControllerItem::stop();
}


Item* CollisionVisualizerItem::doCloneItem(CloneMap* cloneMap) const
{
    return new CollisionVisualizerItem(*this);
}


void CollisionVisualizerItem::onCollisionsDetected(const vector<CollisionLinkPair>& collisions)
{
    for(auto& collision : collisions) {
        Link* link1 = collision.link(0);
        Link* link2 = collision.link(1);
    }
}


void CollisionVisualizerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    CollisionDetectionControllerItem::doPutProperties(putProperty);
}


bool CollisionVisualizerItem::store(Archive& archive)
{
    if(!CollisionDetectionControllerItem::store(archive)) {
        return false;
    }
    return true;
}


bool CollisionVisualizerItem::restore(const Archive& archive)
{
    if(!CollisionDetectionControllerItem::restore(archive)) {
        return false;
    }
    return true;
}