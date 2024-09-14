/**
    @author Kenta Suzuki
 */

 #ifndef CNOID_VFX_PLUGIN_VFX_EVENT_READER_H
 #define CNOID_VFX_PLUGIN_VFX_EVENT_READER_H

#include <cnoid/CustomEffects>
#include <vector>

namespace cnoid {

class VFXEvent : public VFXEffects
{
public:
    VFXEvent();
    VFXEvent(const VFXEvent& org);

    std::string name() { return name_; }
    void setName(const std::string& name) { name_ = name; }
    double beginTime() { return begin_time_; }
    void setBeginTime(const double& begin_time) { begin_time_ = begin_time; }
    double endTime() { return end_time_; }
    void setEndTime(const double& end_time) { end_time_ = end_time; }
    double duration() { return duration_; }
    void setDuration(const double& duration) { duration_ = duration; }
    Vector2 cycle() { return cycle_; }
    void setCycle(const Vector2& cycle) { cycle_ = cycle; }
    std::vector<std::string> targetColliders() { return target_colliders_; }
    void addTargetCollider(const std::string& target_collider) { target_colliders_.push_back(target_collider); }
    void clearTargetCollider() { target_colliders_.clear(); }
    bool isEnabled() { return is_enabled_; }
    void setEnabled(const bool is_enabled) { is_enabled_ = is_enabled; };

private:
    std::string name_;
    double begin_time_;
    double end_time_;
    double duration_;
    Vector2 cycle_;
    std::vector<std::string> target_colliders_;
    bool is_enabled_;
};

class VFXEventReader
{
public:
    VFXEventReader();
    virtual ~VFXEventReader();

    std::vector<VFXEvent> events();

    bool load(const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

}

 #endif // CNOID_VFX_PLUGIN_VFX_EVENT_READER_H