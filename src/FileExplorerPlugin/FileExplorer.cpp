/**
    @author Kenta Suzuki
*/

#include "FileExplorer.h"
#include <cnoid/BodyItem>
#include <cnoid/Buttons>
#include <cnoid/Dialog>
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/Process>
#include <cnoid/RootItem>
#include <cnoid/Separator>
#include <cnoid/TreeWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include "gettext.h"

using namespace cnoid;

namespace {

class ExplorerDialog : public QDialog
{
public:
    ExplorerDialog(QWidget* parent = nullptr);

private:
    void on_updateButton_clicked();
    void on_fileButton_clicked();
    void on_folderButton_clicked();

    QTreeWidget* treeWidget;
    QDialogButtonBox* buttonBox;
};

}


void FileExplorer::initializeClass(ExtensionManager* ext)
{
    static ExplorerDialog* dialog = nullptr;

    if(!dialog) {
        dialog = ext->manage(new ExplorerDialog);

        MainMenu::instance()->add_Tools_Item(
            _("File Explorer"), [](){ dialog->show(); });
    }
}


FileExplorer::FileExplorer()
{

}


FileExplorer::~FileExplorer()
{

}


ExplorerDialog::ExplorerDialog(QWidget* parent)
    : QDialog(parent)
{
    setMinimumSize(640, 480);

    treeWidget = new QTreeWidget(this);
    treeWidget->setHeaderLabels(QStringList()<< _("Body") << _("File Path"));

    const QIcon updateIcon = QIcon::fromTheme("view-refresh");
    auto updateButton = new QPushButton(updateIcon, _("Update"));
    connect(updateButton, &QPushButton::clicked, [&](){ on_updateButton_clicked(); });

    const QIcon fileIcon = QIcon::fromTheme("document-open");
    auto fileButton = new QPushButton(fileIcon, _("gedit"));
    connect(fileButton, &QPushButton::clicked, [&](){ on_fileButton_clicked(); });

    const QIcon folderIcon = QIcon::fromTheme("folder");
    auto folderButton = new QPushButton(folderIcon, _("Nautilus"));
    connect(folderButton, &QPushButton::clicked, [&](){ on_folderButton_clicked(); });

    auto layout = new QHBoxLayout;
    layout->addWidget(updateButton);
    layout->addWidget(fileButton);
    layout->addWidget(folderButton);
    layout->addStretch();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(treeWidget);
    // mainLayout->addStretch();
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("File Explorer"));
}


void ExplorerDialog::on_updateButton_clicked()
{
    treeWidget->clear();

    auto bodyItems = RootItem::instance()->descendantItems<BodyItem>();
    for(auto& bodyItem : bodyItems) {
        auto item = new QTreeWidgetItem(treeWidget);
        auto name = bodyItem->body()->name();
        auto filename = bodyItem->filePath();
        item->setText(0, name.c_str());
        item->setText(1, filename.c_str());
    }
}


void ExplorerDialog::on_fileButton_clicked()
{
    auto items = treeWidget->selectedItems();
    for(auto& item : items) {
        QString fileName = item->text(1);
        QProcess::startDetached("gedit", QStringList() << fileName);
    }
}


void ExplorerDialog::on_folderButton_clicked()
{
    auto items = treeWidget->selectedItems();
    for(auto& item : items) {
        QString fileName = item->text(1);
        QProcess::startDetached("nautilus", QStringList() << "-s" << fileName);
    }
}