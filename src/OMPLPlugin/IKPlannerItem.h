/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPLPLUGIN_IKPLANNERITEM_H
#define CNOID_OMPLPLUGIN_IKPLANNERITEM_H

#include <cnoid/ExtensionManager>
#include <cnoid/SimpleSetupItem>

namespace cnoid {

class IKPlannerItemImpl;

class IKPlannerItem : public SimpleSetupItem
{
public:
    IKPlannerItem();
    IKPlannerItem(const IKPlannerItem& org);
    virtual ~IKPlannerItem();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void prePlannerFunction() override;
    virtual bool midPlannerFunction(const ompl::base::State* state) override;
    virtual void postPlannerFunction(ompl::geometric::PathGeometric& pathes) override;
    virtual Item* doDuplicate() const override;
    virtual void onTreePathChanged() override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    IKPlannerItemImpl* impl;
    friend class IKPlannerItemImpl;
};

typedef ref_ptr<IKPlannerItem> IKPlannerItemPtr;

}

#endif // CNOID_OMPLPLUGIN_IKPLANNERITEM_H
