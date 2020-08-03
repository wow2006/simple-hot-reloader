#pragma once
// STL
#include <array>
// SDL2
#include <SDL2/SDL_render.h>

enum Direction : uint8_t { Up, Down, Left, Right };


struct GameData {
  bool running = false;
  bool reload  = false;

  int step   = 40;
  int width  = 800;
  int height = 600;

  SDL_Color backgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
  SDL_Color forgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

  int currentX = 1;
  int currentY = 1;
  Direction direction = Direction::Up;

  SDL_Texture* pApple = nullptr;
  SDL_Texture* pSnake = nullptr;

  std::array<SDL_Point, 5> borders {
    SDL_Point{0, 0},
    SDL_Point{width, 0},
    SDL_Point{width, height},
    SDL_Point{0, height},
    SDL_Point{0, 0}
  };

  std::array<SDL_Rect, 8> apples;

};

