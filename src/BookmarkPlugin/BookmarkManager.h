/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H

#include <string>

namespace cnoid {

class ExtensionManager;

class BookmarkManager
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BookmarkManager* instance();

    BookmarkManager();
    virtual ~BookmarkManager();

    void show();
    void hide();
    void addProjectFile(const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_BOOKMARK_MANAGER_H
