/**
    @author Kenta Suzuki
*/

#ifndef GAIN_TEST_PLUGIN_GAIN_TEST_VIEW_H
#define GAIN_TEST_PLUGIN_GAIN_TEST_VIEW_H

#include <cnoid/View>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT GainTestView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);
    static GainTestView* instance();

    double p(int index) const;
    double d(int index) const;

    void setP(int index, const int value);
    void setD(int index, const int value);

    GainTestView();
    virtual ~GainTestView();

protected:
    virtual void onActivated() override;
    virtual void onDeactivated() override;
    virtual void onAttachedMenuRequest(cnoid::MenuManager& menuManager) override;
    virtual bool storeState(cnoid::Archive& archive) override;
    virtual bool restoreState(const cnoid::Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // GAIN_TEST_PLUGIN_GAIN_TEST_VIEW_H