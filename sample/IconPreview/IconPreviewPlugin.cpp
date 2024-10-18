/**
    @author Kenta Suzuki
*/

#include <cnoid/MainMenu>
#include <cnoid/Plugin>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>
#include "IconTheme.h"

using namespace cnoid;

class IconPreviewPlugin : public Plugin
{
public:

    IconPreviewPlugin() : Plugin("IconPreview")
    {
        require("Body");
    }

    virtual bool initialize() override
    {
        auto gridLayout = new QGridLayout;
        for(int i = 0; i < themeTexts().size(); ++i) {
            int row = i / 10;
            int column = i % 10;
            QString text = themeTexts().at(i);
            const QIcon themeIcon = QIcon::fromTheme(text);
            auto button = new QPushButton;
            button->setIcon(themeIcon);
            button->setToolTip(text);
            gridLayout->addWidget(button, row, column);
        }

        auto mainLayout = new QVBoxLayout;
        mainLayout->addLayout(gridLayout);

        auto widget = new QWidget;
        widget->setLayout(mainLayout);
        widget->setWindowTitle("Icon Preview");

        MainMenu::instance()->add_Tools_Item(
            ("Icon Preview"), [widget](){ widget->show(); });

        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(IconPreviewPlugin)