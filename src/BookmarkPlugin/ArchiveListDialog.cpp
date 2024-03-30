/**
   @author Kenta Suzuki
*/

#include "ArchiveListDialog.h"
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/ValueTree>

using namespace std;
using namespace cnoid;

ArchiveListDialog::ArchiveListDialog(QWidget* parent)
    : Dialog(parent)
{
    archive_key_ = "default_archive_key";
    max_items = 16;

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

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
    connect(listWidget, &QListWidget::itemDoubleClicked,
        [&](QListWidgetItem* item){ onItemDoubleClicked(item); });
    vbox->addWidget(listWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(this, &Dialog::finished, [this](int result){ onFinished(result); });
    vbox->addWidget(buttonBox);
}


ArchiveListDialog::~ArchiveListDialog()
{

}


void ArchiveListDialog::addItem(const QString& text)
{
    if(!text.isEmpty()) {
        listWidget->addItem(text);
        storeList();
    }
}


void ArchiveListDialog::addItems(const QStringList& texts)
{
    listWidget->addItems(texts);
}


void ArchiveListDialog::addWidget(QWidget* widget)
{
    if(widget) {
        hbox->addWidget(widget);
    }
}


void ArchiveListDialog::updateList()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->findListing(archive_key_);
    if(recentList.isValid() && !recentList.empty()) {
        for(auto& node : recentList) {
            if(node->isString()) {
                auto name = node->toString();
                if(!name.empty()) {
                    list << name.c_str();
                }
            }
        }
    }
    list.removeDuplicates();
    clearList();
    listWidget->addItems(list);
}


void ArchiveListDialog::onButtonClicked()
{
    QListWidgetItem* item = listWidget->currentItem();
    if(item) {
        int row = listWidget->currentRow();
        listWidget->takeItem(row);
    }
}


void ArchiveListDialog::onItemDoubleClicked(QListWidgetItem* item)
{
    string text = item->text().toStdString();
    onItemDoubleClicked(text);
}


void ArchiveListDialog::clearList()
{
    while(listWidget->count()) {
        listWidget->takeItem(0);
    }
}


void ArchiveListDialog::storeList()
{
    ListingPtr recentList = new Listing;
    for(int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if(item) {
            recentList->append(item->text().toStdString(), DOUBLE_QUOTED);
        }
    }
    // auto oldRecentList = AppConfig::archive()->openListing(archive_key_);
    // for(auto& node : *oldRecentList) {
    //     if(node->isString()) {
    //         auto name = node->toString();
    //         if(!name.empty()) {
    //             recentList->append(name, DOUBLE_QUOTED);
    //             if(recentList->size() >= max_items) {
    //                 break;
    //             }
    //         }
    //     }
    // }

    QStringList list;
    for(auto& node : *recentList) {
        if(node->isString()) {
            auto name = node->toString();
            if(!name.empty()) {
                list << name.c_str();
            }
        }
    }
    list.removeDuplicates();

    ListingPtr recentList2 = new Listing;
    for(auto& text : list) {
        recentList2->append(text.toStdString(), DOUBLE_QUOTED);
    }
    if(recentList2->size() == 0) {
        recentList2->append("", DOUBLE_QUOTED);
    }
    AppConfig::archive()->insert(archive_key_, recentList2);
}


void ArchiveListDialog::onFinished(int result)
{
    storeList();
}


void ArchiveListDialog::onItemDoubleClicked(const std::string& text)
{

}
