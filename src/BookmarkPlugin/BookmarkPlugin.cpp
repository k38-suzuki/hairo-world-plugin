/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include "BookmarkManager.h"
#include "HistoryManager.h"
#include "KIOSKManager.h"
#include "LayoutManager.h"
#include "WorldLogManager.h"
#include "HamburgerMenu.h"
#include "ProjectListedDialog.h"

using namespace cnoid;

class BookmarkPlugin : public Plugin
{
public:

    BookmarkPlugin() : Plugin("Bookmark")
    {
        require("Body");
        require("MotionCapture");
    }

    virtual bool initialize() override
    {
        ProjectListedDialog::initializeClass(this);
        HamburgerMenu::initializeClass(this);
        BookmarkManager::initializeClass(this);
        HistoryManager::initializeClass(this);
        KIOSKManager::initializeClass(this);
        LayoutManager::initializeClass(this);
        WorldLogManager::initializeClass(this);
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