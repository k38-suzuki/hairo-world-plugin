/**
   @author Kenta Suzuki
*/

#include "SimpleColliderItem.h"
#include <cnoid/ExtensionManager>
#include <cnoid/BodyItem>
#include <cnoid/WorldItem>
#include <cnoid/SceneGraph>
#include <cnoid/SceneDrawables>
#include <cnoid/Selection>
#include <cnoid/ItemManager>
#include <cnoid/MeshGenerator>
#include <cnoid/EigenUtil>
#include <cnoid/PutPropertyFunction>
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include <cnoid/YAMLReader>
#include <cnoid/YAMLWriter>
#include <cnoid/ConnectionSet>
#include <cnoid/MathUtil>
#include <fmt/format.h>
#include "gettext.h"

using namespace std;
using namespace fmt;
using namespace cnoid;

namespace {

Signal<void()> sigItemsInProjectChanged_;

void notSupportText()
{
    // mvout()
    //     << format(_("This combination is not supported."))
    //     << endl;    
}

class ColliderLocation : public LocationProxy
{
public:
    SimpleColliderItem::Impl* impl;
    Signal<void()> sigLocationChanged_;

    ColliderLocation(SimpleColliderItem::Impl* impl);
    virtual Item* getCorrespondingItem() override;
    virtual Isometry3 getLocation() const override;
    virtual bool setLocation(const Isometry3& T) override;
    virtual SignalProxy<void()> sigLocationChanged() override;
};

}

namespace cnoid {

class SimpleColliderItem::Impl
{
public:
    SimpleColliderItem* self;

    Impl(SimpleColliderItem* self);
    Impl(SimpleColliderItem* self, const Impl& org);

    void createScene();
    void updateScenePosition();
    void updateSceneShape();
    void updateSceneMaterial();

    bool loadSimpleCollider(const string& filename, ostream& os);
    bool saveSimpleCollider(const string& filename, ostream& os);

    BodyItem* bodyItem;
    WorldItem* worldItem;
    Isometry3 position_;
    SgPosTransformPtr scene;
    Selection sceneTypeSelection;
    SgShapePtr sceneShape;
    SgMaterialPtr sceneMaterial;
    Vector3 size_;
    double radius_;
    double height_;
    Vector3 diffuseColor_;
    double specularExponent_;
    double transparency_;
    ScopedConnectionSet connections;
    ref_ptr<ColliderLocation> colliderLocation;
    MappingPtr info;
};

}


void SimpleColliderItem::initializeClass(ExtensionManager* ext)
{
    ext->itemManager()
        .registerClass<SimpleColliderItem>(N_("SimpleColliderItem"))
        .addCreationPanel<SimpleColliderItem>()

        .addLoaderAndSaver<SimpleColliderItem>(
            _("Simple Collider"), "SIMPLE-COLLIDER", "scol",
            [](SimpleColliderItem* item, const string& filename, ostream& os, Item*) {
                return item->impl->loadSimpleCollider(filename, os);
            },
            [](SimpleColliderItem* item, const string& filename, ostream& os, Item*) {
                return item->impl->saveSimpleCollider(filename, os);
            });
}


SimpleColliderItem::SimpleColliderItem()
{
    impl = new Impl(this);
}


SimpleColliderItem::Impl::Impl(SimpleColliderItem* self)
    : self(self)
{
    bodyItem = nullptr;
    worldItem = nullptr;
    position_.setIdentity();
    sceneTypeSelection.setSymbol(BOX, N_("Box"));
    sceneTypeSelection.setSymbol(CYLINDER, N_("Cylinder"));
    sceneTypeSelection.setSymbol(SPHERE, N_("Sphere"));
    sceneTypeSelection.select(BOX);
    size_ << 1.0, 1.0, 1.0;
    radius_ = 0.5;
    height_ = 1.0;
    diffuseColor_ << 0.5, 0.5, 0.5;
    specularExponent_ = 25.0f;
    transparency_ = 0.5;
    info = new Mapping;
}


SimpleColliderItem::SimpleColliderItem(const SimpleColliderItem& org)
    : Item(org)
{
    impl = new Impl(this, *org.impl);
}


