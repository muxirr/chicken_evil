// 使用原来的main函数;
#define SDL_MAIN_HANDLED
#define RES_PATH "./assets/" // 资源路径头;
// #define DEBUG                // 调试模式无敌;
#ifdef DEBUG
#define RES_PATH "../../assets/" // 资源路径头;
#endif

#include "atlas.h"
#include "bullet.h"
#include "camera.h"
#include "chicken_fast.h"
#include "chicken_medium.h"
#include "chicken_slow.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

// 全局变量;
Camera *camera = nullptr;

float fps_delta = 1.0f;
std::chrono::duration<float> delta;
const float FPS_DURATION = 1.0f; // FPS刷新间隔;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
bool is_quit = false; // 是否退出游戏

SDL_Texture *tex_heart = nullptr;       // 生命值纹理;
SDL_Texture *tex_bullet = nullptr;      // 子弹纹理;
SDL_Texture *tex_battery = nullptr;     // 炮台基座纹理;
SDL_Texture *tex_crosshair = nullptr;   // 光标准心纹理;
SDL_Texture *tex_background = nullptr;  // 背景纹理;
SDL_Texture *tex_barrel_idle = nullptr; // 炮管默认状态纹理;

Atlas atlas_barrel_fire;    // 炮管开火动画图集;
Atlas atlas_chicken_fast;   // 快速僵尸鸡动画图集;
Atlas atlas_chicken_medium; // 中速僵尸鸡动画图集;
Atlas atlas_chicken_slow;   // 慢速僵尸鸡动画图集;
Atlas atlas_explosion;      // 僵尸鸡死亡爆炸动画图集;

Mix_Music *music_bgm = nullptr;       // 背景音乐;
Mix_Music *music_loss = nullptr;      // 游戏失败音乐;
Mix_Chunk *sound_hurt = nullptr;      // 生命值降低音效;
Mix_Chunk *sound_fire_1 = nullptr;    // 开火音效;
Mix_Chunk *sound_fire_2 = nullptr;    // 开火音效2;
Mix_Chunk *sound_fire_3 = nullptr;    //  开火音效3;
Mix_Chunk *sound_explosion = nullptr; // 僵尸鸡死亡爆炸音效;

TTF_Font *font = nullptr; // 字体;

int hp = 10;                         // 生命值;
int score = 0;                       // 游戏得分;
const int FPS = 144;                 // 帧率;
std::vector<Bullet> bullet_list;     // 子弹列表;
std::vector<Chicken *> chicken_list; // 僵尸鸡列表;

int num_per_gen = 2;              // 每次生成僵尸鸡数量;
Timer timer_generate;             // 僵尸鸡生成定时器;
Timer timer_increase_num_pre_gen; // 增加每次生成数量定时器;
Timer timer_fps;                  // 帧率计算定时器;

Vector2 pos_crosshair;                     // 光标准心位置;
double angle_barrel = 0;                   // 炮管旋转角度;
const Vector2 pos_battery = {640, 600};    // 炮台基座中心位置;
const Vector2 pos_barrel = {592, 585};     // 炮管无旋转默认位置;
const SDL_FPoint center_barrel = {45, 25}; // 炮管旋转中心点坐标;

bool is_cool_down = true;        // 是否冷却结束;
bool is_fire_key_down = false;   // 是否按下开火键;
Animation animation_barrel_fire; // 炮管开火动画;

void load_resources();                // 加载游戏资源;
void unload_resources();              // 卸载游戏资源;
void init();                          // 游戏程序初始化;
void deinit();                        // 游戏程序反初始化;
void on_update(float delta);          // 游戏逻辑更新;
void on_render(const Camera &camera); // 游戏渲染;
void main_loop();                     // 主循环;
void check_fail();                    // 检测游戏失败;

int WinMain(int argc, char **argv) {

  init();
  main_loop();
  deinit();

  return 0;
}

