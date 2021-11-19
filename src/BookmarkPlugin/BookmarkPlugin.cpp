/**
   \file
   \author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <fmt/format.h>
#include "BookmarkManager.h"
#include "HistoryManager.h"

using namespace cnoid;

namespace {

class BookmarkPlugin : public Plugin
{
public:

    BookmarkPlugin() : Plugin("Bookmark")
    {

    }

    virtual bool initialize() override
    {
        BookmarkManager::initialize(this);
        HistoryManager::initialize(this);
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            fmt::format("Bookmark Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2021 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

}

CNOID_IMPLEMENT_PLUGIN_ENTRY(BookmarkPlugin)
