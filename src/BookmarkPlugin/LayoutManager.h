/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_LAYOUT_MANAGER_H
#define CNOID_BOOKMARK_PLUGIN_LAYOUT_MANAGER_H

#include "ArchiveListWidget.h"

namespace cnoid {

class ExtensionManager;

class LayoutManager : public ArchiveListWidget
{
public:
    static void initializeClass(ExtensionManager* ext);

    LayoutManager(QWidget* parent = nullptr);
    virtual ~LayoutManager();

protected:
    virtual void onItemDoubleClicked(const std::string& text) override;

private:
    void onSaveButtonClicked();
    void onOpenButtonClicked();
};

}

#endif // CNOID_BOOKMARK_PLUGIN_LAYOUT_MANAGER_H