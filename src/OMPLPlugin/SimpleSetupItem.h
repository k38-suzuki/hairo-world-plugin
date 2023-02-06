/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OMPL_PLUGIN_SIMPLE_SETUP_ITEM_H
#define CNOID_OMPL_PLUGIN_SIMPLE_SETUP_ITEM_H

#include <cnoid/Item>
#include <cnoid/LocatableItem>
#include <cnoid/RenderableItem>
#include "SimpleSetup.h"
#include "exportdecl.h"

namespace cnoid {

class SimpleSetupItemImpl;

class CNOID_EXPORT SimpleSetupItem : public Item, public SimpleSetup, public LocatableItem, public RenderableItem
{
public:
    SimpleSetupItem();
    SimpleSetupItem(const SimpleSetupItem& org);
    virtual ~SimpleSetupItem();

    static void initializeClass(ExtensionManager* ext);

    void setRegionOffset(const Isometry3& T);
    const Isometry3& regionOffset() const;

    // virtual LocationProxyPtr getLocationProxy() override;

    virtual SgNode* getScene() override;

protected:
    virtual void prePlannerFunction() override;
    virtual bool midPlannerFunction(const ompl::base::State* state) override;
    virtual void postPlannerFunction(ompl::geometric::PathGeometric& pathes) override;
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    SimpleSetupItemImpl* impl;
    friend class SimpleSetupItemImpl;
};

typedef ref_ptr<SimpleSetupItem> SimpleSetupItemPtr;

}

#endif // CNOID_OMPL_PLUGIN_SIMPLE_SETUP_ITEM_H