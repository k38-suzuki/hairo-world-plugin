/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPLPLUGIN_IKMOTIONPLANNERITEM_H
#define CNOID_OMPLPLUGIN_IKMOTIONPLANNERITEM_H

#include <cnoid/ExtensionManager>
#include <cnoid/Item>

namespace cnoid {

class IKMotionPlannerItemImpl;

class IKMotionPlannerItem : public Item
{
public:
    IKMotionPlannerItem();
    IKMotionPlannerItem(const IKMotionPlannerItem& org);
    virtual ~IKMotionPlannerItem();

    static void initializeClass(ExtensionManager* ext);

    enum PlannerType {
        RRT,
        RRTCONNECT,
        RRTSTAR,
        PRRT,
        NUM_PLANNERS
    };

protected:
    virtual Item* doDuplicate() const override;
    virtual void onTreePathChanged() override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    IKMotionPlannerItemImpl* impl;
    friend class IKMotionPlannerItemImpl;
};

typedef ref_ptr<IKMotionPlannerItem> IKMotionPlannerItemPtr;

}

#endif // CNOID_OMPLPLUGIN_IKMOTIONPLANNERITEM_H
