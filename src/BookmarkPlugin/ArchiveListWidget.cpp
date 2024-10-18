/**
   @author Kenta Suzuki
*/

#include "ArchiveListWidget.h"
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/Menu>
#include <cnoid/ValueTree>
#include <QBoxLayout>
#include <QFileInfo>
#include <QListWidget>
#include <QListWidgetItem>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class ArchiveListWidget::Impl
{
public:
    ArchiveListWidget* self;

    Impl(ArchiveListWidget* self);
    ~Impl();

    void removeDuplicates();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onClearButtonClicked();
    void updateList();
    void clearList();

    Menu* contextMenu;
    Signal<void()> sigListUpdated;

    QListWidget* listWidget;
    QHBoxLayout* elementLayout;

    std::string archive_key;
};

}


ArchiveListWidget::ArchiveListWidget(QWidget* parent)
    : QWidget(parent)
{
    impl = new Impl(this);
}


ArchiveListWidget::Impl::Impl(ArchiveListWidget* self)
    : self(self)
{
    archive_key = "default_archive_key";

    contextMenu = new Menu;

    auto button1 = new PushButton(_("C"));
    button1->sigClicked().connect([&](){ onClearButtonClicked(); });
    auto button2 = new PushButton(_("AC"));
    button2->sigClicked().connect([&](){ clearList(); });

    listWidget = new QListWidget;
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    self->connect(listWidget, &QListWidget::itemDoubleClicked,
        [&](QListWidgetItem* item){ this->onItemDoubleClicked(item); });

    elementLayout = new QHBoxLayout;

    auto layout = new QHBoxLayout;
    layout->addLayout(elementLayout);
    layout->addStretch();
    layout->addWidget(button1);
    layout->addWidget(button2);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(listWidget);
    self->setLayout(mainLayout);
}


ArchiveListWidget::~ArchiveListWidget()
{
    delete impl;
}


ArchiveListWidget::Impl::~Impl()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->openListing(archive_key);
    recentList.clear();

    for(int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        recentList.append(item->text().toStdString(), DOUBLE_QUOTED);
    }

    if(recentList.empty()) {
        AppConfig::archive()->remove(archive_key);
    }
}


void ArchiveListWidget::addItem(const QString& text)
{
    if(!text.isEmpty()) {
        auto action = new QAction(text);
        connect(action, &QAction::triggered,
            [&, text](){ onItemDoubleClicked(text.toStdString()); });

        QFileInfo info(text);
        if(!info.suffix().isEmpty()) {
            if(info.exists()) {
                impl->listWidget->addItem(text);
                impl->contextMenu->addAction(action);
            }
        } else {
            impl->listWidget->addItem(text);
            impl->contextMenu->addAction(action);
        }
        impl->sigListUpdated();
    }
}


void ArchiveListWidget::addItems(const QStringList& texts)
{
    for(auto& text : texts) {
        addItem(text);
    }
}


void ArchiveListWidget::addWidget(QWidget* widget)
{
    if(widget) {
        impl->elementLayout->addWidget(widget);
    }
}


void ArchiveListWidget::removeDuplicates()
{
    impl->removeDuplicates();
}


void ArchiveListWidget::Impl::removeDuplicates()
{
    QStringList list;
    for(int i = 0; i < listWidget->count(); ++i) {
        auto item = listWidget->item(i);
        list << item->text();
    }
    list.removeDuplicates();
    clearList();
    self->addItems(list);
}


void ArchiveListWidget::setArchiveKey(const std::string& archive_key)
{
    impl->archive_key = archive_key;
    impl->updateList();
}


Menu* ArchiveListWidget::contextMenu()
{
    return impl->contextMenu;
}


SignalProxy<void()> ArchiveListWidget::sigListUpdated()
{
    return impl->sigListUpdated;
}


void ArchiveListWidget::Impl::onItemDoubleClicked(QListWidgetItem* item)
{
    string text = item->text().toStdString();
    self->onItemDoubleClicked(text);
}


void ArchiveListWidget::Impl::onClearButtonClicked()
{
    QListWidgetItem* item = listWidget->currentItem();
    if(item) {
        int row = listWidget->currentRow();
        listWidget->takeItem(row);
        auto action = contextMenu->actions()[row];
        contextMenu->removeAction(action);
        sigListUpdated();
    }
}


void ArchiveListWidget::Impl::updateList()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->findListing(archive_key);
    if(recentList.isValid() && !recentList.empty()) {
        for(int i = 0; i < recentList.size(); ++i) {
            if(recentList[i].isString()) {
                list << recentList[i].toString().c_str();
            }
        }
    }
    list.removeDuplicates();
    clearList();
    self->addItems(list);
}


void ArchiveListWidget::Impl::clearList()
{
    while(listWidget->count()) {
        listWidget->takeItem(0);
    }
    contextMenu->clear();
    sigListUpdated();
}