SimpleColliderItem::Impl::Impl(SimpleColliderItem* self, const Impl& org)
    : self(self)
{
    bodyItem = nullptr;
    worldItem = nullptr;
    position_ = org.position_;
    sceneTypeSelection = org.sceneTypeSelection;
    size_ = org.size_;
    radius_ = org.radius_;
    height_ = org.height_;
    diffuseColor_ = org.diffuseColor_;
    specularExponent_ = org.specularExponent_;
    transparency_ = org.transparency_;
    info = org.info;
}


SimpleColliderItem::~SimpleColliderItem()
{
    delete impl;
}


void SimpleColliderItem::storeBodyPosition()
{
    if(impl->bodyItem) {
        impl->position_ = impl->bodyItem->body()->rootLink()->position();
        impl->updateScenePosition();
        notifyUpdate();
        // mvout()
        //     << format(_("The current position of {0} has been stored to {1}."),
        //               impl->bodyItem->name(), name())
        //     << endl;
    }
}


void SimpleColliderItem::restoreBodyPosition()
{
    if(impl->bodyItem) {
        impl->bodyItem->body()->rootLink()->position() = impl->position_;
        impl->bodyItem->notifyKinematicStateChange(true);
        mvout()
            << format(_("The position of {0} has been restored from {1}."),
                      impl->bodyItem->name(), name())
            << endl;
    }
}


SgNode* SimpleColliderItem::getScene()
{
    if(!impl->scene) {
        impl->createScene();
    }
    return impl->scene;
}


void SimpleColliderItem::setPosition(const Isometry3& T)
{
    impl->position_ = T;
    impl->updateScenePosition();
    notifyUpdate();
    if(impl->colliderLocation) {
        impl->colliderLocation->sigLocationChanged_();
    }
}


const Isometry3& SimpleColliderItem::position() const
{
    return impl->position_;
}


bool SimpleColliderItem::setSceneType(int sceneId)
{
    if(!impl->sceneTypeSelection.select(sceneId)) {
        return false;
    }
    impl->updateSceneShape();
    notifyUpdate();
    return true;
}


double SimpleColliderItem::sceneType() const
{
    return impl->sceneTypeSelection.which();
}


void SimpleColliderItem::setSize(const Vector3& size)
{
    impl->size_ = size;
    impl->updateSceneShape();
}


const Vector3& SimpleColliderItem::size() const
{
    return impl->size_;
}


void SimpleColliderItem::setRadius(const double& radius)
{
    impl->radius_ = radius;
    impl->updateSceneShape();
}


const double& SimpleColliderItem::radius() const
{
    return impl->radius_;
}


void SimpleColliderItem::setHeight(const double& height)
{
    impl->height_ = height;
    impl->updateSceneShape();
}


const double& SimpleColliderItem::height() const
{
    return impl->height_;
}


void SimpleColliderItem::setDiffuseColor(const Vector3& diffuseColor)
{
    impl->diffuseColor_ = diffuseColor;
    impl->updateSceneMaterial();
}


void SimpleColliderItem::setTransparency(const double& transparency)
{
    impl->transparency_ = transparency;
    impl->updateSceneMaterial();
}


void SimpleColliderItem::notifyUpdate()
{
    Item::notifyUpdate();
    suggestFileUpdate();
}


SignalProxy<void()> SimpleColliderItem::sigItemsInProjectChanged()
{
    return sigItemsInProjectChanged_;
}


LocationProxyPtr SimpleColliderItem::getLocationProxy()
{
    if(!impl->colliderLocation) {
        impl->colliderLocation = new ColliderLocation(impl);
    }
    return impl->colliderLocation;
}


const Mapping* SimpleColliderItem::info() const
{
    return impl->info;
}


Mapping* SimpleColliderItem::info()
{
    return impl->info;
}


template<> double SimpleColliderItem::info(const std::string& key) const
{
    return impl->info->get(key).toDouble();
}


template<> double SimpleColliderItem::info(const std::string& key, const double& defaultValue) const
{
    double value;
    if(impl->info->read(key, value)) {
        return value;
    }
    return defaultValue;
}


