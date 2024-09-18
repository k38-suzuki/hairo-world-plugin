/**
    @author Kenta Suzuki
*/

#ifndef CNOID_MOTION_CAPTURE_PLUGIN_LOGGER_UTIL_H
#define CNOID_MOTION_CAPTURE_PLUGIN_LOGGER_UTIL_H

#include <string>
#include "exportdecl.h"

namespace cnoid {

enum StandardPath {
    Documents,
    Downloads,
    Music,
    Pictures,
    Videos,
    Home
};

CNOID_EXPORT std::string getCurrentTimeSuffix();

CNOID_EXPORT std::string getStandardPath(const int& type);

CNOID_EXPORT std::string mkdir(const int& type, const std::string& directory);

CNOID_EXPORT std::string mkdirs(const int& type, const std::string& directories);

}

#endif // CNOID_MOTION_CAPTURE_PLUGIN_LOGGER_UTIL_H