/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPLPLUGIN_MOTIONPLANNERITEM_H
#define CNOID_OMPLPLUGIN_MOTIONPLANNERITEM_H

#include <cnoid/EigenTypes>
#include <cnoid/Item>
#include <ompl/geometric/SimpleSetup.h>

namespace cnoid {

class MotionPlannerItemImpl;

class MotionPlannerItem : public Item
{
public:
    MotionPlannerItem();
    MotionPlannerItem(const MotionPlannerItem& org);
    virtual ~MotionPlannerItem();

    static void initializeClass(ExtensionManager* ext);

    void setStartPosition(const Vector3& startPosition);
    void setGoalPosition(const Vector3& goalPosition);
    void planWithSimpleSetup();
    bool isSolved() const;

protected:
    virtual void prePlannerFunction();
    virtual bool midPlannerFunction(const ompl::base::State* state);
    virtual void postPlannerFunction(ompl::geometric::PathGeometric& pathes);
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    MotionPlannerItemImpl* impl;
    friend class MotionPlannerItemImpl;
};

typedef ref_ptr<MotionPlannerItem> MotionPlannerItemPtr;

}

#endif // CNOID_OMPLPLUGIN_MOTIONPLANNERITEM_H
