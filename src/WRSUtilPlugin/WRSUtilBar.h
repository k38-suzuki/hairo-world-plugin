/**
   @author Kenta Suzuki
*/

#ifndef CNOID_WRS_UTIL_PLUGIN_WRS_UTIL_BAR_H
#define CNOID_WRS_UTIL_PLUGIN_WRS_UTIL_BAR_H

#include <cnoid/ToolBar>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT WRSUtilBar : public ToolBar
{
public:
    static void initialize(ExtensionManager* ext);
    static WRSUtilBar* instance();

    WRSUtilBar(const std::string& name);

    struct FormatInfo {
        std::string directory;
        double format_version;
    };

    void addFormat(FormatInfo info);
    void setProjectDirectory(const std::string& directory);
    void setRegistrationFile(const std::string& filename);
    void update();

private:
    class Impl;
    Impl* impl;

    WRSUtilBar();
    virtual ~WRSUtilBar();
};

}

#endif // CNOID_WRS_UTIL_PLUGIN_WRS_UTIL_BAR_H