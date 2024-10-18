/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_PROJECT_LISTED_DIALOG_H
#define CNOID_BOOKMARK_PLUGIN_PROJECT_LISTED_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "ListedWidget.h"

namespace cnoid {

class ExtensionManager;

class ProjectListedDialog : public QDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static ProjectListedDialog* instance();

    ListedWidget* listedWidget() { return listedWidget_; }

    ProjectListedDialog(QWidget* parent = nullptr);
    virtual ~ProjectListedDialog();

private:
    ListedWidget* listedWidget_;
    QDialogButtonBox* buttonBox;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_PROJECT_LISTED_DIALOG_H