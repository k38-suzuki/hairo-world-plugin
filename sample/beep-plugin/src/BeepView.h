/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEP_VIEW_H
#define CNOID_BEEP_PLUGIN_BEEP_VIEW_H

#include <cnoid/View>
#include "BeepWidget.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT BeepView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BeepView* instance();

    BeepView();
    virtual ~BeepView();

    BeepWidget* beepWidget();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif
