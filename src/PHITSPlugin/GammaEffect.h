/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_GAMMA_EFFECT_H
#define CNOID_PHITS_PLUGIN_GAMMA_EFFECT_H

#include "EnergyFilter.h"
#include "PHITSRunner.h"
#include "PHITSWriter.h"

namespace cnoid {

class Camera;

class GammaEffect
{
public:
    GammaEffect(Camera* camera);
    virtual ~GammaEffect();

    void setMaxCas(const int& maxcas) { maxcas_ = maxcas; }
    int maxcas() const { return maxcas_; }
    void setMaxBch(const int& maxbch) { maxbch_ = maxbch; }
    int maxbch() const { return maxbch_; }
    void message(const bool& on) { phitsRunner_.putMessages(on); }

    void start(const bool& checked);

private:
    Camera* camera_;
    PHITSRunner phitsRunner_;
    PHITSWriter phitsWriter_;
    EnergyFilter* energyFilter_;
    int maxcas_;
    int maxbch_;
    bool is_message_checked_;
    std::string default_nuclide_table_file_;
    std::string default_element_table_file_;
    std::string default_energy_filter_file_;
};

}

#endif // CNOID_PHITS_PLUGIN_GAMMA_EFFECT_H
