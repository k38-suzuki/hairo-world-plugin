/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H

#include <cnoid/Dialog>

namespace cnoid {

class ArchiveListDialog : public Dialog
{
public:
    ArchiveListDialog(QWidget* parent = nullptr);
    ~ArchiveListDialog();

    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void addWidget(QWidget* widget);

    void setArchiveKey(const std::string& archive_key);

protected:
    virtual void onItemDoubleClicked(const std::string& text);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_DIALOG_H