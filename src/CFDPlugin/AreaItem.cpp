/**
   \file
   \author Kenta Suzuki
*/

#include "AreaItem.h"
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/EigenTypes>
#include <cnoid/EigenUtil>
#include <cnoid/FloatingNumberString>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/PositionDragger>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Selection>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class AreaItemImpl
{
public:
    AreaItemImpl(AreaItem* self);
    AreaItemImpl(AreaItem* self, const AreaItemImpl& org);
    AreaItem* self;

    Isometry3 position;
    Selection type;
    Vector3 size;
    FloatingNumberString radius;
    FloatingNumberString height;
    Vector3 diffuseColor;
    FloatingNumberString shininess;
    FloatingNumberString transparency;
    SgPosTransformPtr scene;
    PositionDraggerPtr positionDragger;
    SgMaterialPtr material;

    enum AreaTypeID { BOX, CYLINDER, SPHERE, NUM_AREA };

    void createScene();
    void updateScenePosition();
    void updateSceneMaterial();
    void onPositionDragged();
    bool onAreaTypePropertyChanged(const int& index);
    bool onAreaAxesPropertyChanged(const int& index);
    bool onAreaRadiusPropertyChanged(const string& str);
    bool onAreaHeightPropertyChanged(const string& str);
    bool onDiffuseColorPropertyChanged(const string& str);
    bool onTransparencyPropertyChanged(const string& str);
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
    position.setIdentity();
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
    position = org.position;
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


void AreaItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor = diffuseColor;
}


bool AreaItemImpl::onAreaTypePropertyChanged(const int& index)
{
    type.selectIndex(index);
    createScene();
    scene->notifyUpdate();
    return true;
}


bool AreaItemImpl::onAreaRadiusPropertyChanged(const string& str)
{
    double radius = stod(str);
    if(radius >= 0) {
        this->radius = radius;
        createScene();
        scene->notifyUpdate();
        return true;
    }
    return false;
}


bool AreaItemImpl::onAreaHeightPropertyChanged(const string& str)
{
    double height = stod(str);
    if(height >= 0) {
        this->height = height;
        createScene();
        scene->notifyUpdate();
        return true;
    }
    return false;
}


bool AreaItemImpl::onDiffuseColorPropertyChanged(const string& str)
{
    if(toVector3(str, diffuseColor)) {
        updateSceneMaterial();
        self->notifyUpdate();
        return true;
    }
    return false;
}


bool AreaItemImpl::onTransparencyPropertyChanged(const string& str)
{
    double transparency = stod(str);
    if(transparency >= 0) {
        this->transparency = transparency;
        updateSceneMaterial();
        self->notifyUpdate();
        return true;
    }
    return false;
}


void AreaItemImpl::onPositionDragged()
{
    auto p = positionDragger->globalDraggingPosition();
    position.translation() = p.translation();
    position.linear() = p.linear();
    updateScenePosition();
    self->notifyUpdate();
}


void AreaItemImpl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform;
        positionDragger = new PositionDragger(
                    PositionDragger::AllAxes, PositionDragger::WideHandle);
        positionDragger->setDragEnabled(true);
        positionDragger->setOverlayMode(true);
        positionDragger->setPixelSize(48, 2);
        positionDragger->setDisplayMode(PositionDragger::DisplayInEditMode);
        positionDragger->sigPositionDragged().connect([&](){ onPositionDragged(); });
        // scene->addChild(positionDragger);
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
    positionDragger->adjustSize(shape->boundingBox());
}


void AreaItemImpl::updateScenePosition()
{
    if(scene) {
        scene->setTranslation(position.translation());
        scene->setRotation(position.linear());
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

    auto p = this->position.translation();
    Matrix3 rot = this->position.linear();

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


Item* AreaItem::doDuplicate() const
{
    return new AreaItem(*this);
}


void AreaItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void AreaItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    Vector3 p = position.translation();
    Vector3 r = degree(rpyFromRot(position.linear()));

    putProperty(_("Shape"), type,
                [&](int index){ return onAreaTypePropertyChanged(index); });
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
                [&](const string& str){ return onAreaRadiusPropertyChanged(str); });
        if(type.is(CYLINDER)) {
            putProperty(_("Height"), to_string(height.value()),
                    [&](const string& str){ return onAreaHeightPropertyChanged(str); });
        }
    }
    putProperty(_("Translation"), str(p),
            [&](const string& str){
                Vector3 p;
                if(toVector3(str, p)) {
                    position.translation() = p;
                    updateScenePosition();
                    return true;
                }
                return false;
            });
    putProperty(_("RPY"), str(r),
            [&](const string& str){
                Vector3 rpy;
                if(toVector3(str, rpy)) {
                    position.linear() = rotFromRpy(radian(rpy));
                    updateScenePosition();
                    return true;
                }
                return false;
            });
    putProperty(_("DiffuseColor"), str(diffuseColor),
            [&](const string& text){ return onDiffuseColorPropertyChanged(text); });
    putProperty(_("Transparency"), to_string(transparency.value()),
            [&](const string& text){ return onTransparencyPropertyChanged(text); });
}


bool AreaItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool AreaItemImpl::store(Archive& archive)
{
    write(archive, "translation", position.translation());
    write(archive, "rpy", degree(rpyFromRot(position.linear())));
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
    if(read(archive, "translation", p)) {
        position.translation() = p;
    }
    Vector3 rpy;
    if(read(archive, "rpy", rpy)) {
        position.linear() = rotFromRpy(radian(rpy));
    }
    type.selectIndex(archive.get("type", 0));
    read(archive, "size", size);
    radius = archive.get("radius", radius.string());
    height = archive.get("height", height.string());
    read(archive, "diffuse_color", diffuseColor);
    shininess = archive.get("shininess", shininess.string());
    transparency = archive.get("transparency", transparency.string());
    return true;
}
