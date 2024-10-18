/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BODY_CREATOR_PLUGIN_CREATOR_TOOL_BAR_H
#define CNOID_BODY_CREATOR_PLUGIN_CREATOR_TOOL_BAR_H

#include <cnoid/Signal>
#include <QPushButton>
#include <QWidget>

namespace cnoid {

class CreatorToolBar : public QWidget
{
public:
    CreatorToolBar(QWidget* parent = nullptr);
    virtual ~CreatorToolBar();

    QPushButton* addButton(const QIcon& icon, const QString& text);

    SignalProxy<void()> sigNewRequested();
    SignalProxy<void(const std::string& filename)> sigSaveRequested();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BODY_CREATOR_PLUGIN_CREATOR_TOOL_BAR_H