/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_TC_COLLIDER_ITEM_H
#define CNOID_NETEMPLUGIN_TC_COLLIDER_ITEM_H

#include <cnoid/SimpleColliderItem>

namespace cnoid {

class TCColliderItem : public SimpleColliderItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    TCColliderItem();
    TCColliderItem(const TCColliderItem& org);

    double inboundDelay() const { return inboundDelay_; }
    double inboundRate() const { return inboundRate_; }
    double inboundLoss() const { return inboundLoss_; }
    double outboundDelay() const { return outboundDelay_; }
    double outboundRate() const { return outboundRate_; }
    double outboundLoss() const { return outboundLoss_; }
    std::string source() const { return source_; }
    std::string destination() const { return destination_; }

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    double inboundDelay_;
    double inboundRate_;
    double inboundLoss_;
    double outboundDelay_;
    double outboundRate_;
    double outboundLoss_;
    std::string source_;
    std::string destination_;
};

typedef ref_ptr<TCColliderItem> TCColliderItemPtr;

}

#endif
