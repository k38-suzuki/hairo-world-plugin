/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VFAREAITEM_H
#define CNOID_VISUALEFFECTPLUGIN_VFAREAITEM_H

#include <src/FluidDynamicsPlugin/AreaItem.h>
namespace cnoid {

class VFAreaItemImpl;

class VFAreaItem : public AreaItem
{
public:
    VFAreaItem();
    VFAreaItem(const VFAreaItem& org);
    virtual ~VFAreaItem();

    static void initializeClass(ExtensionManager* ext);

    double hue() const;
    double saturation() const;
    double value() const;
    double red() const;
    double green() const;
    double blue() const;
    double coefB() const;
    double coefD() const;
    double stdDev() const;
    double salt() const;
    double pepper() const;
    bool flip() const;
    int filter() const;

    static bool load(VFAreaItem* item, const std::string& filename);
    static bool save(VFAreaItem* item, const std::string& filename);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    VFAreaItemImpl* impl;
    friend class VFAreaItemImpl;
};

}

#endif // CNOID_VISUALEFFECTPLUGIN_VFAREAITEM_H
