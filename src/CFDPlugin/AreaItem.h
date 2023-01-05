/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_AREA_ITEM_H
#define CNOID_CFD_PLUGIN_AREA_ITEM_H

#include <cnoid/Item>
#include <cnoid/RenderableItem>
#include <cnoid/SceneGraph>
#include "exportdecl.h"

namespace cnoid {

class AreaItemImpl;

class CNOID_EXPORT AreaItem : public Item, public RenderableItem
{
public:
    AreaItem();
    AreaItem(const AreaItem& org);
    virtual ~AreaItem();

    static void initializeClass(ExtensionManager* ext);
    virtual SgNode* getScene() override;

    void setDiffuseColor(const Vector3& diffuseColor);
    bool isCollided(const Vector3& position);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    AreaItemImpl* impl;
    friend class AreaItemImpl;
};

typedef ref_ptr<AreaItem> AreaItemPtr;

}

#endif // CNOID_CFD_PLUGIN_AREA_ITEM_H