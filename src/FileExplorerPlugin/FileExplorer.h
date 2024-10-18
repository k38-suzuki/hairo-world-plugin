/**
    @author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLORER_H
#define CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLORER_H

namespace cnoid {

class ExtensionManager;

class FileExplorer
{
public:
    static void initializeClass(ExtensionManager* ext);
    
    FileExplorer();
    virtual ~FileExplorer();
};

}

#endif // CNOID_FILE_EXPLORER_PLUGIN_FILE_EXPLORER_H