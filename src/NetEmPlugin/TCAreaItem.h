/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_TCAREAITEM_H
#define CNOID_NETEMPLUGIN_TCAREAITEM_H

#include <cnoid/FloatingNumberString>
#include <cnoid/AreaItem>

namespace cnoid {

class TCAreaItemImpl;

class TCAreaItem : public AreaItem
{
public:
    TCAreaItem();
    TCAreaItem(const TCAreaItem& org);
    virtual ~TCAreaItem();

    static void initializeClass(ExtensionManager* ext);

    double inboundDelay() const { return inboundDelay_.value(); }
    double inboundRate() const { return inboundRate_.value(); }
    double inboundLoss() const { return inboundLoss_.value(); }
    double outboundDelay() const { return outboundDelay_.value(); }
    double outboundRate() const { return outboundRate_.value(); }
    double outboundLoss() const { return outboundLoss_.value(); }
    std::string source() const { return source_; }
    std::string destination() const { return destination_; }

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    FloatingNumberString inboundDelay_;
    FloatingNumberString inboundRate_;
    FloatingNumberString inboundLoss_;
    FloatingNumberString outboundDelay_;
    FloatingNumberString outboundRate_;
    FloatingNumberString outboundLoss_;
    std::string source_;
    std::string destination_;
};

typedef ref_ptr<TCAreaItem> TCAreaItemPtr;

}

#endif // CNOID_NETEMPLUGIN_TCAREAITEM_H
