/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SIMPLECOLLIDER_PLUGIN_MULTI_COLLIDER_ITEM_H
#define CNOID_SIMPLECOLLIDER_PLUGIN_MULTI_COLLIDER_ITEM_H

#include <cnoid/Selection>
#include <cnoid/SimpleColliderItem>
#include "CustomEffects.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT MultiColliderItem : public SimpleColliderItem,
                            public CFDEffects, public TCEffects, public VFXEffects
{
public:
    static void initializeClass(ExtensionManager* ext);

    MultiColliderItem();
    MultiColliderItem(const MultiColliderItem& org);

    enum ColliderId { CFD, TC, VFX };
    bool setColliderType(int colliderId);
    double colliderType() const { return colliderTypeSelection.which(); }


protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    Selection colliderTypeSelection;
};

typedef ref_ptr<MultiColliderItem> MultiColliderItemPtr;

}

#endif // CNOID_SIMPLECOLLIDER_PLUGIN_MULTI_COLLIDER_ITEM_H