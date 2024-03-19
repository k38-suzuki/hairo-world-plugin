/*!
  @author Kenta Suzuki
*/

#include <cnoid/GeneralSliderView>
#include <cnoid/ItemList>
#include <cnoid/MathUtil>
#include <cnoid/MenuManager>
#include <cnoid/Plugin>
#include <cnoid/RootItem>
#include <cnoid/TimeBar>
#include <cnoid/FluidAreaItem>

using namespace cnoid;
using namespace std;

namespace {

Action* useUnsteadyFlow = nullptr;

}

class UnsteadyFlowPlugin : public Plugin
{
    GeneralSliderView* sliderView;
    GeneralSliderView::SliderPtr slider_pt;
    GeneralSliderView::SliderPtr slider_fx;
    GeneralSliderView::SliderPtr slider_fy;
    GeneralSliderView::SliderPtr slider_fz;

    double pt;
    double fx;
    double fy;
    double fz;

public:
    
    UnsteadyFlowPlugin() : Plugin("UnsteadyFlow")
    {
        require("CFD");
    }
    
    virtual bool initialize()
    {
        MenuManager& mm = menuManager().setPath("/Options").setPath("UnsteadyFlow");
        useUnsteadyFlow = mm.addCheckItem("Use an unsteady flow");

        sliderView = GeneralSliderView::instance();
        pt = 3.0;
        fx = 2.0;
        fy = fz = 0.0;

        auto rootItem = RootItem::instance();
        rootItem->sigSelectedItemsChanged().connect(
            [&](const ItemList<>& selectedItems){ onSelectedItemsChanged(selectedItems); });


        auto bar = TimeBar::instance();
        bar->sigTimeChanged().connect([&](double time){ return onTimeChanged(time); });

        return true;
    }

private:

    void onSelectedItemsChanged(const ItemList<>& selectedItems)
    {
        slider_pt = sliderView->getOrCreateSlider("T [s]", 1.0, 10.0, 1);
        slider_pt->setValue(pt);
        slider_pt->setCallback([&](double value){ pt = value; });

        slider_fx = sliderView->getOrCreateSlider("Fx [N]", 0.0, 10.0, 1);
        slider_fx->setValue(fx);
        slider_fx->setCallback([&](double value){ fx = value; });

        slider_fy = sliderView->getOrCreateSlider("Fy [N]", 0.0, 10.0, 1);
        slider_fy->setValue(fy);
        slider_fy->setCallback([&](double value){ fy = value; });

        slider_fz = sliderView->getOrCreateSlider("Fz [N]", 0.0, 10.0, 1);
        slider_fz->setValue(fz);
        slider_fz->setCallback([&](double value){ fz = value; });
    }

    bool onTimeChanged(const double& time)
    {
        double w = radian(360.0) / pt;
        Vector3 flow = Vector3(fx, fy, fz) * sin(w * time);

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