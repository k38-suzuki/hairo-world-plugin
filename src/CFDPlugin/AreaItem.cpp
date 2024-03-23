/**
   @author Kenta Suzuki
*/

#include "AreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

class RegionLocation : public LocationProxy
{
public:
    RegionLocation(AreaItemImpl* impl);
    AreaItemImpl* impl;

    Signal<void()> sigLocationChanged_;

    virtual Item* getCorrespondingItem() override;
    virtual Isometry3 getLocation() const override;
    virtual bool setLocation(const Isometry3& T) override;
    virtual SignalProxy<void()> sigLocationChanged() override;
};

}

namespace cnoid {

class AreaItemImpl
{
public:
    AreaItem* self;

    AreaItemImpl(AreaItem* self);
    AreaItemImpl(AreaItem* self, const AreaItemImpl& org);

    Isometry3 regionOffset;
    Selection type;
    Vector3 size;
    FloatingNumberString radius;
    FloatingNumberString height;
    Vector3 diffuseColor;
    FloatingNumberString shininess;
    FloatingNumberString transparency;
    SgPosTransformPtr scene;
    SgMaterialPtr material;

    ref_ptr<RegionLocation> regionLocation;

    enum AreaTypeID { BOX, CYLINDER, SPHERE, NUM_AREA };

    void createScene();
    void updateScenePosition();
    void updateSceneMaterial();
    bool isCollided(const Vector3& position);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


AreaItem::AreaItem()
{
    impl = new AreaItemImpl(this);
}


AreaItemImpl::AreaItemImpl(AreaItem* self)
    : self(self)
{
    regionOffset.setIdentity();
    type.setSymbol(BOX, N_("Box"));
    type.setSymbol(CYLINDER, N_("Cylinder"));
    type.setSymbol(SPHERE, N_("Sphere"));
    size << 1.0, 1.0, 1.0;
    radius = 0.5;
    height = 1.0;
    diffuseColor << 0.0, 1.0, 1.0;
    shininess = 0.0;
    transparency = 0.8;
}


AreaItem::AreaItem(const AreaItem& org)
    : Item(org),
      impl(new AreaItemImpl(this, *org.impl))
{

}


AreaItemImpl::AreaItemImpl(AreaItem* self, const AreaItemImpl& org)
    : self(self)
{
    regionOffset = org.regionOffset;
    type = org.type;
    size = org.size;
    radius = org.radius;
    height = org.height;
    diffuseColor = org.diffuseColor;
    shininess = org.shininess;
    transparency = org.transparency;
}


AreaItem::~AreaItem()
{
    delete impl;
}


void AreaItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager().registerClass<AreaItem>(N_("AreaItem"));
}


SgNode* AreaItem::getScene()
{
    if(!impl->scene) {
        impl->createScene();
    }
    return impl->scene;
}


void AreaItem::setRegionOffset(const Isometry3& T)
{
    impl->regionOffset = T;
    impl->updateScenePosition();
    if(impl->regionLocation) {
        impl->regionLocation->sigLocationChanged_();
    }
}


const Isometry3& AreaItem::regionOffset() const
{
    return impl->regionOffset;
}


void AreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


void AreaItemImpl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform(regionOffset);
        updateScenePosition();
        material = new SgMaterial;
        updateSceneMaterial();
    } else {
        scene->clearChildren();
    }

    MeshGenerator generator;

    SgShape* shape = new SgShape;
    if(type.is(BOX)) {
        shape->setMesh(generator.generateBox(size));
    } else if(type.is(CYLINDER)) {
        shape->setMesh(generator.generateCylinder(radius.value(), height.value()));
    } else if(type.is(SPHERE)) {
        shape->setMesh(generator.generateSphere(radius.value()));
    }
    shape->setMaterial(material);
    scene->addChild(shape);
}


void AreaItemImpl::updateScenePosition()
{
    if(scene) {
        scene->setPosition(regionOffset);
        scene->notifyUpdate();
    }
}


void AreaItemImpl::updateSceneMaterial()
{
    if(material) {
        float s = 127.0f * std::max(0.0f, std::min((float)shininess.value(), 1.0f)) + 1.0f;
        material->setDiffuseColor(diffuseColor);
        material->setSpecularExponent(s);
        material->setTransparency(transparency.value());
        material->notifyUpdate();
    }
}


bool AreaItem::isCollided(const Vector3& position)
{
    return impl->isCollided(position);
}


bool AreaItemImpl::isCollided(const Vector3& position)
{
    bool isCollided = false;

    auto p = regionOffset.translation();
    Matrix3 rot = regionOffset.linear();

    int index = type.selectedIndex();
    if(index == BOX) {
        Vector3 min = p - size / 2.0;
        Vector3 max = p + size / 2.0;
        Vector3 rp = position - p;
        Vector3 np = rot.inverse() * rp + p;

        if((min[0] <= np[0]) && (np[0] <= max[0])
                && (min[1] <= np[1]) && (np[1] <= max[1])
                && (min[2] <= np[2]) && (np[2] <= max[2])
                ) {
            isCollided = true;
        }
    } else if(index == CYLINDER) {
        Vector3 a = rot * (Vector3(0.0, 1.0, 0.0) * height.value() / 2.0) + p;
        Vector3 b = rot * (Vector3(0.0, 1.0, 0.0) * height.value() / 2.0 * -1.0) + p;
        Vector3 c = a - b;
        Vector3 d = position - b;
        double cd = c.dot(d);
        if((0.0 < cd) && (cd < c.dot(c))) {
            double r2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
            double rp2 =  radius.value() * radius.value();
            if(r2 < rp2) {
                isCollided = true;
            }
        }
    } else if(index == SPHERE) {
        Vector3 r = p - position;
        if(r.norm() <= radius.value()) {
            isCollided = true;
        }
    }
    return isCollided;
}


LocationProxyPtr AreaItem::getLocationProxy()
{
    if(!impl->regionLocation) {
        impl->regionLocation = new RegionLocation(impl);
    }
    return impl->regionLocation;
}


RegionLocation::RegionLocation(AreaItemImpl* impl)
    : LocationProxy(GlobalLocation),
      impl(impl)
{

}


Item* RegionLocation::getCorrespondingItem()
{
    return impl->self;
}


Isometry3 RegionLocation::getLocation() const
{
    return impl->regionOffset;
}


bool RegionLocation::setLocation(const Isometry3& T)
{
    impl->self->setRegionOffset(T);
    impl->self->notifyUpdate();
    return true;
}


SignalProxy<void()> RegionLocation::sigLocationChanged()
{
    return sigLocationChanged_;
}


Item* AreaItem::doCloneItem(CloneMap* cloneMap) const
{
    return new AreaItem(*this);
}


void AreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void AreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Shape"), type,
                [&](int index){
                    type.selectIndex(index);
                    createScene();
                    scene->notifyUpdate();
                    return true;
                });
    if(type.is(BOX)) {
        putProperty(_("Size"), str(size),
                [&](const string& str){
                    if(toVector3(str, size)) {
                        createScene();
                        scene->notifyUpdate();
                        return true;
                    }
                    return false;
                });
    } else if(type.is(CYLINDER) || type.is(SPHERE)) {
        putProperty(_("Radius"), to_string(radius.value()),
                [&](const string& str){
                    double radius = stod(str);
                    if(radius >= 0) {
                        this->radius = radius;
                        createScene();
                        scene->notifyUpdate();
                        return true;
                    }
                    return false;
                });
        if(type.is(CYLINDER)) {
            putProperty(_("Height"), to_string(height.value()),
                    [&](const string& str){
                        double height = stod(str);
                        if(height >= 0) {
                            this->height = height;
                            createScene();
                            scene->notifyUpdate();
                            return true;
                        }
                        return false;
                    });
        }
    }
    putProperty(_("Translation"), str(Vector3(regionOffset.translation())),
            [&](const string& str){
                Vector3 p;
                if(toVector3(str, p)) {
                    regionOffset.translation() = p;
                    self->setRegionOffset(regionOffset);
                    return true;
                }
                return false;
            });

    auto rpy = rpyFromRot(regionOffset.linear());
    putProperty(_("RPY"), str(degree(rpy)),
            [&](const string& str){
                Vector3 rpy;
                if(toVector3(str, rpy)) {
                    regionOffset.linear() = rotFromRpy(radian(rpy));
                    self->setRegionOffset(regionOffset);
                    return true;
                }
                return false;
            });
    putProperty(_("DiffuseColor"), str(diffuseColor),
            [&](const string& str){
                if(toVector3(str, diffuseColor)) {
                    updateSceneMaterial();
                    self->notifyUpdate();
                    return true;
                }
                return false;
            });
    putProperty(_("Transparency"), to_string(transparency.value()),
            [&](const string& str){
                double transparency = stod(str);
                if(transparency >= 0) {
                    this->transparency = transparency;
                    updateSceneMaterial();
                    self->notifyUpdate();
                    return true;
                }
                return false;
            });
}


bool AreaItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool AreaItemImpl::store(Archive& archive)
{
    if(!regionOffset.translation().isZero()) {
        write(archive, "translation", regionOffset.translation());
    }
    AngleAxis aa(regionOffset.linear());
    if(aa.angle() != 0.0) {
        writeDegreeAngleAxis(archive, "rotation", aa);
    }
    archive.write("type", type.selectedIndex());
    write(archive, "size", size);
    archive.write("radius", radius);
    archive.write("height", height);
    write(archive, "diffuse_color", diffuseColor);
    archive.write("shininess", shininess);
    archive.write("transparency", transparency);
    return true;
}


bool AreaItem::restore(const Archive &archive)
{
    return impl->restore(archive);
}


bool AreaItemImpl::restore(const Archive& archive)
{
    Vector3 p;
    if(read(archive, "size", size)) {

    }
    bool offsetUpdated = false;
    if(read(archive, "translation", p)) {
        regionOffset.translation() = p;
        offsetUpdated = true;
    }
    AngleAxis aa;
    if(readDegreeAngleAxis(archive, "rotation", aa)) {
        regionOffset.linear() = aa.toRotationMatrix();
        offsetUpdated = true;
    }
    if(offsetUpdated) {
        self->setRegionOffset(regionOffset);
    }
    type.selectIndex(archive.get("type", 0));
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    read(archive, "diffuse_color", diffuseColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    return true;
}
