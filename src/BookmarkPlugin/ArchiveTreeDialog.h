/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_ARCHIVE_TREE_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_ARCHIVE_TREE_DIALOG_H

#include <cnoid/Dialog>
#include <QDialogButtonBox>
#include <QBoxLayout>

namespace cnoid {

class TreeWidget;

class ArchiveTreeDialog : public Dialog
{
public:
    ArchiveTreeDialog(QWidget* parent = nullptr);
    ~ArchiveTreeDialog();

    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void addWidget(QWidget* widget);
    void updateList();

    void setArchiveKey(const std::string& archive_key) { archive_key_ = archive_key; }

    enum { MaxArchiveSize = 16 };

protected:
    virtual void onFinished(int result) override;
    virtual void onAccepted() override;
    virtual void onRejected() override;
    virtual void onItemDoubleClicked(std::string& text);

private:
    TreeWidget* treeWidget;
    QDialogButtonBox* buttonBox;
    std::string archive_key_;
    QHBoxLayout* hbox;

    void onButtonClicked();
    void storeList();
};

}

#endif // CNOID_BOOKMARK_PLUGIN_ARCHIVE_TREE_DIALOG_H
