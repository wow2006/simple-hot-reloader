#pragma once
// STL
#include <array>
// SDL2
#include <SDL2/SDL_render.h>

struct GameData {
  bool running = false;
  bool reload  = false;

  int step = 40;
  int width = 800;
  int height = 600;

  SDL_Color backgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
  SDL_Color forgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

  int currentX = 1;
  int currentY = 1;

  SDL_Texture* pApple = nullptr;

  std::array<SDL_Point, 5> borders {
    SDL_Point{0, 0},
    SDL_Point{width, 0},
    SDL_Point{width, height},
    SDL_Point{0, height},
    SDL_Point{0, 0}
  };

};