template<> bool SimpleColliderItem::info(const std::string& key, const bool& defaultValue) const
{
    bool value;
    if(impl->info->read(key, value)) {
        return value;
    }
    return defaultValue;
}


template<> void SimpleColliderItem::setInfo(const std::string& key, const double& value)
{
    impl->info->write(key, value);
}


template<> void SimpleColliderItem::setInfo(const std::string& key, const bool& value)
{
    impl->info->write(key, value);
}


void SimpleColliderItem::resetInfo(Mapping* info)
{
    impl->info = info;
}


void SimpleColliderItem::Impl::createScene()
{
    if(!scene) {
        scene = new SgPosTransform;
        updateScenePosition();
        sceneShape = new SgShape;
        updateSceneShape();
        sceneMaterial = new SgMaterial;
        updateSceneMaterial();
    } else {
        scene->clearChildren();
    }

    sceneShape->setMaterial(sceneMaterial);
    auto scenePos = new SgPosTransform;
    scenePos->setTranslation(Vector3::Zero());
    scenePos->setRotation(position_.linear());
    scenePos->addChild(sceneShape);
    scene->addChild(scenePos);
}


void SimpleColliderItem::Impl::updateScenePosition()
{
    if(scene) {
        auto p = position_.translation();
        scene->setTranslation(p);
        scene->setRotation(position_.linear());
        scene->notifyUpdate();
    }
}


void SimpleColliderItem::Impl::updateSceneShape()
{
    if(sceneShape) {
        MeshGenerator meshGenerator;

        switch(sceneTypeSelection.which()) {
        case BOX:
            sceneShape->setMesh(meshGenerator.generateBox(size_));
            break;
        case CYLINDER:
            sceneShape->setMesh(meshGenerator.generateCylinder(radius_, height_));
            break;
        case SPHERE:
            sceneShape->setMesh(meshGenerator.generateSphere(radius_));
            break;
        default:
            break;
        }
        sceneShape->notifyUpdate();
    }
}


void SimpleColliderItem::Impl::updateSceneMaterial()
{
    if(sceneMaterial) {
        sceneMaterial->setDiffuseColor(diffuseColor_);
        sceneMaterial->setSpecularExponent(specularExponent_);
        sceneMaterial->setTransparency(transparency_);
        sceneMaterial->notifyUpdate();
    }
}


bool SimpleColliderItem::Impl::loadSimpleCollider(const string& filename, ostream& os)
{
    YAMLReader reader;
    MappingPtr archive;
    try {
        archive = reader.loadDocument(filename)->toMapping();
    }
    catch(const ValueNode::Exception& ex) {
        os << ex.message() << endl;
    }
    Vector3 v;
    if(read(archive, "translation", v)) {
        position_.translation() = v;
    }
    if(read(archive, "rotation", v)) {
        position_.linear() = rotFromRpy(radian(v));
    }
    if(read(archive, "size", v)) {
        size_ = v;
    }
    if(read(archive, "diffuse_color", v)) {
        diffuseColor_ = v;
    }
    archive->read("radius", radius_);
    archive->read("height", height_);
    archive->read("specular_exponent", specularExponent_);
    archive->read("transparency", transparency_);
    string type;
    if(archive->read("scene_type", type)) {
        sceneTypeSelection.select(type);
    }
    return true;
}


bool SimpleColliderItem::Impl::saveSimpleCollider(const string& filename, ostream& os)
{
    YAMLWriter writer;
    if(!writer.openFile(filename)) {
        os << format(_("Failed to open \"{0}\"."), filename) << endl;
        return false;
    }

    MappingPtr archive = new Mapping;
    write(archive, "translation", Vector3(position_.translation()));
    write(archive, "rotation", degree(rpyFromRot(position_.linear())));
    write(archive, "size", Vector3(size_));
    write(archive, "diffuse_color", Vector3(diffuseColor_));
    archive->write("radius", radius_);
    archive->write("height", height_);
    archive->write("specular_exponent", specularExponent_);
    archive->write("transparency", transparency_);
    archive->write("scene_type", sceneTypeSelection.selectedSymbol());
    writer.putNode(archive);

    return true;
}


Item* SimpleColliderItem::doCloneItem(CloneMap* cloneMap) const
{
    return new SimpleColliderItem(*this);
}


void SimpleColliderItem::onTreePathChanged()
{
    impl->connections.disconnect();

    auto newBodyItem = findOwnerItem<BodyItem>();
    if(newBodyItem && newBodyItem != impl->bodyItem) {
        impl->bodyItem = newBodyItem;
        storeBodyPosition();
        impl->connections.add(impl->bodyItem->sigKinematicStateChanged().connect(
            [&]() { storeBodyPosition(); }));
        mvout()
            << format(_("SimpleColliderItem \"{0}\" has been attached to {1}."),
                      name(), impl->bodyItem->name())
            << endl;
    }

    auto newWorldItem = findOwnerItem<WorldItem>();
    if(newWorldItem && newWorldItem != impl->worldItem) {
        impl->worldItem = newWorldItem;
        mvout()
            << format(_("SimpleColliderItem \"{0}\" has been attached to {1}."),
                      name(), impl->worldItem->name())
            << endl;
    }
}


void SimpleColliderItem::doPutProperties(PutPropertyFunction& putProperty)
{
    auto p = impl->position_.translation();
    putProperty(_("translation"), format("{0:.3g} {1:.3g} {2:.3g}", p.x(), p.y(), p.z()),
                [this](const string& text) {
                    Vector3 p;
                    if(toVector3(text, p)) {
                        impl->position_.translation() = p;
                        setPosition(impl->position_);
                        return true;
                    }
                    return false;
                });

    auto r = degree(rpyFromRot(impl->position_.linear()));
    putProperty(_("rotation"), format("{0:.0f} {1:.0f} {2:.0f}", r.x(), r.y(), r.z()),
                [this](const string& text) {
                    Vector3 rpy;
                    if(toVector3(text, rpy)) {
                        impl->position_.linear() = rotFromRpy(radian(rpy));
                        setPosition(impl->position_);
                        return true;
                    }
                    return false;
                });

    putProperty(_("collider type"), impl->sceneTypeSelection,
                [this](int which) { return setSceneType(which); });

    auto s = impl->size_;
    auto sceneId = impl->sceneTypeSelection.which();
    switch(sceneId) {
    case BOX:
        putProperty(_("size"), format("{0:.3g} {1:.3g} {2:.3g}", s.x(), s.y(), s.z()),
                    [this](const string& text) {
                        Vector3 s;
                        if(toVector3(text, s)) {
                            setSize(s);
                            return true;
                        }
                        return false;
                    });
        break;
    case CYLINDER:
    case SPHERE:
        putProperty.min(0.0).max(999.)(_("radius"), impl->radius_,
                    [this](double value) {
                        setRadius(value);
                        return true;
                    });
        if(sceneId == CYLINDER) {
            putProperty.min(0.0).max(999.0)(_("height"), impl->height_,
                        [this](double value) {
                            setHeight(value);
                            return true;
                        });
        }
        break;
    default:
        break;
    }

    auto c = impl->diffuseColor_;
    putProperty(_("diffuseColor"), format("{0:.3g} {1:.3g} {2:.3g}", c.x(), c.y(), c.z()),
                [this](const string& text) {
                    Vector3 c;
                    if(toVector3(text, c)) {
                        setDiffuseColor(c);
                        return true;
                    }
                    return false;
                });

    // putProperty.min(0.0).max(100.0)(_("specularExponent"), specularExponent_,
    //             [this](double value) {
    //                 specularExponent_ = value;
    //                 updateSceneMaterial();
    //                 return true;
    //             });

    putProperty.min(0.0).max(1.0)(_("transparency"), impl->transparency_,
                [this](double value) {
                    setTransparency(value);
                    return true;
                });
}


bool SimpleColliderItem::store(Archive& archive)
{
    bool stored = false;
    if(overwriteOrSaveWithDialog()) {
         stored = archive.writeFileInformation(this);
    }
    return stored;
}


bool SimpleColliderItem::restore(const Archive& archive)
{
    return archive.loadFileTo(this);
}


