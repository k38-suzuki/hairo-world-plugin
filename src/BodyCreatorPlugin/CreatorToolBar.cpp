/**
   @author Kenta Suzuki
*/

#include "CreatorToolBar.h"
#include <cnoid/BodyItem>
#include <cnoid/RootItem>
#include <QBoxLayout>
#include <cnoid/UTF8>
#include <cnoid/stdx/filesystem>
#include <cnoid/HamburgerMenu>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class CreatorToolBar::Impl
{
public:
    CreatorToolBar* self;

    Impl(CreatorToolBar* self);

    void newFile();
    void save();
    void saveAs();

    void saveFile(const QString& fileName);
    void reloadItem();

    QString fileName;
    QHBoxLayout* elementLayout;

    Signal<void()> sigNewRequested_;
    Signal<void(const string& filename)> sigSaveRequested_;
};

}


CreatorToolBar::CreatorToolBar(QWidget* parent)
    : QWidget(parent)
{
    impl = new Impl(this);
}


CreatorToolBar::Impl::Impl(CreatorToolBar* self)
    : self(self)
{
    const QIcon newIcon = QIcon::fromTheme("document-new");
    QPushButton* newButton = new QPushButton(newIcon, _("&New"), self);
    self->connect(newButton, &QPushButton::clicked, [&](){ newFile(); });

    const QIcon saveIcon = QIcon::fromTheme("document-save");
    QPushButton* saveButton = new QPushButton(saveIcon, _("&Save"), self);
    self->connect(saveButton, &QPushButton::clicked, [&](){ save(); });

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QPushButton* saveAsButton = new QPushButton(saveAsIcon, _("Save &As..."), self);
    self->connect(saveAsButton, &QPushButton::clicked, [&](){ saveAs(); });

    elementLayout = new QHBoxLayout;

    auto mainLayout = new QHBoxLayout;
    mainLayout->addWidget(newButton);
    mainLayout->addWidget(saveButton);
    mainLayout->addWidget(saveAsButton);
    mainLayout->addStretch();
    mainLayout->addLayout(elementLayout);
    self->setLayout(mainLayout);

    self->setWindowTitle(_("Creator ToolBar"));
}


CreatorToolBar::~CreatorToolBar()
{
    delete impl;
}


QPushButton* CreatorToolBar::addButton(const QIcon& icon, const QString& text)
{
    auto pushButton = new QPushButton(icon, text);
    impl->elementLayout->addWidget(pushButton);
    return pushButton;
}


SignalProxy<void()> CreatorToolBar::sigNewRequested()
{
    return impl->sigNewRequested_;
}


SignalProxy<void(const string& filename)> CreatorToolBar::sigSaveRequested()
{
    return impl->sigSaveRequested_;
}


void CreatorToolBar::Impl::newFile()
{
    sigNewRequested_();
}


void CreatorToolBar::Impl::save()
{
    if(!fileName.isEmpty()) {
        saveFile(fileName);
    } else {
        saveAs();
    }
}


void CreatorToolBar::Impl::saveAs()
{
    fileName = getSaveFileName(_("Body"), "body").c_str();
    saveFile(fileName);
}


void CreatorToolBar::Impl::saveFile(const QString& fileName)
{
    string filename = fileName.toStdString();

    if(!filename.empty()) {
        filesystem::path path(fromUTF8(filename));
        string ext = path.extension().string();
        if(ext != ".body") {
           filename += ".body";
        }
        this->fileName = filename.c_str();
        sigSaveRequested_(filename);
    }

    reloadItem();
}


void CreatorToolBar::Impl::reloadItem()
{
    RootItem* rootItem = RootItem::instance();
    ItemList<BodyItem> bodyItems = rootItem->checkedItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        string filename0 = fileName.toStdString();
        string filename1 = bodyItem->filePath().c_str();
        if(filename0 == filename1) {
            bodyItem->reload();
        }
    }
}