/**
    @author Kenta Suzuki
*/

#include "MinIOClient.h"
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/MessageView>
#include <cnoid/Process>
#include <cnoid/stdx/filesystem>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class MinIOClient::Impl
{
public:
    MinIOClient* self;

    Impl(MinIOClient* self);
    ~Impl();

    void onProcess0Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess1Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess2Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess3Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess4Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess5Finished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcess6Finished(int exitCode, QProcess::ExitStatus exitStatus);

    Process process0;
    Process process1;
    Process process2;
    Process process3;
    Process process4;
    Process process5;
    Process process6;

    Signal<void(const string& bucket_name)> sigBucketCreated_;
    Signal<void(const string& bucket_name)> sigBucketDeleted_;
    Signal<void(vector<string> bucket_names)> sigBucketListed_;
    Signal<void(const string& object_name)> sigObjectUploaded_;
    Signal<void(const string& object_name)> sigObjectDownloaded_;
    Signal<void(const string& object_name)> sigObjectDeleted_;
    Signal<void(vector<string> object_names)> sigObjectListed_;

    QString aliasName;
    QString bucketName;
};

}


void MinIOClient::initializeClass(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized) {
        initialized = filesystem::is_regular_file("/usr/local/bin/mc");
        string text = _("MinIO Client found.");
        string text2 = _("MinIO Client not found.");
        MessageView::instance()->putln(formatR("{0}", initialized ? text : text2));
    }
}


MinIOClient::MinIOClient(QObject* parent)
    : QObject(parent)
{
    impl = new Impl(this);
}


MinIOClient::MinIOClient(const QString& aliasName, const QString& bucketName, QObject* parent)
    : impl(new Impl(this))
{
    setAlias(aliasName);
    setBucket(bucketName);
}


MinIOClient::Impl::Impl(MinIOClient* self)
    : self(self),
      aliasName(""),
      bucketName("")
{
    self->connect(&process0, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess0Finished(exitCode, exitStatus); });
    self->connect(&process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess1Finished(exitCode, exitStatus); });
    self->connect(&process2, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess2Finished(exitCode, exitStatus); });
    self->connect(&process3, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess3Finished(exitCode, exitStatus); });
    self->connect(&process4, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess4Finished(exitCode, exitStatus); });
    self->connect(&process5, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess5Finished(exitCode, exitStatus); });
    self->connect(&process6, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [this](int exitCode, QProcess::ExitStatus exitStatus){ onProcess6Finished(exitCode, exitStatus); });
}


MinIOClient::~MinIOClient()
{
    delete impl;
}


MinIOClient::Impl::~Impl()
{

}


QString MinIOClient::alias() const
{
    return impl->aliasName;
}


void MinIOClient::setAlias(const QString& aliasName)
{
    impl->aliasName = aliasName;
}


QString MinIOClient::bucket() const
{
    return impl->bucketName;
}


void MinIOClient::setBucket(const QString& bucketName)
{
    impl->bucketName = bucketName;
}


void MinIOClient::createBucket(const QString& bucketName)
{
    QString str = QString("%1/%2")
        .arg(impl->aliasName).arg(bucketName);
    impl->process0.start("mc", QStringList() << "mb" << str);
}


void MinIOClient::deleteBucket(const QString& bucketName)
{
    QString str = QString("%1/%2")
        .arg(impl->aliasName).arg(bucketName);
    impl->process1.start("mc", QStringList() << "rb" << str);
}


void MinIOClient::listBuckets()
{
    QString str = QString("%1/")
        .arg(impl->aliasName);
        impl->process2.start("mc", QStringList() << "ls" << str);
}


void MinIOClient::putObject(const QString& fileName, const QString& newPath)
{
    filesystem::path filePath(fileName.toStdString());
    string filename = filePath.filename().string();
    QString str = QString("%1/%2/%3/%4")
        .arg(impl->aliasName).arg(impl->bucketName).arg(newPath).arg(filename.c_str());
    impl->process3.start("mc", QStringList() << "cp" << fileName << str);
}


