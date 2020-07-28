/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_AREA_ITEM_H
#define CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_AREA_ITEM_H

#include <cnoid/Item>
#include <cnoid/SceneGraph>
#include <cnoid/SceneProvider>

namespace cnoid {

class FluidAreaItemImpl;

class FluidAreaItem : public Item, public SceneProvider
{
public:
    FluidAreaItem();
    FluidAreaItem(const FluidAreaItem& org);
    virtual ~FluidAreaItem();

    virtual SgNode* getScene() override;
    static void initializeClass(ExtensionManager* ext);

    void setTranslation(const Vector3 translation);
    Vector3 translation() const;
    void setRotation(const Vector3 rotation);
    Vector3 rotation() const;
    void setDensity(const double density);
    double density() const;
    void setViscosity(const double viscosity);
    double viscosity() const;
    void setType(const std::string type);
    std::string type() const;
    void setSize(const Vector3 size);
    Vector3 size() const;
    void setRadius(const double radius);
    double radius() const;
    void setHeight(const double height);
    double height() const;
    void setFlow(const Vector3 flow);
    Vector3 flow() const;
    void setDiffuseColor(const Vector3 diffuseColor);
    Vector3 diffuseColor() const;
    void setEmissiveColor(const Vector3 emissiveColor);
    Vector3 emissiveColor() const;
    void setSpecularColor(const Vector3 specularColor);
    Vector3 specularColor() const;
    void setShininess(const double shininess);
    double shininess() const;
    void setTransparency(const double transparency);
    double transparency() const;
    void updateScene();

    static bool load(FluidAreaItem* item, const std::string fileName);
    static bool save(FluidAreaItem* item, const std::string fileName);

    enum PrimitiveType { BOX, CYLINDER, SPHERE };

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    FluidAreaItemImpl* impl;
    friend class FluidAreaItemImpl;
};

typedef ref_ptr<FluidAreaItem> FluidAreaItemPtr;

}

#endif // CNOID_FLUID_DYNAMICS_PLUGIN_FLUID_AREA_ITEM_H
