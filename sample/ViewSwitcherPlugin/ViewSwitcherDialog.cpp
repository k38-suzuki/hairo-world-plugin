/**
   \file
   \author Kenta Suzuki
*/

#include "ViewSwitcherDialog.h"
#include <cnoid/Action>
#include <cnoid/AppConfig>
#include <cnoid/ComboBox>
#include <cnoid/ExtensionManager>
#include <cnoid/ImageView>
#include <cnoid/Joystick>
#include <cnoid/JoystickCapture>
#include <cnoid/MenuManager>
#include <cnoid/SceneView>
#include <cnoid/ToolBar>
#include <cnoid/ViewManager>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "gettext.h"

using namespace cnoid;
using namespace std;

namespace {

ViewSwitcherDialog* switcherDialog = nullptr;
Action* useViewSwitcher;
JoystickCapture joystick;

}

namespace cnoid {

class ViewSwitcherDialogImpl
{
public:
    ViewSwitcherDialogImpl(ViewSwitcherDialog* self);
    ViewSwitcherDialog* self;

    ComboBox* viewCombo;
    ComboBox* buttonCombo;

    int mode;
    int size;
    bool isImageEnabled;
    vector<ImageView*> images;
    vector<SceneView*> scenes;

    enum InputKey {
        DIRECTIONAL_V,
        A_BUTTON,
        B_BUTTON,
        X_BUTTON,
        Y_BUTTON,
        L_BUTTON,
        R_BUTTON,
        NUM_INPUTS
    };

    void onButtonPressed(const int& id, const bool& isPressed);
    void onAxisPositioned(const int& id, const double& position);
    void setSelectedViews();
    void onAccepted();
    void onRejected();
};

}


ViewSwitcherDialog::ViewSwitcherDialog()
{
    impl = new ViewSwitcherDialogImpl(this);
}


ViewSwitcherDialogImpl::ViewSwitcherDialogImpl(ViewSwitcherDialog* self)
    : self(self)
{
    self->setWindowTitle(_("Switcher Config"));

    mode = 0;
    size = 0;
    images.clear();
    scenes.clear();
    isImageEnabled = false;

    viewCombo = new ComboBox();
    QStringList views = { _("Scene"), _("Image") };
    viewCombo->addItems(views);

    buttonCombo = new ComboBox();
    QStringList buttons = { _("DIRECTIONAL_V"), _("A_BUTTON"), _("B_BUTTON"), _("X_BUTTON"),
                            _("Y_BUTTON"), _("L_BUTTON"), _("R_BUTTON") };
    buttonCombo->addItems(buttons);

    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();
    int index = 0;
    gbox->addWidget(new QLabel(_("View type")), index, 0);
    gbox->addWidget(viewCombo, index++, 1);
    gbox->addWidget(new QLabel(_("Button")), index, 0);
    gbox->addWidget(buttonCombo, index++, 1);

    QPushButton* okButton = new QPushButton(_("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);

    viewCombo->sigCurrentIndexChanged().connect([&](int index){ isImageEnabled = !isImageEnabled; });
}


ViewSwitcherDialog::~ViewSwitcherDialog()
{
    delete impl;
}


void ViewSwitcherDialog::initializeClass(ExtensionManager* ext)
{
    if(!switcherDialog) {
        switcherDialog = ext->manage(new ViewSwitcherDialog());
    }

    ToolBar* bar = new ToolBar(_("View Switcher"));
    ToolButton* button = bar->addButton(_("Switcher"));
    button->sigClicked().connect([&](){ switcherDialog->show(); });
    ext->addToolBar(bar);

    MenuManager& manager = ext->menuManager().setPath("/Options").setPath(N_("ViewSwitcher"));
    Mapping* config = AppConfig::archive()->openMapping("ViewSwitcher");
    useViewSwitcher = manager.addCheckItem(_("Use ViewSwitcher"));
    useViewSwitcher->setChecked(config->get("useViewSwitcher", false));

    joystick.setDevice("/dev/input/js0");

    joystick.sigButton().connect([&](int id, bool isPressed){ switcherDialog->onButtonPressed(id, isPressed); });
    joystick.sigAxis().connect([&](int id, double position){ switcherDialog->onAxisPositioned(id, position); });

}


void ViewSwitcherDialog::finalizeClass()
{
    Mapping* config = AppConfig::archive()->openMapping("ViewSwitcher");
    config->write("useViewSwitcher", useViewSwitcher->isChecked());
}


void ViewSwitcherDialog::onButtonPressed(const int& id, const bool& isPressed)
{
    impl->onButtonPressed(id, isPressed);
}



void ViewSwitcherDialogImpl::onButtonPressed(const int& id, const bool& isPressed)
{
    bool used = useViewSwitcher->isChecked();

    if(used) {
        int index = buttonCombo->currentIndex();
        int buttonId = NUM_INPUTS;
        switch (index) {
        case A_BUTTON:
            buttonId = Joystick::A_BUTTON;
            break;
        case B_BUTTON:
            buttonId = Joystick::B_BUTTON;
            break;
        case X_BUTTON:
            buttonId = Joystick::X_BUTTON;
            break;
        case Y_BUTTON:
            buttonId = Joystick::Y_BUTTON;
            break;
        case L_BUTTON:
            buttonId = Joystick::L_BUTTON;
            break;
        case R_BUTTON:
            buttonId = Joystick::R_BUTTON;
            break;
        default:
            break;
        }

        setSelectedViews();

        if((buttonId == id) && isPressed) {
            mode = ++mode % size;
            if(!isImageEnabled) {
                scenes[mode]->bringToFront();
            } else {
                images[mode]->bringToFront();
            }
        }
    }
}


void ViewSwitcherDialog::onAxisPositioned(const int& id, const double& position)
{
    impl->onAxisPositioned(id, position);
}


void ViewSwitcherDialogImpl::onAxisPositioned(const int& id, const double& position)
{
    bool used = useViewSwitcher->isChecked();

    if(used) {
        int index = buttonCombo->currentIndex();
        int axisId = NUM_INPUTS;
        switch (index) {
        case DIRECTIONAL_V:
            axisId = Joystick::DIRECTIONAL_PAD_V_AXIS;
            break;
        default:
            break;
        }

        setSelectedViews();

        if((axisId == id) && (fabs(position) > 0.0) && (size != 0)) {
            double pos = position;
            if(pos > 0.0) {
                mode = ++mode % size;
            } else if(pos < 0.0) {
                mode = (--mode % size + size) % size;
            }
            if(!isImageEnabled) {
                scenes[mode]->bringToFront();
            } else {
                images[mode]->bringToFront();
            }
        }
    }
}


void ViewSwitcherDialogImpl::setSelectedViews()
{
    std::vector<View*> views = ViewManager::allViews();
    if(!isImageEnabled) {
        scenes.clear();
        for(size_t i = 0; i < views.size(); ++i) {
            View* view = views[i];
            SceneView* scene = dynamic_cast<SceneView*>(view);
            if(scene) {
                scenes.push_back(scene);
            }
        }

        size = scenes.size();
    } else {
        images.clear();
        for(size_t i = 0; i < views.size(); ++i) {
            View* view = views[i];
            ImageView* image = dynamic_cast<ImageView*>(view);
            if(image) {
                images.push_back(image);
            }
        }

        size = images.size();
    }
}


void ViewSwitcherDialog::onAccepted()
{
    impl->onAccepted();
}


void ViewSwitcherDialogImpl::onAccepted()
{

}


void ViewSwitcherDialog::onRejected()
{
    impl->onRejected();
}


void ViewSwitcherDialogImpl::onRejected()
{

}