void init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  Mix_Init(MIX_INIT_MP3);
  TTF_Init();

  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  Mix_AllocateChannels(32);
  window =
      SDL_CreateWindow("生化危鸡", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_ShowCursor(SDL_DISABLE);

  load_resources();

  camera = new Camera(renderer);

  timer_generate.set_one_shot(false);
  timer_generate.set_wait_time(1.5f);
  timer_generate.set_time_out([&] {
    for (int i = 0; i < num_per_gen; i++) {
      int val = rand() % 100;
      Chicken *chicken = nullptr;
      if (val < 50) {
        chicken = new ChickenSlow(); // 50%的概率生成慢速僵尸鸡;
      } else if (val < 80) {
        chicken = new ChickenMedium(); // 30%的概率生成中速僵尸鸡;
      } else {
        chicken = new ChickenFast(); // 20%的概率生成快速僵尸鸡;
      }
      chicken_list.push_back(chicken);
    }
  });

  timer_increase_num_pre_gen.set_one_shot(false);
  timer_increase_num_pre_gen.set_wait_time(8.0f);
  timer_increase_num_pre_gen.set_time_out([&] { num_per_gen++; });

  timer_fps.set_one_shot(false);
  timer_fps.set_wait_time(FPS_DURATION);
  timer_fps.set_time_out([&] { fps_delta = delta.count(); });

  animation_barrel_fire.set_loop(false);
  animation_barrel_fire.set_interval(0.04f);
  animation_barrel_fire.set_center(center_barrel);
  animation_barrel_fire.add_frame(&atlas_barrel_fire);
  animation_barrel_fire.set_on_finished([&] { is_cool_down = true; });
  animation_barrel_fire.set_position({718, 610});

  Mix_PlayMusic(music_bgm, -1);
}

void deinit() {
  delete camera;

  unload_resources();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
}

void load_resources() {
  tex_heart = IMG_LoadTexture(renderer, (RES_PATH "image/heart.png"));
  tex_bullet = IMG_LoadTexture(renderer, (RES_PATH "image/bullet.png"));
  tex_battery = IMG_LoadTexture(renderer, (RES_PATH "image/battery.png"));
  tex_crosshair = IMG_LoadTexture(renderer, (RES_PATH "image/crosshair.png"));
  tex_background = IMG_LoadTexture(renderer, (RES_PATH "image/background.png"));
  tex_barrel_idle =
      IMG_LoadTexture(renderer, (RES_PATH "image/barrel_idle.png"));

  atlas_barrel_fire.load(renderer, (RES_PATH "image/barrel_fire_%d.png"), 3);
  atlas_chicken_fast.load(renderer, (RES_PATH "image/chicken_fast_%d.png"), 4);
  atlas_chicken_medium.load(renderer, (RES_PATH "image/chicken_medium_%d.png"),
                            6);
  atlas_chicken_slow.load(renderer, (RES_PATH "image/chicken_slow_%d.png"), 8);
  atlas_explosion.load(renderer, (RES_PATH "image/explosion_%d.png"), 5);

  music_bgm = Mix_LoadMUS((RES_PATH "audio/bgm.mp3"));
  music_loss = Mix_LoadMUS((RES_PATH "audio/loss.mp3"));

  sound_hurt = Mix_LoadWAV((RES_PATH "audio/hurt.wav"));
  sound_fire_1 = Mix_LoadWAV((RES_PATH "audio/fire_1.wav"));
  sound_fire_2 = Mix_LoadWAV((RES_PATH "audio/fire_2.wav"));
  sound_fire_3 = Mix_LoadWAV((RES_PATH "audio/fire_3.wav"));
  sound_explosion = Mix_LoadWAV((RES_PATH "audio/explosion.wav"));

  font = TTF_OpenFont((RES_PATH "IPix.ttf"), 28);
}

