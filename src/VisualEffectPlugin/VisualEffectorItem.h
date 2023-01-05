/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECTOR_ITEM_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECTOR_ITEM_H

#include <cnoid/Item>
#include "exportdecl.h"

namespace cnoid {

class VisualEffectorItemImpl;

class CNOID_EXPORT VisualEffectorItem : public Item
{
public:
    VisualEffectorItem();
    VisualEffectorItem(const VisualEffectorItem& org);
    virtual ~VisualEffectorItem();

    static void initializeClass(ExtensionManager* ext);

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

#endif // CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECTOR_ITEM_H