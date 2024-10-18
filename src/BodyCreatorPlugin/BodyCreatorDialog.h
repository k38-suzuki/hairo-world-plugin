/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_DIALOG_H
#define CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_DIALOG_H

#include <cnoid/ListedWidget>
#include <QDialog>
#include <QDialogButtonBox>

namespace cnoid {

class ExtensionManager;

class BodyCreatorDialog : public QDialog
{
public:
    static void initializeClass(ExtensionManager* ext);
    static BodyCreatorDialog* instance();

    ListedWidget* listedWidget() { return listedWidget_; }

    BodyCreatorDialog(QWidget* parent = nullptr);
    virtual ~BodyCreatorDialog();

private:
    ListedWidget* listedWidget_;
    QDialogButtonBox* buttonBox;
};

}

#endif // CNOID_BODY_CREATOR_PLUGIN_BODY_CREATOR_DIALOG_H