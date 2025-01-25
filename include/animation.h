#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "atlas.h"
#include "camera.h"
#include "timer.h"
#include "vector2.h"

#include <SDL.h>

#include <functional>
#include <vector>

class Animation {
public:
  Animation() {
    this->timer.set_one_shot(false);
    this->timer.set_time_out([&]() {
      idx_frame++;
      if (idx_frame >= frame_list.size()) {
        idx_frame = is_loop ? 0 : frame_list.size() - 1;
        if (!is_loop && on_finished) {
          on_finished();
        }
      }
    });
  }

  ~Animation() = default;

  void reset() {
    this->timer.restart();
    idx_frame = 0;
  }

  void set_position(const Vector2 &position) { this->position = position; }

  void set_rotation(double angle) { this->angle = angle; }

  void set_center(const SDL_FPoint &center) { this->center = center; }

  void set_loop(bool is_loop) { this->is_loop = is_loop; }

  void set_interval(float interval) { this->timer.set_wait_time(interval); }

  void set_on_finished(std::function<void()> on_finished) {
    this->on_finished = on_finished;
  }

  void add_frame(SDL_Texture *texture, int num_h) {
    int width, height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    int width_frame = width / num_h;

    for (int i = 0; i < num_h; i++) {
      SDL_Rect src_rect = {i * width_frame, 0, width_frame, height};
      frame_list.emplace_back(texture, src_rect);
    }
  }

  void add_frame(Atlas *atlas) {
    for (int i = 0; i < atlas->get_size(); i++) {
      SDL_Texture *texture = atlas->get_texture(i);
      int width, height;
      SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

      SDL_Rect src_rect = {0, 0, width, height};
      frame_list.emplace_back(texture, src_rect);
    }
  }

  void on_update(float delta) { timer.on_update(delta); }

  void on_render(const Camera &camera) const {
    const Frame &frame = frame_list[idx_frame];
    // const Vector2 &pos_camera = camera.get_position();

    SDL_FRect dst_rect = {position.x - frame.src_rect.w / 2.0f,
                          position.y - frame.src_rect.h / 2.0f,
                          static_cast<float>(frame.src_rect.w),
                          static_cast<float>(frame.src_rect.h)};

    camera.render_texture(frame.texture, &frame.src_rect, &dst_rect, angle,
                          &center);
  }

private:
  struct Frame {
    SDL_Texture *texture = nullptr;
    SDL_Rect src_rect;

    Frame() = default;
    Frame(SDL_Texture *texture, const SDL_Rect &src_rect)
        : texture(texture), src_rect(src_rect) {}
    ~Frame() = default;
  };

private:
  Vector2 position;
  double angle = 0;
  SDL_FPoint center = {0, 0};

  Timer timer;
  bool is_loop = true;
  size_t idx_frame = 0;
  std::vector<Frame> frame_list;
  std::function<void()> on_finished;
};

#endif // _ANIMATION_H_