/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BOOKMARK_PLUGIN_HAMBURGER_BUTTON_H
#define CNOID_BOOKMARK_PLUGIN_HAMBURGER_BUTTON_H

#include <cnoid/PushButton>
#include "exportdecl.h"

namespace cnoid {

class Action;
class Menu;

class CNOID_EXPORT HamburgerButton : public PushButton
{
public:
    HamburgerButton(QWidget* parent = nullptr);
    HamburgerButton(const QString& text, QWidget* parent = nullptr);
    HamburgerButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);

    Action* addAction(const std::string& text);
    Menu* addMenu(const std::string& title);

private:
    void initialize();

    QMenu* menu_;
};

}

#endif // CNOID_BOOKMARK_PLUGIN_HAMBURGER_BUTTON_H