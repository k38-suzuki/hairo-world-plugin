/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEP_COMMAND_ITEM_H
#define CNOID_BEEP_PLUGIN_BEEP_COMMAND_ITEM_H

#include <cnoid/Item>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT BeepCommandItem : public Item
{
public:
    static void initializeClass(ExtensionManager* ext);

    BeepCommandItem();
    BeepCommandItem(const BeepCommandItem& org);
    virtual ~BeepCommandItem();

    void setFrequency(const int& frequency);
    int frequency() const;
    void setLength(const int& length);
    int length() const;
    double waitingTimeAfterStarted() const;
    void setWaitingTimeAfterStarted(double time);
    void showMessage(const bool checked);
    bool execute();
    bool terminate();

protected:
    virtual void onDisconnectedFromRoot() override;
    virtual Item* doCloneItem(CloneMap* cloneMap) const override;
    virtual void doPutProperties(PutPropertyFunction& putProperty) override;
    virtual bool store(Archive& archive) override;
    virtual bool restore(const Archive& archive) override;

private:
    class Impl;
    Impl* impl;
};

typedef ref_ptr<BeepCommandItem> BeepCommandItemPtr;

}

#endif // CNOID_BEEP_PLUGIN_BEEP_COMMAND_ITEM_H