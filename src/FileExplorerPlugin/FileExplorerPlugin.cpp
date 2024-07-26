/**
   @author Kenta Suzuki
*/

#include <cnoid/Format>
#include <cnoid/Plugin>
#include <cnoid/BodyItem>
#include <cnoid/ItemTreeView>
#include <cnoid/MenuManager>
#include <cnoid/Process>
#include <cnoid/stdx/filesystem>
#include "gettext.h"

using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

class FileExplorerPlugin : public Plugin
{
public:
    FileExplorerPlugin() : Plugin("FileExplorer")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        ItemTreeView::instance()->customizeContextMenu<BodyItem>(
            [&](BodyItem* item, MenuManager& menuManager, ItemFunctionDispatcher menuFunction) {
                menuManager.setPath("/").setPath(_("Open"));
                menuManager.addItem(_("gedit"))->sigTriggered().connect(
                    [&, item](){
                        QProcess::startDetached("gedit",
                            QStringList() << item->filePath().c_str()); });
                menuManager.addItem(_("Nautilus"))->sigTriggered().connect(
                    [&, item](){
                        filesystem::path path(item->filePath().c_str());
                        QProcess::startDetached("nautilus",
                            QStringList() << path.parent_path().string().c_str()); });
                menuManager.setPath("/");
                menuManager.addSeparator();
                menuFunction.dispatchAs<Item>(item);
            });
        return true;
    }

    virtual const char* description() const override
    {
        static std::string text =
            formatC("FileExplorer Plugin Version {}\n", CNOID_FULL_VERSION_STRING) +
            "\n" +
            "Copyright (c) 2020 Japan Atomic Energy Agency.\n"
            "\n" +
            MITLicenseText();
        return text.c_str();
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(FileExplorerPlugin)