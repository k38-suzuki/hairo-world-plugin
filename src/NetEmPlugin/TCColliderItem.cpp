/**
   @author Kenta Suzuki
*/

#include "TCColliderItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace std;
using namespace cnoid;

void TCColliderItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<TCColliderItem>(N_("TCColliderItem"))
        .addCreationPanel<TCColliderItem>();
}


TCColliderItem::TCColliderItem()
{
    setDiffuseColor(Vector3(1.0, 1.0, 0.0));
    inboundDelay_ = 0.0;
    inboundRate_ = 0.0;
    inboundLoss_ = 0.0;
    outboundDelay_ = 0.0;
    outboundRate_ = 0.0;
    outboundLoss_ = 0.0;
    source_.clear();
    destination_.clear();
}


TCColliderItem::TCColliderItem(const TCColliderItem& org)
    : SimpleColliderItem(org)
{
    inboundDelay_ = org.inboundDelay_;
    inboundRate_ = org.inboundRate_;
    inboundLoss_ = org.inboundLoss_;
    outboundDelay_ = org.outboundDelay_;
    outboundRate_ = org.outboundRate_;
    outboundLoss_ = org.outboundLoss_;
    source_ = org.source_;
    destination_ = org.destination_;
}


Item* TCColliderItem::doCloneItem(CloneMap* cloneMap) const
{
    return new TCColliderItem(*this);
}


void TCColliderItem::doPutProperties(PutPropertyFunction& putProperty)
{
    SimpleColliderItem::doPutProperties(putProperty);
    putProperty.min(0.0).max(100000.0)(_("InboundDelay"), inboundDelay_, changeProperty(inboundDelay_));
    putProperty.min(0.0).max(11000000.0)(_("InboundRate"), inboundRate_, changeProperty(inboundRate_));
    putProperty.min(0.0).max(100.0)(_("InboundLoss"), inboundLoss_, changeProperty(inboundLoss_));
    putProperty.min(0.0).max(100000.0)(_("OutboundDelay"), outboundDelay_, changeProperty(outboundDelay_));
    putProperty.min(0.0).max(11000000.0)(_("OutboundRate"), outboundRate_, changeProperty(outboundRate_));
    putProperty.min(0.0).max(100.0)(_("OutboundLoss"), outboundLoss_, changeProperty(outboundLoss_));
    putProperty(_("Source IP"), source_, changeProperty(source_));
    putProperty(_("Destination IP"), destination_, changeProperty(destination_));
}


bool TCColliderItem::store(Archive& archive)
{
    SimpleColliderItem::store(archive);
    archive.write("inbound_delay", inboundDelay_);
    archive.write("inbound_rate", inboundRate_);
    archive.write("inbound_loss", inboundLoss_);
    archive.write("outbound_delay", outboundDelay_);
    archive.write("outbound_rate", outboundRate_);
    archive.write("outbound_loss", outboundLoss_);
    archive.write("source", source_);
    archive.write("destination", destination_);
    return true;
}


bool TCColliderItem::restore(const Archive& archive)
{
    SimpleColliderItem::restore(archive);
    archive.read("inbound_delay", inboundDelay_);
    archive.read("inbound_rate", inboundRate_);
    archive.read("inbound_loss", inboundLoss_);
    archive.read("outbound_delay", outboundDelay_);
    archive.read("outbound_rate", outboundRate_);
    archive.read("outbound_loss", outboundLoss_);
    archive.read("source", source_);
    archive.read("destination", destination_);
    return true;
}