void unload_resources() {
  SDL_DestroyTexture(tex_heart);
  SDL_DestroyTexture(tex_bullet);
  SDL_DestroyTexture(tex_battery);
  SDL_DestroyTexture(tex_crosshair);
  SDL_DestroyTexture(tex_background);
  SDL_DestroyTexture(tex_barrel_idle);

  Mix_FreeMusic(music_bgm);
  Mix_FreeMusic(music_loss);

  Mix_FreeChunk(sound_hurt);
  Mix_FreeChunk(sound_fire_1);
  Mix_FreeChunk(sound_fire_2);
  Mix_FreeChunk(sound_fire_3);
  Mix_FreeChunk(sound_explosion);

  TTF_CloseFont(font);
}

void main_loop() {
  using namespace std::chrono;

  SDL_Event event;

  const nanoseconds frame_duartion(1000000000 / FPS);
  steady_clock::time_point last_tick = steady_clock::now();
  while (!is_quit) {
    // 处理消息;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          is_quit = true;
        }
        break;
      }
      case SDL_QUIT: {
        is_quit = true;
        break;
      }
      case SDL_MOUSEMOTION: {
        pos_crosshair = {static_cast<float>(event.motion.x),
                         static_cast<float>(event.motion.y)};
        Vector2 direction = pos_crosshair - pos_battery;
        angle_barrel = std::atan2(direction.y, direction.x) * 180 / M_PI;
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        is_fire_key_down = true;
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        is_fire_key_down = false;
        break;
      }
      }
    }

    steady_clock::time_point frame_start = steady_clock::now();
    delta = duration<float>(frame_start - last_tick);

    on_update(delta.count());

    SDL_RenderClear(renderer);
    on_render(*camera);
    SDL_RenderPresent(renderer);

    check_fail();

    last_tick = frame_start;

    nanoseconds sleep_duration =
        frame_duartion -
        duration_cast<nanoseconds>(steady_clock::now() - frame_start);
    if (sleep_duration > nanoseconds(0)) {
      std::this_thread::sleep_for(sleep_duration);
    }
  }
}

void on_update(float delta) {
  timer_generate.on_update(delta);
  timer_increase_num_pre_gen.on_update(delta);
  timer_fps.on_update(delta);

  // 更新子弹;
  for (Bullet &bullet : bullet_list) {
    bullet.on_update(delta);
  }

  // 更新僵尸鸡并检测碰撞;
  for (Chicken *chicken : chicken_list) {
    chicken->on_update(delta);

    for (auto &bullet : bullet_list) {
      if (!chicken->check_alive() || bullet.can_remove()) {
        continue;
      }

      const Vector2 &pos_chicken = chicken->get_position();
      const Vector2 &pos_bullet = bullet.get_position();
      static const Vector2 size_chicken = {30, 40};
      if (pos_bullet.x >= pos_chicken.x - size_chicken.x / 2 &&
          pos_bullet.x <= pos_chicken.x + size_chicken.x / 2 &&
          pos_bullet.y >= pos_chicken.y - size_chicken.y / 2 &&
          pos_bullet.y <= pos_chicken.y + size_chicken.y / 2) {
        score++;
        bullet.on_hit();
        chicken->on_hurt();
      }
    }

    if (!chicken->check_alive()) {
      continue;
    }

#ifndef DEBUG
    // 僵尸鸡与炮台碰撞  x:550-730 y:600 - 125/2 = 487.5;
    if (chicken->get_position().y >= pos_battery.y - (float)125 / 2 &&
        chicken->get_position().x <= 730 && chicken->get_position().x >= 550) {
      chicken->make_invalid();
      Mix_PlayChannel(-1, sound_hurt, 0);
      hp--;
    }
    // 漏网之鸡减少生命值;
    if (chicken->get_position().y >= 720) {
      chicken->make_invalid();
      Mix_PlayChannel(-1, sound_hurt, 0);
      hp--;
    }
#endif
  }

  // 移除无效子弹;
  bullet_list.erase(
      std::remove_if(bullet_list.begin(), bullet_list.end(),
                     [](const Bullet &bullet) { return bullet.can_remove(); }),
      bullet_list.end());

  // 移除无效僵尸鸡;
  chicken_list.erase(std::remove_if(chicken_list.begin(), chicken_list.end(),
                                    [](Chicken *chicken) {
                                      bool can_remove = chicken->can_remove();
                                      if (can_remove) {
                                        delete chicken;
                                      }
                                      return can_remove;
                                    }),
                     chicken_list.end());

  // 对场景中的僵尸鸡按竖直坐标位置排序;
  std::sort(chicken_list.begin(), chicken_list.end(),
            [](const Chicken *a, const Chicken *b) {
              return a->get_position().y < b->get_position().y;
            });

  // 处理正在开火逻辑;
  if (!is_cool_down) {
    camera->shake(3.0f, 0.1f);
    animation_barrel_fire.on_update(delta);
  }

  // 处理开火瞬间逻辑;
  if (is_fire_key_down && is_cool_down) {
    is_cool_down = false;
    animation_barrel_fire.reset();

    static const float length_barrel = 105.0f;           // 炮管长度;
    static const Vector2 pos_barrel_center = {640, 640}; // 炮管锚点中心位置;

    bullet_list.emplace_back(angle_barrel); // 添加子弹;
    Bullet &bullet = bullet_list.back();
    double angle_bullet = (angle_barrel + rand() % 30 - 15);
    double radians = angle_bullet * M_PI / 180;
    Vector2 direction = {static_cast<float>(std::cos(radians)),
                         static_cast<float>(std::sin(radians))};
    bullet.set_position(pos_barrel_center + direction * length_barrel);

    switch (rand() % 3) {
    case 0:
      Mix_PlayChannel(-1, sound_fire_1, 0);
      break;
    case 1:
      Mix_PlayChannel(-1, sound_fire_2, 0);
      break;
    case 2:
      Mix_PlayChannel(-1, sound_fire_3, 0);
      break;
    }
  }

  // 更新摄像机位置;
  camera->on_update(delta);
}

