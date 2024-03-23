/**
   @author Kenta Suzuki
*/

#include "TCAreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/ItemManager>
#include <cnoid/PutPropertyFunction>
#include "gettext.h"

using namespace cnoid;
using namespace std;

TCAreaItem::TCAreaItem()
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


TCAreaItem::TCAreaItem(const TCAreaItem& org)
    : AreaItem(org)
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


TCAreaItem::~TCAreaItem()
{

}


void TCAreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
            .registerClass<TCAreaItem>(N_("TCAreaItem"))
            .addCreationPanel<TCAreaItem>();
}


Item* TCAreaItem::doCloneItem(CloneMap* cloneMap) const
{
    return new TCAreaItem(*this);
}


void TCAreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    AreaItem::doPutProperties(putProperty);
    putProperty(_("InboundDelay"), inboundDelay_,
                [&](const string& v){ return inboundDelay_.setNonNegativeValue(v); });
    putProperty(_("InboundRate"), inboundRate_,
                [&](const string& v){ return inboundRate_.setNonNegativeValue(v); });
    putProperty(_("InboundLoss"), inboundLoss_,
                [&](const string& v){ return inboundLoss_.setNonNegativeValue(v); });
    putProperty(_("OutboundDelay"), outboundDelay_,
                [&](const string& v){ return outboundDelay_.setNonNegativeValue(v); });
    putProperty(_("OutboundRate"), outboundRate_,
                [&](const string& v){ return outboundRate_.setNonNegativeValue(v); });
    putProperty(_("OutboundLoss"), outboundLoss_,
                [&](const string& v){ return outboundLoss_.setNonNegativeValue(v); });
    putProperty(_("Source IP"), source_, changeProperty(source_));
    putProperty(_("Destination IP"), destination_, changeProperty(destination_));
}


bool TCAreaItem::store(Archive& archive)
{
    AreaItem::store(archive);
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


bool TCAreaItem::restore(const Archive& archive)
{
    AreaItem::restore(archive);
    inboundDelay_ = archive.get("inbound_delay", inboundDelay_.string());
    inboundRate_ = archive.get("inbound_rate", inboundRate_.string());
    inboundLoss_ = archive.get("inbound_loss", inboundLoss_.string());
    outboundDelay_ = archive.get("outbound_delay", outboundDelay_.string());
    outboundRate_ = archive.get("outbound_rate", outboundRate_.string());
    outboundLoss_ = archive.get("outbound_loss", outboundLoss_.string());
    archive.read("source", source_);
    archive.read("destination", destination_);
    return true;
}
