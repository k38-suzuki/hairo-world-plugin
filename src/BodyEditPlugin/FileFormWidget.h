/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODYEDIT_PLUGIN_FILE_FORM_WIDGET_H
#define CNOID_BODYEDIT_PLUGIN_FILE_FORM_WIDGET_H

#include <cnoid/Signal>
#include <cnoid/Widget>

namespace cnoid {

class FileFormWidget : public Widget
{
public:
    FileFormWidget(QWidget* parent = nullptr);
    virtual ~FileFormWidget();

    SignalProxy<void(std::string)> sigClicked();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODYEDIT_PLUGIN_FILE_FORM_WIDGET_H