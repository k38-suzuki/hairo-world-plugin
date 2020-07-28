/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_IMAGE_EFFECT_PLUGIN_CAMERA_VISUALIZER_ITEM_H
#define CNOID_IMAGE_EFFECT_PLUGIN_CAMERA_VISUALIZER_ITEM_H

#include <cnoid/Item>
#include "exportdecl.h"

namespace cnoid {

class CameraVisualizerItemImpl;

class CNOID_EXPORT CameraVisualizerItem : public Item
{
public:
    static void initializeClass(ExtensionManager* ext);

    CameraVisualizerItem();
    CameraVisualizerItem(const CameraVisualizerItem& org);
    virtual ~CameraVisualizerItem();

protected:
    virtual Item* doDuplicate() const override;
    virtual void onPositionChanged() override;
    virtual void onDisconnectedFromRoot() override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    CameraVisualizerItemImpl* impl;
};

typedef ref_ptr<CameraVisualizerItem> CameraVisualizerItemPtr;

}

#endif // CNOID_IMAGE_EFFECT_PLUGIN_CAMERA_VISUALIZER_ITEM_H
