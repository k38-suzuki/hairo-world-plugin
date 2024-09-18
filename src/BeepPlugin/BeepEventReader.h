/**
    @author Kenta Suzuki
*/

#ifndef CNOID_BEEP_PLUGIN_BEEP_EVENT_READER_H
#define CNOID_BEEP_PLUGIN_BEEP_EVENT_READER_H

#include <string>
#include <vector>

namespace cnoid {

class BeepEvent
{
public:
    BeepEvent();
    BeepEvent(const BeepEvent& org);

    std::string name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    std::string link1() const { return link1_; }
    std::string link2() const { return link2_; }
    void setLinkPairs(const std::string& link1, const std::string& link2) {
        link1_ = link1;
        link2_ = link2; }
    int frequency() const { return frequency_; }
    void setFrequency(const int& frequency) { frequency_ = frequency; }
    int length() const { return length_; }
    void setLength(const int& length) { length_ = length; }
    bool isEnabled() const { return is_enabled_; }
    void setEnabled(const bool is_enabled) { is_enabled_ = is_enabled; };

private:
    std::string name_;
    std::string link1_;
    std::string link2_;
    int frequency_;
    int length_;
    bool is_enabled_;
};

class BeepEventReader
{
public:
    BeepEventReader();
    virtual ~BeepEventReader();

    std::vector<BeepEvent> events();

    bool load(const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

}

#endif // CNOID_BEEP_PLUGIN_BEEP_EVENT_READER_H