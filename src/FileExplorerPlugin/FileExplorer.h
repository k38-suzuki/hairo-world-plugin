/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FILEEXPLORERPLUGIN_FILEEXPLORER_H
#define CNOID_FILEEXPLORERPLUGIN_FILEEXPLORER_H

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

#endif // CNOID_FILEEXPLORERPLUGIN_FILEEXPLORER_H