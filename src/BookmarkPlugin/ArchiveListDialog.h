/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class Menu;

class ArchiveListDialog : public Dialog
{
public:
    ArchiveListDialog(QWidget* parent = nullptr);
    virtual ~ArchiveListDialog();

    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void addWidget(QWidget* widget);
    void removeDuplicates();

    void setArchiveKey(const std::string& archive_key);

    Menu* contextMenu();

protected:
    virtual void onItemDoubleClicked(const std::string& text) = 0;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H