/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATOR_H
#define CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class InertiaCalculatorImpl;

class InertiaCalculator
{
public:
    InertiaCalculator(ExtensionManager* ext);
    virtual ~InertiaCalculator();

    static void initialize(ExtensionManager* ext);

private:
    InertiaCalculatorImpl* impl;
    friend class InertiaCalculatorImpl;
};

}

#endif // CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATOR_H