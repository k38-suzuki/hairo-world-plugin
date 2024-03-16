/*!
  @author Kenta Suzuki
*/

#include <cnoid/ItemList>
#include <cnoid/MathUtil>
#include <cnoid/MenuManager>
#include <cnoid/Plugin>
#include <cnoid/RootItem>
#include <cnoid/TimeBar>
#include <cnoid/FluidAreaItem>

using namespace cnoid;

namespace {

Action* useUnsteadyFlow = nullptr;

}

class UnsteadyFlowPlugin : public Plugin
{
public:
    
    UnsteadyFlowPlugin() : Plugin("UnsteadyFlow")
    {
        require("CFD");
    }
    
    virtual bool initialize()
    {
        MenuManager& mm = menuManager().setPath("/Options").setPath("UnsteadyFlow");
        useUnsteadyFlow = mm.addCheckItem("Use an unsteady flow");

        auto bar = TimeBar::instance();
        bar->sigTimeChanged().connect([&](double time){ return onTimeChanged(time); });

        return true;
    }

private:

    bool onTimeChanged(const double& time)
    {
        static const double T = 3.0;
        static const double w = radian(360.0) / T;
        Vector3 flow = Vector3(2.0, 0.0, 0.0) * sin(w * time);

        if(useUnsteadyFlow->isChecked()) {
            ItemList<FluidAreaItem> areaItems = RootItem::instance()->checkedItems<FluidAreaItem>();
            for(auto& areaItem : areaItems) {
                areaItem->setUnsteadyFlow(flow);
            }
        }
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(UnsteadyFlowPlugin)