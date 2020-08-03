// STL
#include <array>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
// Internal
#include "library.hpp"

extern "C" {

void processInput(GameData *pData) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT: {
        pData->running = false;
        break;
      }
      case SDL_KEYDOWN: {
        switch(event.key.keysym.sym) {
          case SDLK_SPACE:
            pData->reload = true;
            break;
          case SDLK_d:
            pData->currentX += pData->step;
            break;
          case SDLK_a:
            pData->currentX -= pData->step;
            break;
          case SDLK_w:
            pData->currentY -= pData->step;
            break;
          case SDLK_s:
            pData->currentY += pData->step;
            break;
        }
      }
    }
  }
}

void update(GameData *pData) {}

void render(SDL_Renderer* pRenderer, const GameData *pData) {
  SDL_SetRenderDrawColor(pRenderer,
    pData->backgroundColor.r,
    pData->backgroundColor.g,
    pData->backgroundColor.b,
    pData->backgroundColor.a
  );

  SDL_RenderClear(pRenderer);
  {
    SDL_SetRenderDrawColor(
      pRenderer,
      pData->forgroundColor.r,
      pData->forgroundColor.g,
      pData->forgroundColor.b,
      pData->forgroundColor.a
    );

    SDL_RenderDrawLines(pRenderer, pData->borders.data(), pData->borders.size());

    for(uint32_t j = pData->step; j < pData->height; j += pData->step) {
      SDL_RenderDrawLine(
        pRenderer,
        0, j,
        pData->width, j
      );
    }

    for(uint32_t i = pData->step; i < pData->width; i += pData->step) {
      SDL_RenderDrawLine(
        pRenderer,
        i, 0,
        i, pData->height
      );
    }

    const SDL_Rect rect = { pData->currentX, pData->currentY, pData->step-1, pData->step-1 };
    SDL_RenderCopy(pRenderer, pData->pApple, nullptr, &rect);
  }
  SDL_RenderPresent(pRenderer);
}

} // extern "C"
