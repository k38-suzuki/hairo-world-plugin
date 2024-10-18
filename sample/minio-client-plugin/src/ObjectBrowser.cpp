/**
    @author Kenta Suzuki
*/

#include "ObjectBrowser.h"
#include <cnoid/Buttons>
#include <cnoid/ComboBox>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/Format>
#include <cnoid/LineEdit>
#include <cnoid/MainMenu>
#include <cnoid/MessageView>
#include <cnoid/Separator>
#include <cnoid/TreeWidget>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <cnoid/MinIOClient>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <vector>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = stdx::filesystem;

namespace {

ObjectBrowser* browserInstance = nullptr;

}

namespace cnoid {

class ObjectBrowser::Impl : public Dialog
{
public:

    Impl();
    ~Impl();

    void onTextEdited(const QString& text);
    void onBucketListed(vector<string> bucket_names);
    void onNewButtonClicked();
    void onUpdateButtonClicked();
    void onDownloadButtonClicked();
    void onDeleteButtonClicked();
    void onObjectUploaded(const QString& objectName);
    void onObjectListed(vector<string> object_names);
    QStringList checkedObjects() const;

    LineEdit* aliasLineEdit;
    ComboBox* bucketComboBox;
    TreeWidget* treeWidget;

    Signal<void(const string& object_name)> sigObjectDownloaded_;

    QHBoxLayout* elementLayout;
    QDialogButtonBox* buttonBox;

    MinIOClient* client;
    vector<MinIOClient*> downloaders;
};

}


void ObjectBrowser::initializeClass(ExtensionManager* ext)
{
    if(!browserInstance) {
        browserInstance = ext->manage(new ObjectBrowser);

        MainMenu::instance()->add_Tools_Item(
            _("Object Browser"), [](){ browserInstance->impl->show(); });
    }
}


ObjectBrowser* ObjectBrowser::instance()
{
    return browserInstance;
}


ObjectBrowser::ObjectBrowser()
{
    impl = new Impl;
}


ObjectBrowser::Impl::Impl()
    : Dialog()
{
    downloaders.clear();

    aliasLineEdit = new LineEdit;
    aliasLineEdit->sigTextEdited().connect([&](const QString& text){ onTextEdited(text); });

    bucketComboBox = new ComboBox;
    bucketComboBox->sigCurrentIndexChanged().connect([&](int index){ onUpdateButtonClicked(); });;

    const QIcon newIcon = QIcon::fromTheme("folder-new");
    auto button1 = new PushButton;
    button1->setIcon(newIcon);
    button1->setToolTip(_("Create a new bucket"));
    button1->sigClicked().connect([&](){ onNewButtonClicked(); });

    const QIcon updateIcon = QIcon::fromTheme("view-refresh");
    auto button2 = new PushButton;
    button2->setIcon(updateIcon);
    button2->setToolTip(_("Update the object list"));
    button2->sigClicked().connect([&](){ onUpdateButtonClicked(); });

    const QIcon downloadIcon = QIcon::fromTheme("emblem-downloads");
    auto button3 = new PushButton;
    button3->setIcon(downloadIcon);
    button3->setToolTip(_("Download objects"));
    button3->sigClicked().connect([&](){ onDownloadButtonClicked(); });

    const QIcon deleteIcon = QIcon::fromTheme("user-trash");
    auto button4 = new PushButton;
    button4->setIcon(deleteIcon);
    button4->setToolTip(_("Delete objects"));
    button4->sigClicked().connect([&](){ onDeleteButtonClicked(); });

    treeWidget = new TreeWidget(this);
    treeWidget->setHeaderLabels(QStringList() << _("Download") << _("Object"));

    elementLayout = new QHBoxLayout;

    auto layout = new QHBoxLayout;
    layout->addWidget(new QLabel(_("Alias")));
    layout->addWidget(aliasLineEdit);
    layout->addWidget(bucketComboBox);
    layout->addWidget(button1);
    layout->addWidget(button2);
    layout->addWidget(button3);
    layout->addWidget(button4);
    layout->addLayout(elementLayout);
    // layout->addStretch();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(treeWidget);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Object Browser"));
}


ObjectBrowser::~ObjectBrowser()
{
    delete impl;
}


ObjectBrowser::Impl::~Impl()
{

}


void ObjectBrowser::addWidget(QWidget* widget)
{
    impl->elementLayout->addWidget(widget);
}


