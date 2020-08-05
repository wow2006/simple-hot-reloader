// STL
#include <array>
#include <random>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
// Internal
#include "library.hpp"

extern "C" {

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
        pData->direction = Direction::Right;
        pData->movement = Movement::Right;
        break;
      case SDLK_a:
        pData->direction = Direction::Left;
        pData->movement = Movement::Left;
        break;
      case SDLK_w:
        pData->direction = Direction::Up;
        pData->movement = Movement::Forward;
        break;
      case SDLK_s:
        pData->direction = Direction::Down;
        pData->movement = Movement::Backword;
        break;
      case SDLK_q:
        pData->snakeBody.push_back(pData->snakeHead);
        break;
      case SDLK_e:
        //pData->snakeHead = {1, 1};
        pData->snakeBody.clear();
        pData->snakeBody.reserve(8);
        break;
      }
    }
    }
  }
}

void update(GameData *pData) {
  if(pData->movement != Movement::None) {
    auto lastRect = pData->snakeHead;
    if(pData->movement == Movement::Right) {
      pData->snakeHead.x += pData->step;
    } else if(pData->movement == Movement::Left) {
      pData->snakeHead.x -= pData->step;
    } else if(pData->movement == Movement::Forward) {
      pData->snakeHead.y -= pData->step;
    } else if(pData->movement == Movement::Backword) {
      pData->snakeHead.y += pData->step;
    }

    pData->movement = Movement::None;

    for(auto& rect : pData->snakeBody) {
      std::swap(rect, lastRect);
    }
  }

  std::cout << "Snake: " << pData->snakeBody.size() << '\n';

  const SDL_Rect snakeRect = {pData->snakeHead.x, pData->snakeHead.y,
                              pData->step-1, pData->step-1};
  for(auto& apple : pData->apples) {
    const SDL_Rect appleRect = {apple.x, apple.y, pData->step-1, pData->step-1};
    if(SDL_HasIntersection(&snakeRect, &appleRect)) {
      if(pData->snakeBody.size() < 8) {
        if(pData->snakeBody.empty()) {
          pData->snakeBody.push_back(pData->snakeHead);
        } else {
          pData->snakeBody.push_back(pData->snakeBody.back());
        }
      }

      static std::mt19937 rng(16);
      static std::uniform_int_distribution<int> gen(0, pData->width/pData->step); // uniform, unbiased
      apple = {gen(rng) * pData->step, gen(rng) * pData->step};
    }
  }
}

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

  SDL_RenderDrawLines(pRenderer, pData->borders.data(), pData->borders.size());

  for (uint32_t j = pData->step; j < pData->height; j += pData->step) {
    SDL_RenderDrawLine(pRenderer, 0, j, pData->width, j);
  }

  for (uint32_t i = pData->step; i < pData->width; i += pData->step) {
    SDL_RenderDrawLine(pRenderer, i, 0, i, pData->height);
  }
}

inline void drawApples(SDL_Renderer *pRenderer, const GameData *pData) {
  for(const auto& apple : pData->apples) {
    SDL_Rect renderRect = {apple.x, apple.y, pData->step-1, pData->step-1};
    const auto position = apple.y / pData->step;
    renderRect.y        = position * pData->step;
    SDL_RenderCopy(pRenderer, pData->pApple, nullptr, &renderRect);
  }
}

inline void drawSnake(SDL_Renderer *pRenderer, const GameData *pData) {
  const SDL_Rect rect = {pData->snakeHead.x, pData->snakeHead.y, pData->step - 1,
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
  SDL_SetRenderDrawColor(pRenderer, 0, 255, 0, 255);
  for(const auto& body : pData->snakeBody) {
    const SDL_Rect bodyRect = {body.x, body.y, pData->step - 1, pData->step - 1};
    SDL_RenderFillRect(pRenderer, &bodyRect);
  }
}

void render(SDL_Renderer *pRenderer, const GameData *pData) {
  drawBackground(pRenderer, pData);
  drawGrid(pRenderer, pData);
  drawApples(pRenderer, pData);
  drawSnake(pRenderer, pData);

  SDL_RenderPresent(pRenderer);
}

} // extern "C"
