/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEVIEW_H
#define CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEVIEW_H

#include <cnoid/View>

namespace cnoid {

class SceneImageViewImpl;

class SceneImageView : public View
{
public:
    SceneImageView();
    virtual ~SceneImageView();

    static void initializeClass(ExtensionManager* ext);

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    SceneImageViewImpl* impl;
    friend class SceneImageViewImpl;
};

}

#endif // CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEVIEW_H
