/**
   @author Kenta Suzuki
*/

#ifndef CNOID_NETEM_PLUGIN_NETEM_H
#define CNOID_NETEM_PLUGIN_NETEM_H

#include <cnoid/Referenced>
#include <vector>

namespace cnoid {

class NetEmImpl;

class NetEm : public Referenced
{
public:
    NetEm();
    virtual ~NetEm();

    std::vector<std::string>& interfaces() const;

    void start(const int& interfaceID = 0, const int& ifbdeviceID = 0);
    void update();
    void stop();

    void setDelay(const int& id, const double& delay);
    void setRate(const int& id, const double& rate);
    void setLoss(const int& id, const double& loss);
    void setSourceIP(const std::string& sourceIP);
    void setDestinationIP(const std::string& destinationIP);

private:
    NetEmImpl* impl;
    friend class NetEmImpl;
};

typedef ref_ptr<NetEm> NetEmPtr;

}

#endif
