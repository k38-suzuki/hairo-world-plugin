/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECT_BUILDER_PLUGIN_PIPE_BUILDER_WIDGET_H
#define CNOID_OBJECT_BUILDER_PLUGIN_PIPE_BUILDER_WIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class PipeBuilderWidgetImpl;

class PipeBuilderWidget : public Widget
{
public:
    PipeBuilderWidget();
    virtual ~PipeBuilderWidget();

    void save(const std::string& filename);

private:
    PipeBuilderWidgetImpl* impl;
    friend class PipeBuilderWidgetImpl;
};

}

#endif // CNOID_OBJECT_BUILDER_PLUGIN_PIPE_BUILDER_WIDGET_H
