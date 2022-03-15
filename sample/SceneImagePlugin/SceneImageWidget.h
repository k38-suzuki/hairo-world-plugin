/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEWIDGET_H
#define CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEWIDGET_H

#include <cnoid/Archive>
#include <cnoid/Widget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>

namespace cnoid {

class SceneImageWidgetImpl;

class SceneImageWidget : public Widget
{
public:
    SceneImageWidget();
    virtual ~SceneImageWidget();

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    SceneImageWidgetImpl* impl;
    friend class SceneImageWidgetImpl;
};

}

#endif // CNOID_SCENEIMAGEPLUGIN_SCENEIMAGEWIDGET_H
