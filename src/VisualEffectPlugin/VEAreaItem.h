/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VEAREAITEM_H
#define CNOID_VISUALEFFECTPLUGIN_VEAREAITEM_H

#include <src/FluidDynamicsPlugin/AreaItem.h>
namespace cnoid {

class VEAreaItemImpl;

class VEAreaItem : public AreaItem
{
public:
    VEAreaItem();
    VEAreaItem(const VEAreaItem& org);
    virtual ~VEAreaItem();

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

    static bool load(VEAreaItem* item, const std::string& filename);
    static bool save(VEAreaItem* item, const std::string& filename);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    VEAreaItemImpl* impl;
    friend class VEAreaItemImpl;
};

}

#endif // CNOID_VISUALEFFECTPLUGIN_VEAREAITEM_H
