/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODY_EDIT_PLUGIN_BODY_EDITOR_H
#define CNOID_BODY_EDIT_PLUGIN_BODY_EDITOR_H

#include <cnoid/ExtensionManager>

namespace cnoid {

class BodyEdit
{
public:
    BodyEdit();
    virtual ~BodyEdit();

    static void initializeClass(ExtensionManager* ext);
};

}

#endif // CNOID_BODY_EDIT_PLUGIN_BODY_EDITOR_H
