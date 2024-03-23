/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_CROSSSECTION_ITEM_H
#define CNOID_PHITS_PLUGIN_CROSSSECTION_ITEM_H

#include <cnoid/Item>
#include <cnoid/RenderableItem>
#include <cnoid/Signal>
#include "GammaData.h"
#include "OrthoNodeData.h"

namespace cnoid {

class CrossSectionItem : public Item, public RenderableItem
{
public:
    CrossSectionItem();
    CrossSectionItem(const CrossSectionItem& org);
    virtual ~CrossSectionItem();

    static void initializeClass(ExtensionManager* ext);
    virtual SgNode* getScene() override;

    GammaData& gammaData() const;
    OrthoNodeData* nodeData() const;

    SignalProxy<void()> sigGammaDataLoaded() const;

protected:
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<CrossSectionItem> CrossSectionItemPtr;

}

#endif
