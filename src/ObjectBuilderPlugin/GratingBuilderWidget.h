/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECTBUILDERPLUGIN_GRATINGBUILDERWIDGET_H
#define CNOID_OBJECTBUILDERPLUGIN_GRATINGBUILDERWIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class GratingBuilderWidgetImpl;

class GratingBuilderWidget : public Widget
{
public:
    GratingBuilderWidget();
    virtual ~GratingBuilderWidget();

    void save(const std::string& filename);

private:
    GratingBuilderWidgetImpl* impl;
    friend class GratingBuilderWidgetImpl;
};

}

#endif // CNOID_OBJECTBUILDERPLUGIN_GRATINGBUILDERWIDGET_H
