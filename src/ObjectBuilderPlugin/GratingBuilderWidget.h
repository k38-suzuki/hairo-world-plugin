/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECT_BUILDER_PLUGIN_GRATING_BUILDER_WIDGET_H
#define CNOID_OBJECT_BUILDER_PLUGIN_GRATING_BUILDER_WIDGET_H

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

#endif // CNOID_OBJECT_BUILDER_PLUGIN_GRATING_BUILDER_WIDGET_H
