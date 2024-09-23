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
    static void initializeClass(ExtensionManager* ext);

    InertiaCalculator();
    virtual ~InertiaCalculator();

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_INERTIA_CALCULATOR_PLUGIN_INERTIA_CALCULATOR_H