void MinIOClient::getObject(const QString& fileName, const QString& objectKey)
{
    impl->process4.start("mc", QStringList() << "get" << objectKey << fileName);
}


void MinIOClient::deleteObject(const QString& objectKey)
{
    QString str = QString("%1/%2/%3")
        .arg(impl->aliasName).arg(impl->bucketName).arg(objectKey);
    impl->process5.start("mc", QStringList() << "rm" << str);
}


void MinIOClient::listObjects()
{
    QString str = QString("%1/%2")
        .arg(impl->aliasName).arg(impl->bucketName);
    impl->process6.start("mc", QStringList() << "ls" << "--recursive" << str);
}


SignalProxy<void(const string& bucket_name)> MinIOClient::sigBucketCreated()
{
    return impl->sigBucketCreated_;
}


SignalProxy<void(const string& bucket_name)> MinIOClient::sigBucketDeleted()
{
    return impl->sigBucketDeleted_;
}


SignalProxy<void(vector<string> bucket_names)> MinIOClient::sigBucketListed()
{
    return impl->sigBucketListed_;
}


SignalProxy<void(const string& filename)> MinIOClient::sigObjectUploaded()
{
    return impl->sigObjectUploaded_;
}


SignalProxy<void(const string& filename)> MinIOClient::sigObjectDownloaded()
{
    return impl->sigObjectDownloaded_;
}


SignalProxy<void(const string& filename)> MinIOClient::sigObjectDeleted()
{
    return impl->sigObjectDeleted_;
}


SignalProxy<void(vector<string> object_names)> MinIOClient::sigObjectListed()
{
    return impl->sigObjectListed_;

}


void MinIOClient::Impl::onProcess0Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // create bucket
        QString text(process0.readAllStandardOutput());
        QStringList list = text.split("`");
        if(!text.contains("ERROR")) {
            sigBucketCreated_(list[1].toStdString());
        }
    }
}


void MinIOClient::Impl::onProcess1Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // delete bucket
        QString text(process1.readAllStandardOutput());
        QStringList list = text.split("`");
        if(!text.contains("ERROR")) {
            sigBucketDeleted_(list[1].toStdString());
        }
    }
}


void MinIOClient::Impl::onProcess2Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // list buckets
        QStringList items;
        vector<string> buckets;
        QString text(process2.readAllStandardOutput());
        QStringList list = text.split("\n");
        for(int i = 0; i < list.size(); ++i) {
            QStringList list2 = list[i].split(" ");
            QString bucketName = list2[list2.size() - 1];
            if(!bucketName.isEmpty()) {
                items << bucketName;
                buckets.push_back(bucketName.toStdString());
            }
        }
        sigBucketListed_(buckets);
    }
}


void MinIOClient::Impl::onProcess3Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // put object
        QString text(process3.readAllStandardOutput());
        QStringList list = text.split("`");
        if(!text.contains("ERROR") && list.size() != 1) {
            sigObjectUploaded_(list[3].toStdString());
        }
    }
}


void MinIOClient::Impl::onProcess4Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // get object
        QString text(process4.readAllStandardOutput());
        QStringList list = text.split("`");
        if(!text.contains("ERROR") && list.size() != 1) {
            sigObjectDownloaded_(list[3].toStdString());
        }
    }
}


void MinIOClient::Impl::onProcess5Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // delete object
        QString text(process5.readAllStandardOutput());
        QStringList list = text.split("`");
        if(!text.contains("ERROR")) {
            sigObjectDeleted_(list[1].toStdString());
        }
    }
}


void MinIOClient::Impl::onProcess6Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit || exitCode != 0) {

    } else {
        // list objects
        QStringList items;
        vector<string> objects;
        QString text(process6.readAllStandardOutput());
        QStringList list = text.split("\n");
        for(int i = 0; i < list.size(); ++i) {
            QStringList list2 = list[i].split(" ");
            QString objectName = list2[list2.size() - 1];
            if(!objectName.isEmpty()) {
                items << objectName;
                objects.push_back(objectName.toStdString());
            }
        }
        sigObjectListed_(objects);
    }
}