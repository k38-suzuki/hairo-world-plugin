/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VEAREAITEM_H
#define CNOID_VISUALEFFECTPLUGIN_VEAREAITEM_H

#include <cnoid/EigenTypes>
#include <src/CFDPlugin/AreaItem.h>

namespace cnoid {

class VEAreaItemImpl;

class VEAreaItem : public AreaItem
{
public:
    VEAreaItem();
    VEAreaItem(const VEAreaItem& org);
    virtual ~VEAreaItem();

    static void initializeClass(ExtensionManager* ext);

    Vector3 hsv() const;
    Vector3 rgb() const;
    double coefB() const;
    double coefD() const;
    double stdDev() const;
    double salt() const;
    double pepper() const;
    bool flip() const;
    int filter() const;

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
