// STL
#include <array>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
// Internal
#include "library.hpp"

inline void drawBackground(SDL_Renderer *pRenderer, const GameData *pData) {
  SDL_SetRenderDrawColor(pRenderer, pData->backgroundColor.r,
                         pData->backgroundColor.g, pData->backgroundColor.b,
                         pData->backgroundColor.a);

  SDL_RenderClear(pRenderer);
}

inline void drawGrid(SDL_Renderer *pRenderer, const GameData *pData) {
  SDL_SetRenderDrawColor(pRenderer, pData->forgroundColor.r,
                         pData->forgroundColor.g, pData->forgroundColor.b,
                         pData->forgroundColor.a);

  //SDL_RenderDrawLines(pRenderer, pData->borders.data(), pData->borders.size());

  for (uint32_t j = pData->step; j < pData->height; j += pData->step) {
    SDL_RenderDrawLine(pRenderer, 0, j, pData->width, j);
  }

  for (uint32_t i = pData->step; i < pData->width; i += pData->step) {
    SDL_RenderDrawLine(pRenderer, i, 0, i, pData->height);
  }
}

inline void drawApples(SDL_Renderer *pRenderer, const GameData *pData) {
  for(const auto& rect : pData->apples) {
    auto renderRect = rect;
    const auto position = rect.y / pData->step;
    renderRect.y        = position * pData->step;
    SDL_RenderCopy(pRenderer, pData->pApple, nullptr, &renderRect);
  }
}

inline void drawSnake(SDL_Renderer *pRenderer, const GameData *pData) {
  const SDL_Rect rect = {pData->currentX, pData->currentY, pData->step - 1,
                         pData->step - 1};
  auto angle = 0.0;
  if (pData->direction == Direction::Up) {
    angle = 0;
  } else if (pData->direction == Direction::Down) {
    angle = 180;
  } else if (pData->direction == Direction::Left) {
    angle = -90;
  } else if (pData->direction == Direction::Right) {
    angle = 90;
  }

  SDL_RenderCopyEx(pRenderer, pData->pSnake, nullptr, &rect, angle, nullptr,
                   SDL_FLIP_NONE);
}

extern "C" {

void initialize(GameData *pData) { (void)pData; }

void clean(GameData *pData) { (void)pData; }

void processInput(GameData *pData) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      pData->running = false;
      break;
    }
    case SDL_KEYDOWN: {
      switch (event.key.keysym.sym) {
      case SDLK_SPACE:
        pData->reload = true;
        break;
      case SDLK_d:
        pData->currentX += pData->step;
        pData->direction = Direction::Right;
        break;
      case SDLK_a:
        pData->currentX -= pData->step;
        pData->direction = Direction::Left;
        break;
      case SDLK_w:
        pData->currentY -= pData->step;
        pData->direction = Direction::Up;
        break;
      case SDLK_s:
        pData->currentY += pData->step;
        pData->direction = Direction::Down;
        break;
      }
    }
    }
  }
}

void update(GameData *pData) {
  for(auto& rect : pData->apples) {
    rect.y += 1;
    if(rect.y > pData->height) {
      rect.y = 1;
    }
  }
}

void render(const GameData *pData) {
  glClearColor(0, 0, 0, 255);
  glClear(GL_COLOR_BUFFER_BIT);
  pData->gl.glBindBuffer(GL_ARRAY_BUFFER, pData->vbo);
  pData->gl.glDrawArrays(GL_LINE_STRIP, 0, 5);
  pData->gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
  //drawBackground(pRenderer, pData);
  //drawGrid(pRenderer, pData);
  //drawApples(pRenderer, pData);
  //drawSnake(pRenderer, pData);

  //SDL_RenderPresent(pRenderer);
}

} // extern "C"
