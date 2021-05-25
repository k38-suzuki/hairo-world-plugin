/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VIEWSPACERPLUGIN_EMPTYVIEW_H
#define CNOID_VIEWSPACERPLUGIN_EMPTYVIEW_H

#include <cnoid/View>

namespace cnoid {

class EmptyViewImpl;

class EmptyView : public View
{
public:
    EmptyView();
    virtual ~EmptyView();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onActivated() override;
    virtual void onDeactivated() override;

private:
    EmptyViewImpl* impl;
    friend class EmptyViewImpl;
};

}

#endif // CNOID_VIEWSPACERPLUGIN_EMPTYVIEW_H
