/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTORITEM_H
#define CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTORITEM_H

#include <cnoid/BodyItem>
#include <cnoid/ConnectionSet>
#include <cnoid/ImageableItem>
#include <cnoid/Item>
#include <cnoid/RangeCamera>
#include "ImageGenerator.h"
#include "VisualEffector.h"
#include "exportdecl.h"

namespace cnoid {

class VisualEffectorItemImpl;

class VisualEffectorItemBase
{
public:
    Item* visualizerItem;
    BodyItem* bodyItem;
    ScopedConnection sigCheckToggledConnection;
    VisualEffectorItemBase(Item* visualizerItem);
    void setBodyItem(BodyItem* bodyItem);
    void updateVisualization();

    virtual void enableVisualization(bool on) = 0;
    virtual void doUpdateVisualization() = 0;
};


class CNOID_EXPORT VEImageVisualizerItem : public Item, public ImageableItem, public VisualEffectorItemBase
{
public:
    VEImageVisualizerItem();
    virtual const Image* getImage() override;
    virtual SignalProxy<void()> sigImageUpdated() override;
    void setBodyItem(BodyItem* bodyItem, Camera* camera);
    virtual void enableVisualization(bool on) override;
    virtual void doUpdateVisualization() override;

    CameraPtr camera;
    VisualEffector* effector;
    ImageGenerator generator;
    ScopedConnectionSet connections;
    std::shared_ptr<const Image> image;
    Signal<void()> sigImageUpdated_;
};

typedef ref_ptr<VEImageVisualizerItem> VEImageVisualizerItemPtr;


class CNOID_EXPORT VisualEffectorItem : public Item
{
public:
    static void initializeClass(ExtensionManager* ext);

    VisualEffectorItem();
    VisualEffectorItem(const VisualEffectorItem& org);
    virtual ~VisualEffectorItem();

protected:
    virtual Item* doDuplicate() const override;
    virtual void onPositionChanged() override;
    virtual void onDisconnectedFromRoot() override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    VisualEffectorItemImpl* impl;
};

typedef ref_ptr<VisualEffectorItem> VisualEffectorItemPtr;

}

#endif // CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTORITEM_H
