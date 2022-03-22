/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_TCAREAITEM_H
#define CNOID_NETEMPLUGIN_TCAREAITEM_H

#include <src/CFDPlugin/AreaItem.h>

namespace cnoid {

class TCAreaItemImpl;

class TCAreaItem : public AreaItem
{
public:
    TCAreaItem();
    TCAreaItem(const TCAreaItem& org);
    virtual ~TCAreaItem();

    static void initializeClass(ExtensionManager* ext);

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

#endif // CNOID_NETEMPLUGIN_TCAREAITEM_H
