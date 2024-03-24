/**
   @author Kenta Suzuki
*/

#include <cnoid/Plugin>
#include <cnoid/ToolBar>
#include <cnoid/MainWindow>
#include <cnoid/RootItem>
#include <cnoid/SimpleColliderItem>

using namespace cnoid;

class ColliderTestPlugin : public Plugin
{
public:
    
    ColliderTestPlugin() : Plugin("ColliderTest")
    {
        require("Body");
    }
    
    virtual bool initialize() override
    {
        std::vector<ToolBar*> toolBars = MainWindow::instance()->toolBars();
        for(auto& bar : toolBars) {
            if(bar->name() == "FileBar") {
                auto button1 = bar->addButton("C");
                button1->sigClicked().connect([&](){ onButtonClicked(); });
            }
        }
        return true;
    }

private:
    
    void onButtonClicked()
    {
        auto colliders = RootItem::instance()->checkedItems<SimpleColliderItem>();

        if(colliders.size() > 1) {
            if(collision(colliders[0], colliders[1])) {
                colliders[0]->setDiffuseColor(Vector3(1.0, 0.0, 0.0));
                colliders[1]->setDiffuseColor(Vector3(0.0, 1.0, 0.0));
            } else {
                colliders[0]->setDiffuseColor(Vector3(0.5, 0.5, 0.5));
                colliders[1]->setDiffuseColor(Vector3(0.5, 0.5, 0.5));
            }
        }
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(ColliderTestPlugin)
