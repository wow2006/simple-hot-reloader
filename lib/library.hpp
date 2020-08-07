#pragma once
// STL
#include <array>
#include <vector>
// SDL2
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

enum class Direction : uint8_t { None, Up, Down, Left, Right };
enum class Movement : uint8_t { None, Forward, Backword, Left, Right };

struct GameData {
  bool running = false;
  bool reload  = false;

  int step   = 40;
  int width  = 800;
  int height = 600;

  TTF_Font *pFont = nullptr;
  Uint64 score = 0;

  SDL_Color backgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
  SDL_Color forgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

  SDL_Texture* pApple = nullptr;
  SDL_Texture* pSnake = nullptr;

  std::vector<std::pair<SDL_Texture*, SDL_Rect>> mTextures;

  Direction direction = Direction::Right;
  Movement  movement  = Movement::None;
  SDL_Point snakeHead = {1, 1};
  std::vector<SDL_Point> snakeBody;

  std::array<SDL_Point, 5> borders {
    SDL_Point{0, 0},
    SDL_Point{width, 0},
    SDL_Point{width, height},
    SDL_Point{0, height},
    SDL_Point{0, 0}
  };

  std::array<SDL_Point, 8> apples;

};

