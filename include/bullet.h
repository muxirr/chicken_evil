#ifndef _BULLET_H_
#define _BULLET_H_

#include "camera.h"
#include "vector2.h"

#include <SDL.h>

extern SDL_Texture *tex_bullet;

class Bullet {
public:
  Bullet(double angle) : angle(angle) {
    double radians = angle * M_PI / 180.0;
    velocity = Vector2{static_cast<float>(std::cos(radians) * speed),
                       static_cast<float>(std::sin(radians) * speed)};
  }

  ~Bullet() = default;

  void set_position(const Vector2 &position) { this->position = position; }

  const Vector2 &get_position() const { return this->position; }

  void on_update(float delta) {
    this->position += this->velocity * delta;
    if (this->position.x <= 0 || this->position.x >= 1280 ||
        this->position.y <= 0 || this->position.y >= 720) {
      this->is_valid = false;
    }
  }

  void on_render(const Camera &camera) const {
    const SDL_FRect dst_rect = {this->position.x - 4, this->position.y - 2, 8,
                                4};
    camera.render_texture(tex_bullet, nullptr, &dst_rect, angle, nullptr);
  }

  void on_hit() { this->is_valid = false; }

  bool can_remove() const { return !this->is_valid; }

private:
  double angle = 0;
  Vector2 position;
  Vector2 velocity;
  bool is_valid = true;
  float speed = 800.0f;
};

#endif // _BULLET_H_