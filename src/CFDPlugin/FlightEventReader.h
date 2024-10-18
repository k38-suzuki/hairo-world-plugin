/**
    @author Kenta Suzuki
*/

#ifndef CNOID_CFD_PLUGIN_FLIGHT_EVENT_READER_H
#define CNOID_CFD_PLUGIN_FLIGHT_EVENT_READER_H

#include <string>
#include <vector>

namespace cnoid {

class FlightEvent
{
public:
    FlightEvent();
    FlightEvent(const FlightEvent& org);

    std::string name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    double mass() const { return mass_; }
    void setMass(const double& mass) { mass_ = mass; }
    double duration() const { return duration_; }
    void setDuration(const double& duration) { duration_ = duration; }

private:
    std::string name_;
    double mass_;
    double duration_;
};

class FlightEventReader
{
public:
    FlightEventReader();
    virtual ~FlightEventReader();

    std::vector<FlightEvent> events();

    bool load(const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_CFD_PLUGIN_FLIGHT_EVENT_READER_H