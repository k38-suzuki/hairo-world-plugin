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
    InertiaCalculator();
    virtual ~InertiaCalculator();

    static void initialize(ExtensionManager* ext);

    void show();

private:
    InertiaCalculatorImpl* impl;
    friend class InertiaCalculatorImpl;
};

}

#endif // CNOID_INERTIACALCULATORPLUGIN_INERTIACALCULATOR_H
