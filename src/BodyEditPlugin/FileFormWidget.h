/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_FILE_FORM_WIDGET_H
#define CNOID_BODY_EDIT_PLUGIN_FILE_FORM_WIDGET_H

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

#endif // CNOID_BODY_EDIT_PLUGIN_FILE_FORM_WIDGET_H