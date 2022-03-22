/**
   \file
   \author Kenta Suzuki
*/

#include "TCAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class TCAreaItemImpl
{
public:
    TCAreaItemImpl(TCAreaItem* self);
    TCAreaItemImpl(TCAreaItem* self, const TCAreaItemImpl& org);
    TCAreaItem* self;

    FloatingNumberString inboundDelay;
    FloatingNumberString inboundRate;
    FloatingNumberString inboundLoss;
    FloatingNumberString outboundDelay;
    FloatingNumberString outboundRate;
    FloatingNumberString outboundLoss;
    string source;
    string destination;

    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


TCAreaItem::TCAreaItem()
{
    impl = new TCAreaItemImpl(this);
}


TCAreaItemImpl::TCAreaItemImpl(TCAreaItem* self)
    : self(self)
{
    inboundDelay = 0.0;
    inboundRate = 0.0;
    inboundLoss = 0.0;
    outboundDelay = 0.0;
    outboundRate = 0.0;
    outboundLoss = 0.0;
    source.clear();
    destination.clear();
}


TCAreaItem::TCAreaItem(const TCAreaItem& org)
    : AreaItem(org),
      impl(new TCAreaItemImpl(this, *org.impl))
{

}


TCAreaItemImpl::TCAreaItemImpl(TCAreaItem* self, const TCAreaItemImpl& org)
    : self(self)
{
    inboundDelay = org.inboundDelay;
    inboundRate = org.inboundRate;
    inboundLoss = org.inboundLoss;
    outboundDelay = org.outboundDelay;
    outboundRate = org.outboundRate;
    outboundLoss = org.outboundLoss;
    source = org.source;
    destination = org.destination;
}


TCAreaItem::~TCAreaItem()
{
    delete impl;
}


void TCAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<TCAreaItem>(N_("TCAreaItem"));
    ext->itemManager().addCreationPanel<TCAreaItem>();
}


void TCAreaItem::setInboundDelay(const double& inboundDelay)
{
    impl->inboundDelay = inboundDelay;
}


double TCAreaItem::inboundDelay() const
{
    return impl->inboundDelay.value();
}


void TCAreaItem::setInboundRate(const double& inboundRate)
{
    impl->inboundRate = inboundRate;
}


double TCAreaItem::inboundRate() const
{
    return impl->inboundRate.value();
}


void TCAreaItem::setInboundLoss(const double& inboundLoss)
{
    impl->inboundLoss = inboundLoss;
}


double TCAreaItem::inboundLoss() const
{
    return impl->inboundLoss.value();
}


void TCAreaItem::setOutboundDelay(const double& outboundDelay)
{
    impl->outboundDelay = outboundDelay;
}


double TCAreaItem::outboundDelay() const
{
    return impl->outboundDelay.value();
}


void TCAreaItem::setOutboundRate(const double& outboundRate)
{
    impl->outboundRate = outboundRate;
}


double TCAreaItem::outboundRate() const
{
    return impl->outboundRate.value();
}


void TCAreaItem::setOutboundLoss(const double& outboundLoss)
{
    impl->outboundLoss = outboundLoss;
}


double TCAreaItem::outboundLoss() const
{
    return impl->outboundLoss.value();
}


void TCAreaItem::setSource(const string& source)
{
    impl->source = source;
}


string TCAreaItem::source() const
{
    return impl->source;
}


void TCAreaItem::setDestination(const string& destination)
{
    impl->destination = destination;
}


string TCAreaItem::destination() const
{
    return impl->destination;
}


Item* TCAreaItem::doDuplicate() const
{
    return new TCAreaItem(*this);
}


void TCAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void TCAreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("InboundDelay"), inboundDelay,
                [&](const string& v){ return inboundDelay.setNonNegativeValue(v); });
    putProperty(_("InboundRate"), inboundRate,
                [&](const string& v){ return inboundRate.setNonNegativeValue(v); });
    putProperty(_("InboundLoss"), inboundLoss,
                [&](const string& v){ return inboundLoss.setNonNegativeValue(v); });
    putProperty(_("OutboundDelay"), outboundDelay,
                [&](const string& v){ return outboundDelay.setNonNegativeValue(v); });
    putProperty(_("OutboundRate"), outboundRate,
                [&](const string& v){ return outboundRate.setNonNegativeValue(v); });
    putProperty(_("OutboundLoss"), outboundLoss,
                [&](const string& v){ return outboundLoss.setNonNegativeValue(v); });
    putProperty(_("Source IP"), source, changeProperty(source));
    putProperty(_("Destination IP"), destination, changeProperty(destination));
}


bool TCAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
    return impl->store(archive);
}


bool TCAreaItemImpl::store(Archive& archive)
{
    archive.write("inbound_delay", inboundDelay);
    archive.write("inbound_rate", inboundRate);
    archive.write("inbound_loss", inboundLoss);
    archive.write("outbound_delay", outboundDelay);
    archive.write("outbound_rate", outboundRate);
    archive.write("outbound_loss", outboundLoss);
    archive.write("source", source);
    archive.write("destination", destination);
    return true;
}


bool TCAreaItem::restore(const Archive& archive)
{
    AreaItem::restore(archive);
    return impl->restore(archive);
}


bool TCAreaItemImpl::restore(const Archive& archive)
{
    inboundDelay = archive.get("inbound_delay", inboundDelay.string());
    inboundRate = archive.get("inbound_rate", inboundRate.string());
    inboundLoss = archive.get("inbound_loss", inboundLoss.string());
    outboundDelay = archive.get("outbound_delay", outboundDelay.string());
    outboundRate = archive.get("outbound_rate", outboundRate.string());
    outboundLoss = archive.get("outbound_loss", outboundLoss.string());
    archive.read("source", source);
    archive.read("destination", destination);
    return true;
}
