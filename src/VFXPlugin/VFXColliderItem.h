/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFXPLUGIN_VFX_COLLIDER_ITEM_H
#define CNOID_VFXPLUGIN_VFX_COLLIDER_ITEM_H

#include <cnoid/Selection>
#include <cnoid/SimpleColliderItem>
#include "CameraEffects.h"

namespace cnoid {

class VFXColliderItem : public SimpleColliderItem, public CameraEffects
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
    Vector3 hsv_;
    Vector3 rgb_;
    double coef_b_;
    double coef_d_;
    double std_dev_;
    double salt_;
    double pepper_;
    bool flipped_;
    Selection filter_;
};

}

#endif
