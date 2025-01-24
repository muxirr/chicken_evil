#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "timer.h"
#include "vector2.h"

#include <SDL.h>

class Camera {
public:
  Camera(SDL_Renderer *renderer) {
    this->renderer = renderer;
    this->timer_shake.set_one_shot(true);
    this->timer_shake.set_time_out([&]() {
      is_shaking = false;
      this->reset();
    });
  };

  ~Camera() = default;

  const Vector2 &get_position() const { return position; }

  void reset() { this->position = Vector2{0, 0}; }

  void on_update(float delta) {
    timer_shake.on_update(delta);
    if (is_shaking) {
      this->position = Vector2{(rand() % 100 - 50) / 50.0f * shaking_strength,
                               (rand() % 100 - 50) / 50.0f * shaking_strength};
    }
  }

  void shake(float strength, float duration) {
    is_shaking = true;
    shaking_strength = strength;

    timer_shake.set_wait_time(duration);
    timer_shake.restart();
  }

  void render_texture(SDL_Texture *texture, const SDL_Rect *src_rect,
                      const SDL_FRect *dst_rect, double angle,
                      const SDL_FPoint *center) const {
    SDL_FRect dst_rect_win = *dst_rect;
    dst_rect_win.x -= position.x;
    dst_rect_win.y -= position.y;

    SDL_RenderCopyExF(renderer, texture, src_rect, &dst_rect_win, angle, center,
                      SDL_RendererFlip::SDL_FLIP_NONE);
  }

private:
  Vector2 position;
  float shaking_strength;
  bool is_shaking = false;
  Timer timer_shake;
  SDL_Renderer *renderer = nullptr;
};

#endif // _CAMERA_H_
