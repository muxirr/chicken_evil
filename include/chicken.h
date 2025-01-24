#ifndef _CHICKEN_H_
#define _CHICKEN_H_

#include "animation.h"
#include "camera.h"
#include "vector2.h"

#include <SDL_mixer.h>

extern Atlas atlas_explosion;

extern Mix_Chunk *sound_explosion;

class Chicken {
public:
  Chicken() {
    this->animation_run.set_loop(true);
    this->animation_run.set_interval(0.1f);

    animation_explosion.set_loop(false);
    animation_explosion.set_interval(0.08f);
    animation_explosion.add_frame(&atlas_explosion);
    animation_explosion.set_on_finished([&]() { is_valid = false; });

    position = {40.0f + rand() % 1200, -50.0f};
  }

  ~Chicken() = default;

  const Vector2 &get_position() const { return this->position; }

  void on_update(float delta) {
    if (is_alive) {
      position.y += speed_run * delta;
    }

    current_animation = (is_alive ? &animation_run : &animation_explosion);
    current_animation->set_position(position);
    current_animation->on_update(delta);
  }

  void on_render(Camera &camera) { current_animation->on_render(camera); }

  void on_hurt() {
    is_alive = false;

    Mix_PlayChannel(-1, sound_explosion, 0);
  }

  void make_invalid() { this->is_valid = false; }

  bool check_valid() const { return this->is_valid; }

  bool can_remove() const { return !this->is_alive; }

protected:
  float speed_run = 10.0f;
  Animation animation_run;

private:
  Vector2 position;
  Animation animation_explosion;
  Animation *current_animation = nullptr;

  bool is_alive = true;
  bool is_valid = true;
};

#endif // _CHICKEN_H_