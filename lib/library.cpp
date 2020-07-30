// STL
#include <array>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>

constexpr auto cStep = 40;
constexpr auto cWidth = 800;
constexpr auto cHeight = 600;
constexpr auto cBackgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
constexpr auto cForgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

constexpr std::array<SDL_Point, 5> borders {
  SDL_Point{0, 0},
  SDL_Point{cWidth, 0},
  SDL_Point{cWidth, cHeight},
  SDL_Point{0, cHeight},
  SDL_Point{0, 0}
};

extern "C" {

void processInput(bool& bRunning, bool& bReload) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT: {
        bRunning = false;
        break;
      }

      case SDL_KEYDOWN: {
        switch(event.key.keysym.sym) {
          case SDLK_SPACE:
            bReload = true;
            break;
        }
      }

    }
  }
}

void update() {}

void render(SDL_Renderer* pRenderer) {
  SDL_SetRenderDrawColor(pRenderer,
    cBackgroundColor.r,
    cBackgroundColor.g,
    cBackgroundColor.b,
    cBackgroundColor.a
  );

  SDL_RenderClear(pRenderer);
  {
    SDL_SetRenderDrawColor(
      pRenderer,
      cForgroundColor.r,
      cForgroundColor.g,
      cForgroundColor.b,
      cForgroundColor.a
    );

    SDL_RenderDrawLines(pRenderer, borders.data(), borders.size());

    for(uint32_t j = cStep; j < cHeight; j += cStep) {
      SDL_RenderDrawLine(
        pRenderer,
        0, j,
        cWidth, j
      );
    }

    for(uint32_t i = cStep; i < cWidth; i += cStep) {
      SDL_RenderDrawLine(
        pRenderer,
        i, 0,
        i, cHeight
      );
    }
  }
  SDL_RenderPresent(pRenderer);
}

} // extern "C"

