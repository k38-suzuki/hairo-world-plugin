/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUIDDYNAMICSPLUGIN_AREAITEM_H
#define CNOID_FLUIDDYNAMICSPLUGIN_AREAITEM_H

#include <cnoid/Item>
#include <cnoid/Link>
#include <cnoid/SceneGraph>
#include <cnoid/SceneProvider>
#include "exportdecl.h"

namespace cnoid {

class AreaItemImpl;

class CNOID_EXPORT AreaItem : public Item, public SceneProvider
{
public:
    AreaItem();
    AreaItem(const AreaItem& org);
    virtual ~AreaItem();

    virtual SgNode* getScene() override;
    static void initializeClass(ExtensionManager* ext);

    void setTranslation(const Vector3& translation);
    Vector3 translation() const;
    void setRotation(const Vector3& rotation);
    Vector3 rotation() const;
    void setType(const int& type);
    int type() const;
    void setAxes(const int& axes);
    int axes() const;
    void setSize(const Vector3& size);
    Vector3 size() const;
    void setRadius(const double& radius);
    double radius() const;
    void setHeight(const double& height);
    double height() const;
    void setDiffuseColor(const Vector3& diffuseColor);
    Vector3 diffuseColor() const;
    void setEmissiveColor(const Vector3& emissiveColor);
    Vector3 emissiveColor() const;
    void setSpecularColor(const Vector3& specularColor);
    Vector3 specularColor() const;
    void setShininess(const double& shininess);
    double shininess() const;
    void setTransparency(const double& transparency);
    double transparency() const;
    void updateScene();

    bool isCollided(const Link* link);

    enum AreaType { BOX, CYLINDER, SPHERE, NUM_AREA };

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

#endif // CNOID_FLUIDDYNAMICSPLUGIN_AREAITEM_H
