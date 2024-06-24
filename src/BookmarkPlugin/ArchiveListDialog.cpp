/**
   @author Kenta Suzuki
*/

#include "ArchiveListDialog.h"
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/ValueTree>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>

using namespace std;
using namespace cnoid;

namespace cnoid {

class ArchiveListDialog::Impl
{
public:
    ArchiveListDialog* self;

    Impl(ArchiveListDialog* self);
    ~Impl();

    QListWidget* listWidget;
    QDialogButtonBox* buttonBox;
    std::string archive_key_;
    int max_items;
    QHBoxLayout* hbox;

    void onButtonClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void updateList();
    void clearList();
    void storeList();
};

}


ArchiveListDialog::ArchiveListDialog(QWidget* parent)
    : Dialog(parent)
{
    impl = new Impl(this);
}


ArchiveListDialog::Impl::Impl(ArchiveListDialog* self)
    : self(self)
{
    archive_key_ = "default_archive_key";
    max_items = 16;

    auto vbox = new QVBoxLayout;
    self->setLayout(vbox);

    auto hbox1 = new QHBoxLayout;
    hbox = new QHBoxLayout;
    auto button1 = new PushButton("C");
    button1->sigClicked().connect([&](){ onButtonClicked(); });
    auto button2 = new PushButton("AC");
    button2->sigClicked().connect([&](){ clearList(); });
    hbox1->addLayout(hbox);
    hbox1->addStretch();
    hbox1->addWidget(button1);
    hbox1->addWidget(button2);
    vbox->addLayout(hbox1);

    listWidget = new QListWidget;
    listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    self->connect(listWidget, &QListWidget::itemDoubleClicked,
        [&](QListWidgetItem* item){ this->onItemDoubleClicked(item); });
    vbox->addWidget(listWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    self->connect(buttonBox, &QDialogButtonBox::accepted, self, &QDialog::accept);
    vbox->addWidget(buttonBox);
}


ArchiveListDialog::~ArchiveListDialog()
{
    delete impl;
}


ArchiveListDialog::Impl::~Impl()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->openListing(archive_key_);
    recentList.clear();

    for(int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if(item) {
            recentList.append(item->text().toStdString(), DOUBLE_QUOTED);
        }
    }

    if(recentList.size() == 0) {
        AppConfig::archive()->remove(archive_key_);
    }
}


void ArchiveListDialog::addItem(const QString& text)
{
    if(!text.isEmpty()) {
        impl->listWidget->addItem(text);
    }
}


void ArchiveListDialog::addItems(const QStringList& texts)
{
    impl->listWidget->addItems(texts);
}


void ArchiveListDialog::addWidget(QWidget* widget)
{
    if(widget) {
        impl->hbox->addWidget(widget);
    }
}


void ArchiveListDialog::setArchiveKey(const std::string& archive_key)
{
    impl->archive_key_ = archive_key;
    impl->updateList();
}


void ArchiveListDialog::Impl::onButtonClicked()
{
    QListWidgetItem* item = listWidget->currentItem();
    if(item) {
        int row = listWidget->currentRow();
        listWidget->takeItem(row);
    }
}


void ArchiveListDialog::Impl::onItemDoubleClicked(QListWidgetItem* item)
{
    string text = item->text().toStdString();
    self->onItemDoubleClicked(text);
}


void ArchiveListDialog::Impl::updateList()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->findListing(archive_key_);
    if(recentList.isValid() && !recentList.empty()) {
        for(int i = 0; i < recentList.size(); ++i) {
            if(recentList[i].isString()) {
                list << recentList[i].toString().c_str();
            }
        }
    }

    list.removeDuplicates();
    clearList();
    listWidget->addItems(list);
}


void ArchiveListDialog::Impl::clearList()
{
    while(listWidget->count()) {
        listWidget->takeItem(0);
    }
}


void ArchiveListDialog::onItemDoubleClicked(const std::string& text)
{

}