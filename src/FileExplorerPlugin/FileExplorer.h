/**
   @author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLORER_H
#define CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLORER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class FileExplorerImpl;

class FileExplorer
{
public:
    FileExplorer();
    virtual ~FileExplorer();

    static void initializeClass(ExtensionManager* ext);

private:
    FileExplorerImpl* impl;
    friend class FileExplorerImpl;
};

}

#endif
