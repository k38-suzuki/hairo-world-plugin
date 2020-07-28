/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_CRAWLER_ROBOT_BUILDER_PLUGIN_CRAWLER_ROBOT_BUILDER_DIALOG_H
#define CNOID_CRAWLER_ROBOT_BUILDER_PLUGIN_CRAWLER_ROBOT_BUILDER_DIALOG_H

#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>

namespace cnoid {

class CrawlerRobotBuilderDialogImpl;

class CrawlerRobotBuilderDialog : public Dialog
{
public:
    CrawlerRobotBuilderDialog();
    virtual ~CrawlerRobotBuilderDialog();

    static void initializeClass(ExtensionManager* ext);

protected:
    virtual void onAccepted() override;
    virtual void onRejected() override;

private:
    CrawlerRobotBuilderDialogImpl* impl;
    friend class CrawlerRobotBuilderDialogImpl;
};

}

#endif // CNOID_CRAWLER_ROBOT_BUILDER_PLUGIN_CRAWLER_ROBOT_BUILDER_DIALOG_H
