/**
   @author Kenta Suzuki
*/

#ifndef CNOID_SIMPLECOLLIDER_PLUGIN_SIMPLE_COLLIDER_ITEM_H
#define CNOID_SIMPLECOLLIDER_PLUGIN_SIMPLE_COLLIDER_ITEM_H

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

    virtual void notifyUpdate() override;

    static SignalProxy<void()> sigItemsInProjectChanged();

    // LocatableItem function
    virtual LocationProxyPtr getLocationProxy() override;

    const Mapping* info() const;
    Mapping* info();
    template<typename T> T info(const std::string& key) const;
    template<typename T> T info(const std::string& key, const T& defaultValue) const;
    template<typename T> void setInfo(const std::string& key, const T& value);
    void resetInfo(Mapping* info);

    class Impl;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void onTreePathChanged() override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

    virtual void onConnectedToRoot() override;
    virtual void onDisconnectedFromRoot() override;

private:
    Impl* impl;
};

typedef ref_ptr<SimpleColliderItem> SimpleColliderItemPtr;

CNOID_EXPORT bool collision(SimpleColliderItem* colliderItem, const Vector3& point);
CNOID_EXPORT bool collision(SimpleColliderItem* colliderItem1, SimpleColliderItem* colliderItem2);

}

#endif // CNOID_SIMPLECOLLIDER_PLUGIN_SIMPLE_COLLIDER_ITEM_H
