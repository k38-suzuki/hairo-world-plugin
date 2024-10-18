/**
    @author Kenta Suzuki
*/

#include "ProjectListedDialog.h"
#include <cnoid/ExtensionManager>
#include <cnoid/MainMenu>
#include <cnoid/Separator>
#include <QBoxLayout>
#include "gettext.h"

using namespace cnoid;


void ProjectListedDialog::initializeClass(ExtensionManager* ext)
{
    MainMenu::instance()->add_Tools_Item(
        _("Show the manager list"), [](){ ProjectListedDialog::instance()->show(); });
}


ProjectListedDialog* ProjectListedDialog::instance()
{
    static ProjectListedDialog* dialog = new ProjectListedDialog;
    return dialog;
}


ProjectListedDialog::ProjectListedDialog(QWidget* parent)
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

    setWindowTitle(_(" "));
}


ProjectListedDialog::~ProjectListedDialog()
{

}