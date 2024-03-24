/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SIMPLE_COLLIDER_PLUGIN_SIMPLE_COLLIDER_ITEM_H
#define CNOID_SIMPLE_COLLIDER_PLUGIN_SIMPLE_COLLIDER_ITEM_H

#include <cnoid/Item>
#include <cnoid/RenderableItem>
#include <cnoid/LocatableItem>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT SimpleColliderItem : public Item, public RenderableItem, public LocatableItem
{
public:
    static void initializeClass(ExtensionManager* ext);

    SimpleColliderItem();
    SimpleColliderItem(const SimpleColliderItem& org);
    ~SimpleColliderItem();

    void storeBodyPosition();
    void restoreBodyPosition();
    virtual SgNode* getScene() override;
    void setPosition(const Isometry3& T);
    const Isometry3& position() const;
    enum SceneId { BOX, CYLINDER, SPHERE };
    bool setSceneType(int sceneId);
    double sceneType() const;
    void setSize(const Vector3& size);
    const Vector3& size() const;
    void setRadius(const double& radius);
    const double& radius() const;
    void setHeight(const double& height);
    const double& height() const;
    void setDiffuseColor(const Vector3& diffuseColor);
    void setTransparency(const double& transparency);

    // LocatableItem function
    virtual LocationProxyPtr getLocationProxy() override;

    class Impl;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void onTreePathChanged() override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    Impl* impl;
};

CNOID_EXPORT bool collision(SimpleColliderItem* colliderItem, const Vector3& point);

}

#endif
