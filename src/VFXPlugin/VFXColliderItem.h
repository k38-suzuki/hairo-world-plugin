/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_VFX_COLLIDER_ITEM_H
#define CNOID_VFX_PLUGIN_VFX_COLLIDER_ITEM_H

#include <cnoid/Selection>
#include <cnoid/SimpleColliderItem>
#include "VisualEffects.h"

namespace cnoid {

class VFXColliderItem : public SimpleColliderItem, public VisualEffects
{
public:
    static void initializeClass(ExtensionManager* ext);

    VFXColliderItem();
    VFXColliderItem(const VFXColliderItem& org);

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    Selection filterTypeSelection_;
};

typedef ref_ptr<VFXColliderItem> VFXColliderItemPtr;

}

#endif // CNOID_VFX_PLUGIN_VFX_COLLIDER_ITEM_H
