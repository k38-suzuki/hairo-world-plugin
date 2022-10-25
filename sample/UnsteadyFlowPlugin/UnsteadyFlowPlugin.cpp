/*!
  @author Kenta Suzuki
*/

#include <cnoid/ItemList>
#include <cnoid/Plugin>
#include <cnoid/RootItem>
#include <cnoid/TimeBar>
#include <cnoid/FluidAreaItem>

using namespace cnoid;

class UnsteadyFlowPlugin : public Plugin
{
public:
    
    UnsteadyFlowPlugin() : Plugin("UnsteadyFlow")
    {

    }
    
    virtual bool initialize()
    {
        auto bar = TimeBar::instance();
        bar->sigTimeChanged().connect([&](double time){ return onTimeChanged(time); });

        return true;
    }

private:

    bool onTimeChanged(const double& time)
    {
        ItemList<FluidAreaItem> areaItems = RootItem::instance()->checkedItems<FluidAreaItem>();
        for(auto& areaItem : areaItems) { 
            if(time < 2.0) {
                areaItem->setUnsteadyFlow(Vector3(2.0, 0.0, 0.0));
            } else if(time < 4.0) {
                areaItem->setUnsteadyFlow(Vector3(-2.0, 0.0, 0.0));
            }
        }
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(UnsteadyFlowPlugin)
