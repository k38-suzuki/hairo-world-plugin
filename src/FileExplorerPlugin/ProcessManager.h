/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FILEEXPLORERPLUGIN_PROCESSMANAGER_H
#define CNOID_FILEEXPLORERPLUGIN_PROCESSMANAGER_H

#include <cnoid/ExtensionManager>
#include <cnoid/Item>

namespace cnoid {

class ProcessManagerImpl;

class ProcessManager
{
public:
    ProcessManager();
    virtual ~ProcessManager();

    static void initialize(ExtensionManager* ext);

    enum ProgramId { NAUTILUS, GEDIT, NUM_PROGRAMS };

    void execute(const Item* item, const int& id);
    void execute(const int argc, const char* argv[]);
    void finalize();

private:
    ProcessManagerImpl* impl;
    friend class ProcessManagerImpl;
};

}

#endif // CNOID_FILEEXPLORERPLUGIN_PROCESSMANAGER_H
