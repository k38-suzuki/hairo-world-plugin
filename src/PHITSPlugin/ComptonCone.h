/**
   @author Kenta Suzuki
*/

#ifndef CNOID_PHITS_PLUGIN_COMPTON_CONE_H
#define CNOID_PHITS_PLUGIN_COMPTON_CONE_H

#include <string>
#include <vector>
#include "ComptonCamera.h"

namespace cnoid {

class ComptonCone
{
public:
    ComptonCone();
    virtual ~ComptonCone();

    static bool readComptonCone(std::string strFName, double Energy, ComptonCamera* camera);

    void setPosition(double ScatterX, double ScatterY);
    void setDirection(int index, double ScatterX, double ScatterY, double AbsorbX, double AbsorbY, double Gapsa);
    void setHAngle(double angle) { HAngle = angle; }
    bool isPointContainedInArm(std::vector<double> point, double arm);

private:
    std::vector<double> _Position; // vertex position
    std::vector<double> _Direction; // bus vector
    double HAngle; // half-top angle

//     double *Position{ get; set; }
//     double *Direction{ get; set; }
//     double HAngle{ get; set; }

};

}

#endif // CNOID_PHITS_PLUGIN_COMPTON_CONE_H
