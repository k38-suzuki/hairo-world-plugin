/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_TC_PLUGIN_TC_AREA_ITEM_H
#define CNOID_TC_PLUGIN_TC_AREA_ITEM_H

#include <src/FluidDynamicsPlugin/AreaItem.h>

namespace cnoid {

class TCAreaItemImpl;

class TCAreaItem : public AreaItem
{
public:
    TCAreaItem();
    TCAreaItem(const TCAreaItem& org);
    virtual ~TCAreaItem();

    static void initializeClass(ExtensionManager* ext);

    void setId(const int& id);
    int id() const;
    void setInboundDelay(const double& inboundDelay);
    double inboundDelay() const;
    void setInboundRate(const double& inboundRate);
    double inboundRate() const;
    void setInboundLoss(const double& inboundLoss);
    double inboundLoss() const;
    void setOutboundDelay(const double& outboundDelay);
    double outboundDelay() const;
    void setOutboundRate(const double& outboundRate);
    double outboundRate() const;
    void setOutboundLoss(const double& outboundLoss);
    double outboundLoss() const;
    void setSource(const std::string& source);
    std::string source() const;
    void setDestination(const std::string& destination);
    std::string destination() const;

    static bool load(TCAreaItem* item, const std::string& filename);
    static bool save(TCAreaItem* item, const std::string& filename);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    TCAreaItemImpl* impl;
    friend class TCAreaItemImpl;
};

typedef ref_ptr<TCAreaItem> TCAreaItemPtr;

}

#endif // CNOID_TC_PLUGIN_TC_AREA_ITEM_H
