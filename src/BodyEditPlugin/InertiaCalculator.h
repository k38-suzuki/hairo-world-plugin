/**
   @author Kenta Suzuki
*/

#ifndef CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H
#define CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class InertiaCalculatorImpl;

class InertiaCalculator
{
public:
    InertiaCalculator();
    virtual ~InertiaCalculator();

    static void initializeClass(ExtensionManager* ext);

private:
    InertiaCalculatorImpl* impl;
    friend class InertiaCalculatorImpl;
};

}

#endif
