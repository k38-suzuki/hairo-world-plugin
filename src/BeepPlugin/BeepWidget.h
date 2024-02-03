/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEP_WIDGET_H
#define CNOID_BEEP_PLUGIN_BEEP_WIDGET_H

#include <cnoid/Archive>
#include <cnoid/TreeWidget>
#include <cnoid/Widget>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT BeepWidget : public Widget
{
public:
    BeepWidget();
    virtual ~BeepWidget();

    TreeWidget* treeWidget();
    int numItems() const;
    void play(const int& index);

    bool storeState(Archive& archive);
    bool restoreState(const Archive& archive);

private:
    class Impl;
    Impl* impl;
};

}

#endif
