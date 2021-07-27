/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FILEBOXPLUGIN_FILEBOXVIEW_H
#define CNOID_FILEBOXPLUGIN_FILEBOXVIEW_H

#include <cnoid/View>

namespace cnoid {

class FileBoxViewImpl;

class FileBoxView : public View
{
public:
    FileBoxView();
    virtual ~FileBoxView();

    static void initializeClass(ExtensionManager* ext);
    static FileBoxView* instance();

protected:
    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;
    virtual void onActivated() override;
    virtual void onDeactivated() override;

private:
    FileBoxViewImpl* impl;
    friend class FileBoxViewImpl;
};

}

#endif // CNOID_FILEBOXPLUGIN_FILEBOXVIEW_H
