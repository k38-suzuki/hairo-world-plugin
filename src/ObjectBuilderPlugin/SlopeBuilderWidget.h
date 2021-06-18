/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECTBUILDERPLUGIN_SLOPEBUILDERWIDGET_H
#define CNOID_OBJECTBUILDERPLUGIN_SLOPEBUILDERWIDGET_H

#include <cnoid/Widget>

namespace cnoid {

class SlopeBuilderWidgetImpl;

class SlopeBuilderWidget : public Widget
{
public:
    SlopeBuilderWidget();
    virtual ~SlopeBuilderWidget();

    void save(const std::string& filename);

private:
    SlopeBuilderWidgetImpl* impl;
    friend class SlopeBuilderWidgetImpl;
};

}

#endif // CNOID_OBJECTBUILDERPLUGIN_SLOPEBUILDERWIDGET_H
