#ifndef __WORLD_H_INCLUDED
#define __WORLD_H_INCLUDED

#include <memory>

#include "robot_pose.h"

template <typename SensorData>
class SensorDataObserver {
public:
  virtual void handle_sensor_data(SensorData &) = 0;
};

class WorldPoseObserver {
public:
  virtual void on_pose_update(const RobotPose &rs) = 0;
};

template<typename MapType>
class WorldMapObserver {
public:
  virtual void on_map_update(const MapType &map) = 0;
};

template<typename MapType>
class WorldObservable {
public:
  void subscribe_map(std::shared_ptr<WorldMapObserver<MapType>> obs) {
    _world_map_observers.push_back(obs);
  }

  void subscribe_pose(std::shared_ptr<WorldPoseObserver> obs) {
    _world_pose_observers.push_back(obs);
  }

protected: // methods

#define NOTIFY_EACH_WOBSERVER(var)                     \
  for (auto &raw_obs : _world_##var##_observers) {     \
    auto obs_ptr = raw_obs.lock();                     \
    if (obs_ptr) {                                     \
      obs_ptr->on_##var##_update(var);                 \
    }                                                  \
  }

  void notify_with_pose(const RobotPose &pose) {
    NOTIFY_EACH_WOBSERVER(pose);
  }

  void notify_with_map(const MapType &map) {
    NOTIFY_EACH_WOBSERVER(map);
  }

#undef NOTIFY_EACH_WOBSERVER

private:
  std::vector<std::weak_ptr<WorldMapObserver<MapType>>> _world_map_observers;
  std::vector<std::weak_ptr<WorldPoseObserver>> _world_pose_observers;
};

// TODO: try to simplify template params
template <typename ObservationType, typename MapType>
class World : public WorldObservable<MapType>
            , public SensorDataObserver<ObservationType> {
public:
  // data-in
  virtual void update_robot_pose(const RobotPoseDelta& delta) {
    _pose += delta;
  }

  // data-out
  virtual const World<ObservationType, MapType>& world() const { return *this; }
  virtual const RobotPose& pose() const { return _pose; }
  virtual const MapType& map() const = 0;
protected:
  virtual void handle_observation(ObservationType&) = 0;
  virtual ~World() = default;
private:
  RobotPose _pose;
};

#endif
