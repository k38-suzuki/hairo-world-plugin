/**
   @author Kenta Suzuki
*/

#ifndef CNOID_IKPLANNER_PLUGIN_IK_PLANNER_ITEM_H
#define CNOID_IKPLANNER_PLUGIN_IK_PLANNER_ITEM_H

#include <cnoid/SimpleSetupItem>

namespace cnoid {

class ExtensionManager;

class IKPlannerItem : public SimpleSetupItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    IKPlannerItem();
    IKPlannerItem(const IKPlannerItem& org);
    virtual ~IKPlannerItem();

protected:
    virtual void prePlannerFunction() override;
    virtual bool midPlannerFunction(const ompl::base::State* state) override;
    virtual void postPlannerFunction(ompl::geometric::PathGeometric& pathes) override;
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
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

#endif // CNOID_IKPLANNER_PLUGIN_IK_PLANNER_ITEM_H
