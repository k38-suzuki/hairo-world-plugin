/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "BookmarkBar.h"
#include "BookmarkManager.h"
#include "HistoryManager.h"
#include "KIOSKManager.h"
#include "ToolsUtil.h"
#include "WorldLogManager.h"

using namespace cnoid;

class BookmarkPlugin : public Plugin
{
public:

    BookmarkPlugin() : Plugin("Bookmark")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        ToolsMenu::initializeClass(this);
        // BookmarkBar::initialize(this);
        BookmarkManager::initializeClass(this);
        WorldLogManager::initializeClass(this);
        HistoryManager::initializeClass(this);
        KIOSKManager::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("Bookmark Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(BookmarkPlugin)