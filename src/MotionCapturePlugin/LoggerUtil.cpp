/**
    @author Kenta Suzuki
*/

#include "LoggerUtil.h"
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <QDateTime>
#include <QDir>
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
    QDir dir(getStandardPath(type).c_str());
    if(!dir.mkdir(directory.c_str())) {

    }
    return getStandardPath(type) + "/" + directory;
}


string mkdirs(const int& type, const std::string& directories)
{
    filesystem::path dir(fromUTF8(getStandardPath(type).c_str()));
    string user_path = toUTF8((dir / directories.c_str()).string());

    filesystem::path user_dir(fromUTF8(user_path));
    if(!filesystem::exists(user_dir)) {
        filesystem::create_directories(user_dir);
    }
    return getStandardPath(type) + "/" + directories;
}

}