void ObjectBrowser::putObject(const QString& fileName, const QString& newPath)
{
    QString aliasName = impl->aliasLineEdit->text();
    QString bucketName = impl->bucketComboBox->currentText();

    if(!bucketName.isEmpty()) {
        auto mc = new MinIOClient;
        mc->setAlias(aliasName);
        mc->setBucket(bucketName);
        mc->sigObjectUploaded().connect(
            [&](const string& object_name){ impl->onObjectUploaded(object_name.c_str()); });
        mc->putObject(fileName, newPath);
    }
}


SignalProxy<void(const string& object_name)> ObjectBrowser::sigObjectDownloaded()
{
    return impl->sigObjectDownloaded_;
}


void ObjectBrowser::Impl::onTextEdited(const QString& text)
{
    bucketComboBox->clear();

    if(!text.isEmpty()) {
        auto mc = new MinIOClient;
        mc->sigBucketListed().connect(
            [&](vector<string> bucket_names){ onBucketListed(bucket_names); });
        mc->setAlias(text);
        mc->listBuckets();
    }
}


void ObjectBrowser::Impl::onBucketListed(vector<string> bucket_names)
{
    for(auto& bucket_name : bucket_names) {
        bucketComboBox->addItem(bucket_name.c_str());
    }
}


void ObjectBrowser::Impl::onNewButtonClicked()
{
    QString aliasName = aliasLineEdit->text();

    if(!aliasName.isEmpty()) {
        bool ok;
        QString text = QInputDialog::getText(this, _("Create Bucket"),
            _("Bucket name:"), QLineEdit::Normal, "mybucket", &ok);

        if(ok && !text.isEmpty()) {
            auto mc = new MinIOClient;
            mc->setAlias(aliasName);
            mc->createBucket(text);
            onTextEdited(aliasName);
        }
    }
}


void ObjectBrowser::Impl::onUpdateButtonClicked()
{
    treeWidget->clear();

    QString aliasName = aliasLineEdit->text();
    QString bucketName = bucketComboBox->currentText();

    if(!bucketName.isEmpty()) {
        client = new MinIOClient;
        client->setAlias(aliasName);
        client->setBucket(bucketName);
        client->sigObjectListed().connect([&](vector<string> object_names){ onObjectListed(object_names); });
        client->listObjects();
    }
}


void ObjectBrowser::Impl::onDownloadButtonClicked()
{
    downloaders.clear();

    QString aliasName = aliasLineEdit->text();
    QString bucketName = bucketComboBox->currentText();

    QStringList items = checkedObjects();

    for(auto& objectName : items) {
        filesystem::path objectPath(fromUTF8(aliasName.toStdString() + "/" + bucketName.toStdString()));
        string object_key = toUTF8((objectPath / filesystem::path(fromUTF8(objectName.toStdString()))).string());
        QString fileName = QString("./minio_ws/%1")
            .arg(objectName);

        auto mc = new MinIOClient(aliasName, bucketName);
        mc->sigObjectDownloaded().connect([&](const string& object_name){ sigObjectDownloaded_(object_name); });
        downloaders.push_back(mc);
        mc->getObject(fileName, object_key.c_str());
    }
}


void ObjectBrowser::Impl::onDeleteButtonClicked()
{
    QString aliasName = aliasLineEdit->text();
    QString bucketName = bucketComboBox->currentText();

    QStringList items = checkedObjects();

    if(items.size() > 0) {
        QMessageBox msgBox;
        msgBox.setText(_("The objects are checked."));
        msgBox.setInformativeText(_("Do you want to delete objects?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setWindowTitle(_("Delete Objects"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        switch(ret) {
            case QMessageBox::Yes:
                // Yes was clicked
                for(auto& objectName : items) {
                    auto mc = new MinIOClient(aliasName, bucketName);
                    mc->deleteObject(objectName);
                }
                break;
            case QMessageBox::Discard:
                // No was clicked
                break;
            default:
                // should never be reached
                break;
        }
    }
}


void ObjectBrowser::Impl::onObjectUploaded(const QString& objectName)
{
    MessageView::instance()->putln(formatR(_("{0} has been uploaded."), objectName.toStdString()));
    onUpdateButtonClicked();
}


void ObjectBrowser::Impl::onObjectListed(vector<string> object_names)
{
    for(auto& object_name : object_names) {
        auto item = new QTreeWidgetItem(treeWidget);
        item->setCheckState(0, Qt::Unchecked);
        item->setText(1, object_name.c_str());
    }

    if(treeWidget->topLevelItemCount() > 0) {
        auto item = treeWidget->topLevelItem(0);
        treeWidget->setCurrentItem(item);
    }
}


QStringList ObjectBrowser::Impl::checkedObjects() const
{
    QStringList items;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        auto item = treeWidget->topLevelItem(i);
        if(item->checkState(0) == Qt::Checked) {
            items << item->text(1);
        }
    }
    return items;
}