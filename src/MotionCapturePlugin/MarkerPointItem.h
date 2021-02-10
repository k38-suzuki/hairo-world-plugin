/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_MOTION_CAPTURE_PLUGIN_MARKER_POINT_ITEM_H
#define CNOID_MOTION_CAPTURE_PLUGIN_MARKER_POINT_ITEM_H

#include <cnoid/MultiSE3SeqItem>
#include <cnoid/SceneGraph>
#include <cnoid/SceneProvider>

namespace cnoid {

class MarkerPointItemImpl;

class MarkerPointItem : public MultiSE3SeqItem, public SceneProvider
{
public:
    MarkerPointItem();
    MarkerPointItem(const MarkerPointItem& org);
    virtual ~MarkerPointItem();

    virtual SgNode* getScene() override;
    static void initializeClass(ExtensionManager* ext);

    void addPoint(const Vector3 point, const double radius, const Vector3f color, const double transparency);
    void addLabel(const std::string label);
    std::vector<std::string> labels() const;

    static bool load(MarkerPointItem* item, const std::string fileName);
    static bool save(MarkerPointItem* item, const std::string fileName);

protected:
    virtual Item* doDuplicate() const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    MarkerPointItemImpl* impl;
    friend class MarkerPointItemImpl;
};

typedef ref_ptr<MarkerPointItem> MarkerPointItemPtr;

}

#endif // CNOID_MOTION_CAPTURE_PLUGIN_MARKER_POINT_ITEM_H
