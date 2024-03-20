/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_FILE_FORM_WIDGET_H
#define CNOID_BODY_EDIT_PLUGIN_FILE_FORM_WIDGET_H

#include <cnoid/Signal>
#include <cnoid/Widget>

namespace cnoid {

class FileFormWidget : public Widget
{
public:
    FileFormWidget();
    virtual ~FileFormWidget();

    SignalProxy<void(std::string)> sigClicked();

private:
    class Impl;
    Impl* impl;
};

std::string getSaveFileName(const std::string& caption, const std::string& extensions);
std::vector<std::string> getSaveFileNames(const std::string& caption, const std::string& extensions);

}

#endif
