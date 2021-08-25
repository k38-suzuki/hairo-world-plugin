/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORERPLUGIN_FILEEXPLORER_H
#define CNOID_FILE_EXPLORERPLUGIN_FILEEXPLORER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class FileExplorerImpl;

class FileExplorer
{
public:
    FileExplorer();
    virtual ~FileExplorer();

    static void initializeClass(ExtensionManager* ext);
    static void finalizeClass();

    enum ToolType { NAUTILUS, GEDIT, NUM_TOOLS };

    void execute(const Item* item, const int& type);
    void finalize();

private:
    FileExplorerImpl* impl;
    friend class FileExplorerImpl;
};

}

#endif // CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLOREROR_H
