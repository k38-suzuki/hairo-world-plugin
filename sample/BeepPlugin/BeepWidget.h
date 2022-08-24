/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BEEPPLUGIN_BEEPWIDGET_H
#define CNOID_BEEPPLUGIN_BEEPWIDGET_H

#include <cnoid/Archive>
#include <cnoid/TreeWidget>
#include <cnoid/Widget>
#include "exportdecl.h"

namespace cnoid {

class BeepWidgetImpl;

class CNOID_EXPORT BeepWidget : public Widget
{
public:
    BeepWidget();
    virtual ~BeepWidget();

    TreeWidget* treeWidget();
    void play(const int& index);

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

private:
    BeepWidgetImpl* impl;
    friend class BeepWidgetImpl;
};

}

#endif // CNOID_BEEPPLUGIN_BEEPWIDGET_H
