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
#include <cnoid/MultiColliderItem>
#include <vector>

using namespace std;
using namespace cnoid;

namespace {

Action* useUnsteadyFlow = nullptr;

}

class UnsteadyFlowPlugin : public Plugin
{
    vector<GeneralSliderView::SliderPtr> sliders;
    double pt;
    double fx;
    double fy;
    double fz;

public:

    UnsteadyFlowPlugin() : Plugin("UnsteadyFlow")
    {
        require("Body");
        require("CFD");
    }

    virtual bool initialize() override
    {
        MenuManager& mm = menuManager().setPath("/Options").setPath("UnsteadyFlow");
        useUnsteadyFlow = mm.addCheckItem("Use an unsteady flow");

        sliders.clear();
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
        GeneralSliderView* sliderView = GeneralSliderView::instance();
        sliders.clear();

        ItemList<MultiColliderItem> colliders = selectedItems;
        if(colliders.size()) {
            GeneralSliderView::SliderPtr slider_pt = sliderView->getOrCreateSlider("T [s]", 1.0, 10.0, 1);
            slider_pt->setValue(pt);
            slider_pt->setCallback([&](double value){ pt = value; });

            GeneralSliderView::SliderPtr slider_fx = sliderView->getOrCreateSlider("Fx [N]", 0.0, 10.0, 1);
            slider_fx->setValue(fx);
            slider_fx->setCallback([&](double value){ fx = value; });

            GeneralSliderView::SliderPtr slider_fy = sliderView->getOrCreateSlider("Fy [N]", 0.0, 10.0, 1);
            slider_fy->setValue(fy);
            slider_fy->setCallback([&](double value){ fy = value; });

            GeneralSliderView::SliderPtr slider_fz = sliderView->getOrCreateSlider("Fz [N]", 0.0, 10.0, 1);
            slider_fz->setValue(fz);
            slider_fz->setCallback([&](double value){ fz = value; });

            sliders.push_back(slider_pt);
            sliders.push_back(slider_fx);
            sliders.push_back(slider_fy);
            sliders.push_back(slider_fz);
        }
    }

    bool onTimeChanged(const double& time)
    {
        double w = radian(360.0) / pt;
        Vector3 flow = Vector3(fx, fy, fz) * sin(w * time);

        if(useUnsteadyFlow->isChecked()) {
            ItemList<MultiColliderItem> colliders = RootItem::instance()->checkedItems<MultiColliderItem>();
            for(auto& collider : colliders) {
                collider->setUnsteadyFlow(flow);
            }
        }
        return true;
    }
};

CNOID_IMPLEMENT_PLUGIN_ENTRY(UnsteadyFlowPlugin)