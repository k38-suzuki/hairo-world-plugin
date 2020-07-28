/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_TC_PLUGIN_TC_AREA_ITEM_H
#define CNOID_TC_PLUGIN_TC_AREA_ITEM_H

#include <cnoid/EigenTypes>
#include <cnoid/Item>
#include <cnoid/SceneProvider>

namespace cnoid {

class TCAreaItemImpl;

class TCAreaItem : public Item, public SceneProvider
{
public:
    TCAreaItem();
    TCAreaItem(const TCAreaItem& org);
    virtual ~TCAreaItem();

    virtual SgNode* getScene() override;
    static void initializeClass(ExtensionManager* ext);

    static bool load(TCAreaItem* item, const std::string fileName);
    static bool save(TCAreaItem* item, const std::string fileName);

    void updateScene();

    void setId(const int id);
    int id() const;
    void setTranslation(const Vector3 translation);
    Vector3 translation() const;
    void setRotation(const Vector3 rotation);
    Vector3 rotation() const;
    void setType(const std::string type);
    std::string type() const;
    void setSize(const Vector3 size);
    Vector3 size() const;
    void setRadius(const double radius);
    double radius() const;
    void setHeight(const double height);
    double height() const;
    void setInboundDelay(const double inboundDelay);
    double inboundDelay() const;
    void setInboundRate(const double inboundRate);
    double inboundRate() const;
    void setInboundLoss(const double inboundLoss);
    double inboundLoss() const;
    void setOutboundDelay(const double outboundDelay);
    double outboundDelay() const;
    void setOutboundRate(const double outboundRate);
    double outboundRate() const;
    void setOutboundLoss(const double outboundLoss);
    double outboundLoss() const;
    void setSource(const std::string source);
    std::string source() const;
    void setDestination(const std::string destination);
    std::string destination() const;
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

    enum PrimitiveType { BOX, CYLINDER, SPHERE };

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    TCAreaItemImpl* impl;
    friend class TCAreaItemImpl;
};

typedef ref_ptr<TCAreaItem> TCAreaItemPtr;

}

#endif // CNOID_TC_PLUGIN_TC_AREA_ITEM_H
