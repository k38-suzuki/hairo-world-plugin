/**
    @author Kenta Suzuki
*/

#include "BodyCreatorDialog.h"
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <QBoxLayout>
#include "gettext.h"

using namespace cnoid;


void BodyCreatorDialog::initializeClass(ExtensionManager* ext)
{
    MainMenu::instance()->add_Tools_Item(
        _("Create a body"), [](){ BodyCreatorDialog::instance()->show(); });
}


BodyCreatorDialog* BodyCreatorDialog::instance()
{
    static BodyCreatorDialog* dialog = new BodyCreatorDialog;
    return dialog;
}


BodyCreatorDialog::BodyCreatorDialog(QWidget* parent)
    : QDialog(parent)
{
    listedWidget_ = new ListedWidget;

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(listedWidget_);
    mainLayout->addWidget(new HSeparator);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(_("Body Creator"));
}


BodyCreatorDialog::~BodyCreatorDialog()
{

}