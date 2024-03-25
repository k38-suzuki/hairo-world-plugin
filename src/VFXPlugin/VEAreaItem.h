/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VFX_PLUGIN_VE_AREA_ITEM_H
#define CNOID_VFX_PLUGIN_VE_AREA_ITEM_H

#include <cnoid/EigenTypes>
#include <cnoid/Selection>
#include <cnoid/AreaItem>

namespace cnoid {

class VEAreaItem : public AreaItem
{
public:
    VEAreaItem();
    VEAreaItem(const VEAreaItem& org);
    ~VEAreaItem();

    static void initializeClass(ExtensionManager* ext);

    void setHsv(const Vector3& hsv) { hsv_ = hsv; }
    Vector3 hsv() const { return hsv_; }
    void setRgb(const Vector3& rgb) { rgb_ = rgb; }
    Vector3 rgb() const { return rgb_; }
    void setCoefB(const double coef_b) { coef_b_ = coef_b; }
    double coefB() const { return coef_b_; }
    void setCoefD(const double& coef_d) { coef_d_ = coef_d; }
    double coefD() const { return coef_d_; }
    void setStdDev(const double& std_dev) { std_dev_ = std_dev; }
    double stdDev() const { return std_dev_; }
    void setSalt(const double& salt) { salt_ = salt; }
    double salt() const { return salt_; }
    void setPepper(const double& pepper) { pepper_ = pepper; }
    double pepper() const { return pepper_; }
    void setFlipped(const bool& flip) { flip_ = flip; }
    bool flipped() const { return flip_; }
    void setFilter(const int& filter) { filter_.selectIndex(filter); }
    int filter() const { return filter_.which(); }

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
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

#endif
