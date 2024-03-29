/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H

#include <cnoid/Dialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>

namespace cnoid {

class ArchiveListDialog : public Dialog
{
public:
    ArchiveListDialog(QWidget* parent = nullptr);
    ~ArchiveListDialog();

    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void addWidget(QWidget* widget);
    void updateList();

    void setArchiveKey(const std::string& archive_key) { archive_key_ = archive_key; }

protected:
    virtual void onFinished(int result) override;
    virtual void onItemDoubleClicked(std::string& text);

private:
    QListWidget* listWidget;
    QDialogButtonBox* buttonBox;
    std::string archive_key_;
    int max_items;
    QHBoxLayout* hbox;

    void onButtonClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void storeList();
};

}

#endif // CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H
