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
    FileExplorer();
    virtual ~FileExplorer();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif
