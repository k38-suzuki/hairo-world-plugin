/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEP_VIEW_H
#define CNOID_BEEP_PLUGIN_BEEP_VIEW_H

#include <cnoid/TreeWidget>
#include <cnoid/View>

namespace cnoid {

class BeepView : public View
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BeepView* instance();

    BeepView();
    virtual ~BeepView();

    TreeWidget* treeWidget();
    void play(const int& index);

protected:
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BEEP_PLUGIN_BEEP_VIEW_H