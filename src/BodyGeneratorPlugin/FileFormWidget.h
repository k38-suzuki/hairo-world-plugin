/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_OBJECTBUILDERPLUGIN_OBJECTBUILDERDIALOG_H
#define CNOID_OBJECTBUILDERPLUGIN_OBJECTBUILDERDIALOG_H

#include <cnoid/Signal>
#include <cnoid/Widget>

namespace cnoid {

class FileFormWidgetImpl;

class FileFormWidget : public Widget
{
public:
    FileFormWidget();
    virtual ~FileFormWidget();

    SignalProxy<void(std::string)> sigClicked();

private:
    FileFormWidgetImpl* impl;
    friend class FileFormWidgetImpl;
};

}

#endif // CNOID_OBJECTBUILDERPLUGIN_OBJECTBUILDERDIALOG_H