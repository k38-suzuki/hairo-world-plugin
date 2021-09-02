/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_BODYGENERATORPLUGIN_CELLMANAGER_H
#define CNOID_BODYGENERATORPLUGIN_CELLMANAGER_H

#include <string>

namespace cnoid {

class CellManagerImpl;

class CellManager
{
public:
    CellManager();
    virtual ~CellManager();

    bool read(const std::string& filename);
    int xsize() const;
    int ysize() const;
    double pointax(const int& x, const int& y, const int& index) const;
    double pointay(const int& x, const int& y, const int& index) const;
    double pointaz(const int& x, const int& y, const int& index) const;
    double pointbx(const int& x, const int& y, const int& index) const;
    double pointby(const int& x, const int& y, const int& index) const;
    double pointbz(const int& x, const int& y, const int& index) const;
    int id(const int& x, const int& y, const int& index, const int& sindex) const;

private:
    CellManagerImpl *impl;
    friend class CellManagerImpl;
};

}

#endif // CNOID_BODYGENERATORPLUGIN_CELLMANAGER_H
