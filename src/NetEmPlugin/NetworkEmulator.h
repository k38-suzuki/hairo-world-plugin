/**
   \file
   \author Kenta Suzuki
*/

#ifndef CNOID_NETEMPLUGIN_NETWORKEMULATOR_H
#define CNOID_NETEMPLUGIN_NETWORKEMULATOR_H

#include <cnoid/ExtensionManager>
#include <cnoid/Referenced>
#include <vector>

namespace cnoid {

class NetworkEmulatorImpl;

class NetworkEmulator : public Referenced
{
public:
    NetworkEmulator();
    virtual ~NetworkEmulator();

    static void initializeClass(ExtensionManager* ext);

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
    NetworkEmulatorImpl* impl;
    friend class NetworkEmulatorImpl;
};

typedef ref_ptr<NetworkEmulator> NetworkEmulatorPtr;

}

#endif // CNOID_NETEMPLUGIN_NETWORKEMULATOR_H
