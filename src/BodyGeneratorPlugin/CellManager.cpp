/**
   \file
   \author Kenta Suzuki
*/

#include "CellManager.h"
#include <fstream>
#include <sstream>
#include "TerrainBuilderDialog.h"

using namespace cnoid;
using namespace std;

namespace cnoid {

class CellManagerImpl
{
public:
    CellManagerImpl(CellManager* self);
    CellManager* self;

    double height[512][512];
    double cella[512][512][4];
    double cellb[512][512][4];
    double pointa[512][512][4][3];
    double pointb[512][512][4][3];

    double xsize;
    double ysize;
    int id;

    bool read(const string& filename);
};

}


CellManager::CellManager()
{
    impl = new CellManagerImpl(this);
}


CellManagerImpl::CellManagerImpl(CellManager* self)
    : self(self)
{

}


CellManager::~CellManager()
{
    delete impl;
}


bool CellManager::read(const string& filename)
{
    impl->read(filename);
    return true;
}


bool CellManagerImpl::read(const string& filename)
{
    //set height
    ifstream ifs(filename.c_str());
    if(!ifs) {
        return false;
    }

    string str;
    int row = 0;
    int clm = 0;

    while(getline(ifs, str)) {
        string token;
        istringstream stream(str);
        row = 0;
        while(getline(stream, token, ',')) {
            height[clm][row] = atof(token.c_str());
            if(height[clm][row] < 0.0) {
                height[clm][row] = 0.0;
            }
            row++;
        }
        clm++;
    }
    xsize = row;
    ysize = clm;

    id = 0;

    //set cella
    for(int k = 0; k < ysize; k++) {
        for(int j = 0; j < xsize; j++) {
            for(int i = 0; i < 4; i++) {
                cella[k][j][i] = id++;
            }
        }
    }

    //set cellb
    for(int k = 0; k < ysize; k++) {
        for(int j = 0; j < xsize; j++) {
            for(int i = 0; i < 4; i++) {
                cellb[k][j][i] = id++;
            }
        }
    }

    //set pointa, pointb
    double scale = 0.1;

    TerrainBuilderDialog* widget = TerrainBuilderDialog::instance();
    if(widget) {
        scale *= widget->scale();
    }
    for(int j = 0; j < ysize; j++) {
        for(int i = 0; i < xsize; i++) {
            pointa[j][i][0][0] = scale * (i - xsize / 2);
            pointa[j][i][0][1] = scale * (j - ysize / 2) * -1;
            pointa[j][i][0][2] = scale * height[j][i];
            pointa[j][i][1][0] = scale * (i - xsize / 2);
            pointa[j][i][1][1] = scale * (j + 1 - ysize / 2) * -1;
            pointa[j][i][1][2] = scale * height[j][i];
            pointa[j][i][2][0] = scale * (i + 1 - xsize / 2);
            pointa[j][i][2][1] = scale * (j + 1 - ysize / 2) * -1;
            pointa[j][i][2][2] = scale * height[j][i];
            pointa[j][i][3][0] = scale * (i + 1 - xsize / 2);
            pointa[j][i][3][1] = scale * (j - ysize / 2) * -1;
            pointa[j][i][3][2] = scale * height[j][i];

            pointb[j][i][0][0] = scale * (i - xsize / 2);
            pointb[j][i][0][1] = scale * (j - ysize / 2) * -1;
            pointb[j][i][0][2] = scale * 0.0;
            pointb[j][i][1][0] = scale * (i - xsize / 2);
            pointb[j][i][1][1] = scale * (j + 1 - ysize / 2) * -1;
            pointb[j][i][1][2] = scale * 0.0;
            pointb[j][i][2][0] = scale * (i + 1 - xsize / 2);
            pointb[j][i][2][1] = scale * (j + 1 - ysize / 2) * -1;
            pointb[j][i][2][2] = scale * 0.0;
            pointb[j][i][3][0] = scale * (i + 1 - xsize / 2);
            pointb[j][i][3][1] = scale * (j - ysize / 2) * -1;
            pointb[j][i][3][2] = scale * 0.0;
        }
    }

    return true;
}


int CellManager::xsize() const
{
    return impl->xsize;
}


int CellManager::ysize() const
{
    return impl->ysize;
}


double CellManager::pointax(const int& x, const int& y, const int& index) const
{
    return impl->pointa[y][x][index][0];
}


double CellManager::pointay(const int& x, const int& y, const int& index) const
{
    return impl->pointa[y][x][index][1];
}


double CellManager::pointaz(const int& x, const int& y, const int& index) const
{
    return impl->pointa[y][x][index][2];
}


double CellManager::pointbx(const int& x, const int& y, const int& index) const
{
    return impl->pointb[y][x][index][0];
}


double CellManager::pointby(const int& x, const int& y, const int& index) const
{
    return impl->pointb[y][x][index][1];
}


double CellManager::pointbz(const int& x, const int& y, const int& index) const
{
    return impl->pointb[y][x][index][2];
}


int CellManager::id(const int& x, const int& y, const int& index, const int& sindex) const
{
    if(sindex == 0) {
        return impl->cella[y][x][index];
    } else {
        return impl->cellb[y][x][index];
    }
}
