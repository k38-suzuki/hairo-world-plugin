/**
    @author Kenta Suzuki
*/

#include "LoggerUtil.h"
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <QDateTime>
 #include <QStandardPaths>

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

string getCurrentTimeSuffix()
{
    string suffix;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(currentDateTime.isValid()) {
        suffix = currentDateTime.toString("-yyyy-MM-dd-hh-mm-ss").toStdString();
    }
    return suffix;
}

string getStandardPath(const int& type)
{
    QStandardPaths::StandardLocation pathType;
    switch(type) {
        case Documents:
            pathType = QStandardPaths::DocumentsLocation;
            break;
        case Downloads:
            pathType = QStandardPaths::DownloadLocation;
            break;
        case Music:
            pathType = QStandardPaths::MusicLocation;
            break;
        case Pictures:
            pathType = QStandardPaths::PicturesLocation;
            break;
        case Videos:
            pathType = QStandardPaths::MoviesLocation;
            break;
        case Home:
            pathType = QStandardPaths::HomeLocation;
        default:
            break;
    }
    return QStandardPaths::writableLocation(pathType).toStdString();
}


string mkdir(const int& type, const std::string& directory)
{
    string user_dir = toUTF8((filesystem::path(fromUTF8(getStandardPath(type))) / filesystem::path(fromUTF8(directory))).string());
    filesystem::path path(fromUTF8(user_dir));
    if(!filesystem::exists(path)) {
        filesystem::create_directory(path);
    }
    return user_dir;
}


string mkdirs(const int& type, const std::string& directories)
{
    string user_dir = toUTF8((filesystem::path(fromUTF8(getStandardPath(type))) / filesystem::path(fromUTF8(directories))).string());
    filesystem::path path(fromUTF8(user_dir));
    if(!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
    return user_dir;
}

}