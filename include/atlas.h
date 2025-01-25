#ifndef _ALTAS_H_
#define _ALTAS_H_

#include <SDL.h>
#include <SDL_image.h>

#include <vector>

class Atlas {
public:
  Atlas() = default;

  ~Atlas() {
    for (auto texture : tex_list) {
      if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
      }
    }
  }

  void load(SDL_Renderer *renderer, const char *path_template, int num) {
    this->tex_list.clear();
    this->tex_list.resize(num);

    char path[256];

    for (int i = 0; i < num; i++) {
      sprintf_s(path, path_template, i + 1);
      SDL_Texture *texture = IMG_LoadTexture(renderer, path);
      tex_list[i] = std::move(texture); // resize后不要插入，直接赋值!!!
    }
  }

  void clear() { this->tex_list.clear(); }

  int get_size() { return static_cast<int>(this->tex_list.size()); }

  SDL_Texture *get_texture(int index) {
    if (index < 0 || index >= tex_list.size())
      return nullptr;
    return this->tex_list[index];
  }
  void add_texture(SDL_Texture *texture) { this->tex_list.push_back(texture); }

private:
  std::vector<SDL_Texture *> tex_list;
};

#endif // _ALTAS_H_