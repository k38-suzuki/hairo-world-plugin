/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECTBUILDERPLUGIN_PIPEBUILDERWIDGET_H
#define CNOID_OBJECTBUILDERPLUGIN_PIPEBUILDERWIDGET_H

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

#endif // CNOID_OBJECTBUILDERPLUGIN_PIPEBUILDERWIDGET_H