void SimpleColliderItem::onConnectedToRoot()
{
    sigItemsInProjectChanged_();
}

void SimpleColliderItem::onDisconnectedFromRoot()
{
    sigItemsInProjectChanged_();
}


ColliderLocation::ColliderLocation(SimpleColliderItem::Impl* impl)
    : LocationProxy(GlobalLocation),
      impl(impl)
{

}


Item* ColliderLocation::getCorrespondingItem()
{
    return impl->self;
}


Isometry3 ColliderLocation::getLocation() const
{
    return impl->position_;
}


bool ColliderLocation::setLocation(const Isometry3& T)
{
    impl->self->setPosition(T);
    impl->self->notifyUpdate();
    return true;
}


SignalProxy<void()> ColliderLocation::sigLocationChanged()
{
    return sigLocationChanged_;
}


namespace cnoid {

bool collision(SimpleColliderItem* colliderItem, const Vector3& point)
{
    auto p = colliderItem->position().translation();
    auto R = colliderItem->position().linear();
    auto size = colliderItem->size();
    auto radius = colliderItem->radius();
    auto height = colliderItem->height();

    // box
    Vector3 min = p - size / 2.0;
    Vector3 max = p + size / 2.0;
    Vector3 p2 = R.inverse() * (point - p) + p;

    // cylinder
    Vector3 a = R * (Vector3::UnitY() * height / 2.0) + p;
    Vector3 b = R * (Vector3::UnitY() * height / 2.0 * -1.0) + p;
    Vector3 c = a - b;
    Vector3 d = point - b;
    double c_dot_d = c.dot(d);

    // sphere
    Vector3 r = p - point;

    int sceneId = colliderItem->sceneType();
    switch(sceneId) {
    case SimpleColliderItem::BOX:
        if((min[0] <= p2[0]) && (p2[0] <= max[0])
                && (min[1] <= p2[1]) && (p2[1] <= max[1])
                && (min[2] <= p2[2]) && (p2[2] <= max[2])) {
            return true;
        }
        break;
    case SimpleColliderItem::CYLINDER:
        if((0.0 < c_dot_d) && (c_dot_d < c.dot(c))) {
            double l2 = d.dot(d) - d.dot(c) * d.dot(c) / c.dot(c);
            double r2 = radius * radius;
            if(l2 < r2) {
                return true;
            }
        }
        break;
    case SimpleColliderItem::SPHERE:
        if(r.norm() <= radius) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}


bool collision(SimpleColliderItem* colliderItem1, SimpleColliderItem* colliderItem2)
{
    auto p1 = colliderItem1->position().translation();
    auto p2 = colliderItem2->position().translation();
    auto R1 = colliderItem1->position().linear();
    auto R2 = colliderItem2->position().linear();
    auto size1 = colliderItem1->size();
    auto size2 = colliderItem2->size();
    auto radius1 = colliderItem1->radius();
    auto radius2 = colliderItem2->radius();
    auto height1 = colliderItem1->height();
    auto height2 = colliderItem2->height();

    // box1 x box2

    // box1 x cylinder2

    // box1 x sphere2

    // cylinder1 x box2

    // cylinder1 x cylinder2

    // cylinder1 x sphere2

    // sphere1 x box2

    // sphere1 x cylinder2

    // sphere1 x sphere2

    int sceneId1 = colliderItem1->sceneType();
    int sceneId2 = colliderItem2->sceneType();

    switch(sceneId1) {
    case SimpleColliderItem::BOX:
        if(sceneId2 == SimpleColliderItem::BOX) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::CYLINDER) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::SPHERE) {
            notSupportText();
        }
        break;
    case SimpleColliderItem::CYLINDER:
        if(sceneId2 == SimpleColliderItem::BOX) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::CYLINDER) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::SPHERE) {
            notSupportText();
        }
        break;
    case SimpleColliderItem::SPHERE:
        if(sceneId2 == SimpleColliderItem::BOX) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::CYLINDER) {
            notSupportText();
        } else if(sceneId2 == SimpleColliderItem::SPHERE) {
            if((p1 - p2).norm() <= (radius1 + radius2)) {
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

}
