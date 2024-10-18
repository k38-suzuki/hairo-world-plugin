/**
   @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_WIDGET_H
#define CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_WIDGET_H

#include <cnoid/Signal>
#include <QWidget>

namespace cnoid {

class Menu;

class ArchiveListWidget : public QWidget
{
public:
    ArchiveListWidget(QWidget* parent = nullptr);
    virtual ~ArchiveListWidget();

    void addItem(const QString& text);
    void addItems(const QStringList& texts);
    void addWidget(QWidget* widget);
    void removeDuplicates();

    void setArchiveKey(const std::string& archive_key);

    Menu* contextMenu();
    
    SignalProxy<void()> sigListUpdated();

protected:
    virtual void onItemDoubleClicked(const std::string& text) = 0;

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_ARCHIVE_LIST_WIDGET_H