/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OPERATION_REVIEW_PLUGIN_OPERATION_REVIEW_COLLECTOR_ITEM_H
#define CNOID_OPERATION_REVIEW_PLUGIN_OPERATION_REVIEW_COLLECTOR_ITEM_H

#include <cnoid/SubSimulatorItem>

namespace cnoid {

class OperationReviewCollectorItemImpl;

class OperationReviewCollectorItem : public SubSimulatorItem
{
public:
    OperationReviewCollectorItem();
    OperationReviewCollectorItem(const OperationReviewCollectorItem& org);
    ~OperationReviewCollectorItem();

    static void initializeClass(ExtensionManager* ext);

    virtual bool initializeSimulation(SimulatorItem* simulatorItem) override;

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    OperationReviewCollectorItemImpl* impl;
    friend class OperationReviewCollectorItemImpl;
};

typedef ref_ptr<OperationReviewCollectorItem> OperationReviewCollectorItemPtr;

}

#endif // CNOID_OPERATION_REVIEW_PLUGIN_OPERATION_REVIEW_COLLECTOR_ITEM_H
