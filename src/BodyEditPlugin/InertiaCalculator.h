/**
   @author Kenta Suzuki
*/

#ifndef CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H
#define CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H

namespace cnoid {

class ExtensionManager;

class InertiaCalculator
{
public:
    InertiaCalculator();
    virtual ~InertiaCalculator();

    static void initializeClass(ExtensionManager* ext);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H
