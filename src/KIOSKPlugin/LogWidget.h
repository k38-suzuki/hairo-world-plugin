/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_KIOSK_PLUGIN_LOG_WIDGET_H
#define CNOID_KIOSK_PLUGIN_LOG_WIDGET_H

#include <cnoid/Archive>
#include <cnoid/Widget>

namespace cnoid {

class LogWidgetImpl;

class LogWidget : public Widget
{
public:
    LogWidget();
    virtual ~LogWidget();

    void addItem(const std::string& filename, const std::string& text);

    void store(Mapping& archive);
    void restore(const Mapping& archive);

private:
    LogWidgetImpl* impl;
    friend class LogWidgetImpl;
};

}

#endif // CNOID_KIOSK_PLUGIN_LOG_WIDGET_H