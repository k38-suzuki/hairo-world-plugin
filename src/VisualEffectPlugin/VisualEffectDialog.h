/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTDIALOG_H
#define CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTDIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class VisualEffectDialogImpl;

class VisualEffectDialog : public Dialog
{
public:
    VisualEffectDialog();
    virtual ~VisualEffectDialog();

    double hue() const;
    double saturation() const;
    double value() const;
    double red() const;
    double green() const;
    double blue() const;
    double coefB() const;
    double coefD() const;
    double stdDev() const;
    double salt() const;
    double pepper() const;
    bool flip() const;
    int filter() const;

    bool store(Archive& archive);
    bool restore(const Archive& archive);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    VisualEffectDialogImpl* impl;
    friend class VisualEffectDialogImpl;
};

}

#endif // CNOID_VISUALEFFECTPLUGIN_VISUALEFFECTDIALOG_H
