/**
   @author Kenta Suzuki
*/

#ifndef CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECTOR_ITEM_H
#define CNOID_VISUAL_EFFECT_PLUGIN_VISUAL_EFFECTOR_ITEM_H

#include <cnoid/Item>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT VisualEffectorItem : public Item
{
public:
    static void initializeClass(ExtensionManager* ext);

    VisualEffectorItem();
    virtual ~VisualEffectorItem();

protected:
    VisualEffectorItem(const VisualEffectorItem& org);
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void onTreePathChanged() override;
    virtual void onDisconnectedFromRoot() override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};
        
typedef ref_ptr<VisualEffectorItem> VisualEffectorItemPtr;

}

#endif
