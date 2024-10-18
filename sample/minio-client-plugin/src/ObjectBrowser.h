/**
    @author Kenta Suzuki
*/

#ifndef CNOID_MINIO_CLIENT_PLUGIN_OBJECT_BROWSER_H
#define CNOID_MINIO_CLIENT_PLUGIN_OBJECT_BROWSER_H

#include <cnoid/Signal>
#include <QWidget>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;

class CNOID_EXPORT ObjectBrowser
{
public:
    static void initializeClass(ExtensionManager* ext);
    static ObjectBrowser* instance();

    ObjectBrowser();
    virtual ~ObjectBrowser();

    void addWidget(QWidget* widget);
    void putObject(const QString& fileName, const QString& newPath);

    SignalProxy<void(const std::string& object_name)> sigObjectDownloaded();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_MINIO_CLIENT_PLUGIN_OBJECT_BROWSER_H