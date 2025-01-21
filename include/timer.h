#ifndef _TIMER_H_
#define _TIMER_H_

#include <functional>

class Timer {
public:
  Timer() = default;
  ~Timer() = default;

  void restart() {
    this->pass_time = 0;
    this->shotted = false;
  }

  void set_wait_time(float wait_time) { this->wait_time = wait_time; }

  void set_one_shot(bool one_shot) { this->one_shot = one_shot; }

  void set_time_out(std::function<void()> on_timeout) {
    this->on_timeout = on_timeout;
  }

  void pause() { this->paused = true; }

  void resume() { this->paused = false; }

  void on_update(float delta) {
    if (paused)
      return;
    this->pass_time += delta;
    if (this->pass_time >= this->wait_time) {
      bool can_shot = (!one_shot || (one_shot && !shotted));
      this->shotted = true;
      if (can_shot && on_timeout)
        this->on_timeout();
      this->pass_time -= this->wait_time;
    }
  }

private:
  float pass_time = 0;
  float wait_time = 0;
  bool paused = false;
  bool shotted = false;
  bool one_shot = false;
  std::function<void()> on_timeout;
};

#endif