void on_render(const Camera &camera) {
  // 渲染背景;
  {
    int width_bg, height_bg;
    SDL_QueryTexture(tex_background, nullptr, nullptr, &width_bg, &height_bg);
    const SDL_FRect rect_bg = {static_cast<float>(1280 - width_bg) / 2.0f,
                               static_cast<float>(720 - height_bg) / 2.0f,
                               static_cast<float>(width_bg),
                               static_cast<float>(height_bg)};
    camera.render_texture(tex_background, nullptr, &rect_bg, 0, nullptr);
  }
  // 绘制僵尸鸡;
  for (Chicken *chicken : chicken_list) {
    chicken->on_render(camera);
  }
  // 绘制子弹;
  for (const Bullet &bullet : bullet_list) {
    bullet.on_render(camera);
  }

  // 绘制炮台;
  {
    // 绘制炮台基座;
    int width_battery, height_battery;
    SDL_QueryTexture(tex_battery, nullptr, nullptr, &width_battery,
                     &height_battery);
    const SDL_FRect rect_battery = {pos_battery.x - width_battery / 2.0f,
                                    pos_battery.y - height_battery / 2.0f,
                                    static_cast<float>(width_battery),
                                    static_cast<float>(height_battery)};

    camera.render_texture(tex_battery, nullptr, &rect_battery, 0, nullptr);

    // 绘制炮管;
    int width_barrel, height_barrel;
    SDL_QueryTexture(tex_barrel_idle, nullptr, nullptr, &width_barrel,
                     &height_barrel);
    const SDL_FRect rect_barrel = {pos_barrel.x, pos_barrel.y,
                                   static_cast<float>(width_barrel),
                                   static_cast<float>(height_barrel)};
    if (is_cool_down) {
      camera.render_texture(tex_barrel_idle, nullptr, &rect_barrel,
                            angle_barrel, &center_barrel);
    } else {
      animation_barrel_fire.set_rotation(angle_barrel);
      animation_barrel_fire.on_render(camera);
    }
  }

  // 绘制生命值;
  {
    int width_heart, height_heart;
    SDL_QueryTexture(tex_heart, nullptr, nullptr, &width_heart, &height_heart);

    for (int i = 0; i < hp; i++) {
      const SDL_Rect rect_heart = {15 + (width_heart + 10) * i, 15, width_heart,
                                   height_heart};
      SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_heart);
    }
  }

  // 绘制游戏得分;
  {
    std::string str_score = "Score: " + std::to_string(score);
    SDL_Surface *suf_score_bg =
        TTF_RenderUTF8_Blended(font, str_score.c_str(), {55, 55, 55, 255});
    SDL_Surface *suf_score_fg =
        TTF_RenderUTF8_Blended(font, str_score.c_str(), {255, 255, 255, 255});
    SDL_Texture *tex_score_bg =
        SDL_CreateTextureFromSurface(renderer, suf_score_bg);
    SDL_Texture *tex_score_fg =
        SDL_CreateTextureFromSurface(renderer, suf_score_fg);

    SDL_Rect rect_dst_score = {1280 - suf_score_bg->w - 15, 15, suf_score_bg->w,
                               suf_score_bg->h};
    SDL_RenderCopy(renderer, tex_score_bg, nullptr, &rect_dst_score);
    rect_dst_score.x -= 2;
    rect_dst_score.y -= 2;
    SDL_RenderCopy(renderer, tex_score_fg, nullptr, &rect_dst_score);

    SDL_DestroyTexture(tex_score_bg);
    SDL_DestroyTexture(tex_score_fg);
    SDL_FreeSurface(suf_score_bg);
    SDL_FreeSurface(suf_score_fg);
  }

  // 绘制准心;
  {
    int width_crosshair, height_crosshair;
    SDL_QueryTexture(tex_crosshair, nullptr, nullptr, &width_crosshair,
                     &height_crosshair);
    const SDL_FRect rect_crosshair = {pos_crosshair.x - width_crosshair / 2.0f,
                                      pos_crosshair.y - height_crosshair / 2.0f,
                                      static_cast<float>(width_crosshair),
                                      static_cast<float>(height_crosshair)};
    camera.render_texture(tex_crosshair, nullptr, &rect_crosshair, 0, nullptr);
  }

  // 绘制fps;
  char fps_str[20];
  sprintf(fps_str, "FPS: %.2f", 1.0f / fps_delta);
  SDL_Surface *suf_fps_bg =
      TTF_RenderUTF8_Blended(font, fps_str, {55, 55, 55, 255});
  SDL_Surface *suf_fps_fg =
      TTF_RenderUTF8_Blended(font, fps_str, {255, 255, 255, 255});
  SDL_Texture *tex_fps_bg = SDL_CreateTextureFromSurface(renderer, suf_fps_bg);
  SDL_Texture *tex_fps_fg = SDL_CreateTextureFromSurface(renderer, suf_fps_fg);

  SDL_Rect rect_dst_fps = {15, 720 - suf_fps_bg->h - 15, suf_fps_bg->w,
                           suf_fps_bg->h};
  SDL_RenderCopy(renderer, tex_fps_bg, nullptr, &rect_dst_fps);
  rect_dst_fps.x -= 2;
  rect_dst_fps.y -= 2;
  SDL_RenderCopy(renderer, tex_fps_fg, nullptr, &rect_dst_fps);

  SDL_DestroyTexture(tex_fps_bg);
  SDL_DestroyTexture(tex_fps_fg);
  SDL_FreeSurface(suf_fps_bg);
  SDL_FreeSurface(suf_fps_fg);
}

void check_fail() {
  // 检测游戏失败或退出;
  if (hp <= 0 || is_quit) {
    is_quit = true;
    Mix_HaltMusic();
    Mix_PlayMusic(music_loss, -1);
    std::string msg = "游戏结束，得分：" + std::to_string(score);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "游戏结束",
                             msg.c_str(), window);
  }
}