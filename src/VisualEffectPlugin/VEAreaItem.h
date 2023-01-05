/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VE_AREA_ITEM_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VE_AREA_ITEM_H

#include <cnoid/EigenTypes>
#include <cnoid/Selection>
#include <cnoid/AreaItem>

namespace cnoid {

class VEAreaItem : public AreaItem
{
public:
    VEAreaItem();
    VEAreaItem(const VEAreaItem& org);
    virtual ~VEAreaItem();

    static void initializeClass(ExtensionManager* ext);

    Vector3 hsv() const { return hsv_; }
    Vector3 rgb() const { return rgb_; }
    double coefB() const { return coef_b_; }
    double coefD() const { return coef_d_; }
    double stdDev() const { return std_dev_; }
    double salt() const { return salt_; }
    double pepper() const { return pepper_; }
    bool flip() const { return flip_; }
    int filter() const { return filter_.which(); }

protected:
    virtual Item* doDuplicate() const override;
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
    bool flip_;
    Selection filter_;
};

}

#endif // CNOID_VISUAL_EFFECT_PLUGIN_VE_AREA_ITEM_H