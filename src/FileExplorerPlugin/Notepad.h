/**
   @author Kenta Suzuki
*/

#ifndef CNOID_FILE_EXPLORER_PLUGIN_NOTEPAD_H
#define CNOID_FILE_EXPLORER_PLUGIN_NOTEPAD_H

#include <QMainWindow>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT Notepad : public QMainWindow
{
public:
    Notepad(QWidget* parent = nullptr);
    virtual ~Notepad();

    void loadFile(const QString& fileName);

private:
    class Impl;
    Impl* impl;
};

}

#endif
