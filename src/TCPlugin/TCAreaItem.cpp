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
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

void loadItem(Mapping& node, TCAreaItem* item)
{
    string s;
    Vector3 v;
    int i;
    double d = 0.0;

    if(node.read("inboundDelay", d)) item->setInboundDelay(d);
    if(node.read("inboundRate", d)) item->setInboundRate(d);
    if(node.read("inboundLoss", d)) item->setInboundLoss(d);
    if(node.read("outboundDelay", d)) item->setOutboundDelay(d);
    if(node.read("outboundRate", d)) item->setOutboundRate(d);
    if(node.read("outboundLoss", d)) item->setOutboundLoss(d);
    if(node.read("sourceIP", s)) item->setSource(s);
    if(node.read("destinationIP", s)) item->setDestination(s);

    if(node.read("name", s)) item->setName(s);
    if(read(node, "translation", v)) item->setTranslation(v);
    if(read(node, "rotation", v)) item->setRotation(v);
    if(node.read("type", i)) item->setType(i);
    if(read(node, "size", v)) item->setSize(v);
    if(node.read("radius", d)) item->setRadius(d);
    if(node.read("height", d)) item->setHeight(d);
    if(read(node, "diffuseColor", v)) item->setDiffuseColor(v);
    if(read(node, "emissiveColor", v)) item->setEmissiveColor(v);
    if(read(node, "specularColor", v)) item->setSpecularColor(v);
    if(node.read("shininess", d)) item->setShininess(d);
    double t = 0.9;
    if(node.read("transparency", t)) item->setTransparency(t);
    item->updateScene();
}


bool loadDocument(const string& filename)
{
    YAMLReader reader;
    if(reader.load(filename)) {
        Mapping* topNode = reader.loadDocument(filename)->toMapping();
        Listing* trafficList = topNode->findListing("traffics");
        if(trafficList->isValid()) {
            for(int i = 0; i < trafficList->size(); i++) {
                TCAreaItem* item = new TCAreaItem();
                Mapping* node = trafficList->at(i)->toMapping();
                loadItem(*node, item);
            }
        }
    }
    return true;
}


bool loadDocument(TCAreaItem* item, const string& filename)
{
    YAMLReader reader;
    if(reader.load(filename)) {
        Mapping* topNode = reader.loadDocument(filename)->toMapping();
        Listing* trafficList = topNode->findListing("traffics");
        if(trafficList->isValid()) {
            for(int i = 0; i < trafficList->size(); i++) {
                Mapping* node = trafficList->at(i)->toMapping();
                loadItem(*node, item);
            }
        }
    }
    return true;
}


void putKeyVector3(YAMLWriter* writer, const string& key, const Vector3& value)
{
    writer->putKey(key);
    writer->startFlowStyleListing();
    for(int i = 0; i < 3; i++) {
        writer->putScalar(value[i]);
    }
    writer->endListing();
}

}


namespace cnoid {

class TCAreaItemImpl
{
public:
    TCAreaItemImpl(TCAreaItem* self);
    TCAreaItemImpl(TCAreaItem* self, const TCAreaItemImpl& org);
    TCAreaItem* self;

    int id_;
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
    id_ = INT_MAX;
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
    id_ = org.id_;
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
    ItemManager& im = ext->itemManager();

    im.registerClass<TCAreaItem>(N_("TCAreaItem"));
    im.addCreationPanel<TCAreaItem>();

//    im.addLoaderAndSaver<TCAreaItem>(
//        _("TC Area"), "TC-AREA-FILE", "yaml;yml",
//        [](TCAreaItem* item, const string& filename, std::ostream& os, Item*){ return load(item, filename); },
//        [](TCAreaItem* item, const string& filename, std::ostream& os, Item*){ return save(item, filename); },
//        ItemManager::PRIORITY_CONVERSION);
}


bool TCAreaItem::load(TCAreaItem* item, const string& filename)
{
    if(!loadDocument(item, filename)) {
        return false;
    }
    return true;
}


bool TCAreaItem::save(TCAreaItem* item, const string& filename)
{
    if(!filename.empty()) {
        YAMLWriter writer(filename);

        writer.startMapping();
        writer.putKey("traffics");
        writer.startListing();

        writer.startMapping();
        writer.putKeyValue("name", item->name());
        putKeyVector3(&writer, "translation", item->translation());
        putKeyVector3(&writer, "rotation", item->rotation());
        writer.putKeyValue("type", item->type());
        if(item->type() == AreaItem::BOX) {
            putKeyVector3(&writer, "size", item->size());
        } else if(item->type() == AreaItem::CYLINDER) {
            writer.putKeyValue("radius", item->radius());
            writer.putKeyValue("height", item->height());
        } else if(item->type() == AreaItem::SPHERE) {
            writer.putKeyValue("radius", item->radius());
        }
        writer.putKeyValue("inboundDelay", item->inboundDelay());
        writer.putKeyValue("inboundRate", item->inboundRate());
        writer.putKeyValue("inboundLoss", item->inboundLoss());
        writer.putKeyValue("outboundDelay", item->outboundDelay());
        writer.putKeyValue("outboundRate", item->outboundRate());
        writer.putKeyValue("outboundLoss", item->outboundLoss());

        writer.putKeyValue("sourceIP", item->source());
        writer.putKeyValue("destinationIP", item->destination());

        putKeyVector3(&writer, "diffuseColor", item->diffuseColor());
        putKeyVector3(&writer, "emissiveColor", item->emissiveColor());
        putKeyVector3(&writer, "specularColor", item->specularColor());

        writer.putKeyValue("shininess", item->shininess());
        writer.putKeyValue("transparency", item->transparency());
        writer.endMapping();

        writer.endListing();
        writer.endMapping();
    }
    return true;
}


void TCAreaItem::setId(const int& id)
{
    impl->id_ = id;
}


int TCAreaItem::id() const
{
    return impl->id_;
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
    impl->doPutProperties(putProperty);
    AreaItem::doPutProperties(putProperty);
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
