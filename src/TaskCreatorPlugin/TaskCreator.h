/**
   @author Kenta Suzuki
*/

#ifndef CNOID_TASK_CREATOR_PLUGIN_TASK_CREATOR_H
#define CNOID_TASK_CREATOR_PLUGIN_TASK_CREATOR_H

namespace cnoid {

class ExtensionManager;

class TaskCreator
{
public:
    static void initializeClass(ExtensionManager* ext);
    static TaskCreator* instance();

    TaskCreator();
    virtual ~TaskCreator();

private:
    class Impl;
    Impl* impl;
};

}

#endif
