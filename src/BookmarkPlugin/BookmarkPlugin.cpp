/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "BookmarkBar.h"
#include "HistoryManager.h"
#include "KIOSKManager.h"

using namespace cnoid;

namespace {

class BookmarkPlugin : public Plugin
{
public:

    BookmarkPlugin() : Plugin("Bookmark")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        BookmarkBar::initialize(this);
        HistoryManager::initializeClass(this);
        KIOSKManager::initializeClass(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("Bookmark Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2023 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BookmarkPlugin)