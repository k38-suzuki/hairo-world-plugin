/**
   @author Kenta Suzuki
*/

#include "ArchiveTreeDialog.h"
#include <cnoid/AppConfig>
#include <cnoid/Buttons>
#include <cnoid/TreeWidget>
#include <cnoid/ValueTree>
#include <QTreeWidgetItem>

using namespace std;
using namespace cnoid;

ArchiveTreeDialog::ArchiveTreeDialog(QWidget* parent)
    : Dialog(parent)
{
    archive_key_ = "default_archive_key";

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

    hbox = new QHBoxLayout;
    auto hbox1 = new QHBoxLayout;
    auto button = new PushButton;
    button->setIcon(QIcon::fromTheme("user-trash"));
    button->sigClicked().connect([&](){ onButtonClicked(); });
    hbox1->addLayout(hbox);
    hbox1->addStretch();
    hbox1->addWidget(button);
    vbox->addLayout(hbox1);

    treeWidget = new TreeWidget;
    treeWidget->setHeaderHidden(true);
    connect(treeWidget, &TreeWidget::itemDoubleClicked,
        [&](QTreeWidgetItem* item, int column){
            string text = item->text(column).toStdString();
            onItemDoubleClicked(text); });

    vbox->addWidget(treeWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(this, &Dialog::finished, [this](int result){ onFinished(result); });
    vbox->addWidget(buttonBox);
}


ArchiveTreeDialog::~ArchiveTreeDialog()
{

}


void ArchiveTreeDialog::addItem(const QString& text)
{
    if(!text.isEmpty()) {
        QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
        item->setText(0, text);
        treeWidget->setCurrentItem(item);
    }
}


void ArchiveTreeDialog::addItems(const QStringList& texts)
{
    for(auto& text : texts) {
        addItem(text);
    }
}


void ArchiveTreeDialog::addWidget(QWidget* widget)
{
    if(widget) {
        hbox->addWidget(widget);
    }
}


void ArchiveTreeDialog::onButtonClicked()
{
    QTreeWidgetItem* item = treeWidget->currentItem();
    if(item) {
        int index = treeWidget->indexOfTopLevelItem(item);
        treeWidget->takeTopLevelItem(index);
    }
}


void ArchiveTreeDialog::storeList()
{
    ListingPtr recentList = new Listing;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = treeWidget->topLevelItem(i);
        if(item) {
            recentList->append(item->text(0).toStdString(), DOUBLE_QUOTED);
        }
    }
    // auto oldRecentList = AppConfig::archive()->openListing(archive_key_);
    // for(auto& node : *oldRecentList) {
    //     if(node->isString()) {
    //         auto name = node->toString();
    //         if(!name.empty()) {
    //             recentList->append(name, DOUBLE_QUOTED);
    //             if(recentList->size() >= MaxArchiveSize) {
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


void ArchiveTreeDialog::updateList()
{
    QStringList list;
    auto& recentList = *AppConfig::archive()->findListing(archive_key_);
    if(recentList.isValid() && !recentList.empty()) {
        for(int i = recentList.size() - 1; i >= 0; --i) {
            if(recentList[i].isString()) {
                list << recentList[i].toString().c_str();
            }
        }
    }
    list.removeDuplicates();
    addItems(list);
}


void ArchiveTreeDialog::onFinished(int result)
{
    storeList();
}


void ArchiveTreeDialog::onAccepted()
{

}


void ArchiveTreeDialog::onRejected()
{

}


void ArchiveTreeDialog::onItemDoubleClicked(std::string& text)
{

}
