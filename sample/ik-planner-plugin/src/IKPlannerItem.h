/**
   @author Kenta Suzuki
*/

#ifndef CNOID_IK_PLANNER_PLUGIN_IK_PLANNER_ITEM_H
#define CNOID_IK_PLANNER_PLUGIN_IK_PLANNER_ITEM_H

#include <cnoid/ExtensionManager>
#include <cnoid/SimpleSetupItem>

namespace cnoid {

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
    class Impl;
    Impl* impl;
};

typedef ref_ptr<IKPlannerItem> IKPlannerItemPtr;

}

#endif
