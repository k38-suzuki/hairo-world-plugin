/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORERPLUGIN_FILEEXPLORER_H
#define CNOID_FILE_EXPLORERPLUGIN_FILEEXPLORER_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class FileExplorer
{
public:
    FileExplorer();
    virtual ~FileExplorer();

    static void initializeClass(ExtensionManager* ext);
    static void finalizeClass();
};

}

#endif // CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLOREROR_H
