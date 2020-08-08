// STL
#include <array>
#include <random>
#include <fstream>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
// rapidjson
#include <rapidjson/document.h>
// Internal
#include "library.hpp"

inline auto loadImage(const char *path, SDL_Renderer *pRenderer)
    -> SDL_Texture * {
  const auto pImage = IMG_Load(path);
  if (pImage == nullptr) {
    fprintf(stderr, "Could not load image: %s\n", IMG_GetError());
    return nullptr;
  }

  const auto pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage);
  if (pTexture == nullptr) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
  }

  SDL_FreeSurface(pImage);
  return pTexture;
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

  SDL_RenderDrawLines(pRenderer, pData->borders.data(), static_cast<int>(pData->borders.size()));

  for (uint32_t j = pData->step; j < static_cast<uint32_t>(pData->height); j += pData->step) {
    SDL_RenderDrawLine(pRenderer, 0, j, pData->width, j);
  }

  for (uint32_t i = pData->step; i < static_cast<uint32_t>(pData->width); i += pData->step) {
    SDL_RenderDrawLine(pRenderer, i, 0, i, pData->height);
  }
}

inline void drawApples(SDL_Renderer *pRenderer, const GameData *pData) {
  for (const auto &apple : pData->apples) {
    SDL_Rect renderRect = {apple.x, apple.y, pData->step - 1, pData->step - 1};
    const auto position = apple.y / pData->step;
    renderRect.y = position * pData->step;
    SDL_RenderCopy(pRenderer, pData->pApple, nullptr, &renderRect);
  }
}

inline void drawSnake(SDL_Renderer *pRenderer, const GameData *pData) {
  const SDL_Rect rect = {pData->snakeHead.x, pData->snakeHead.y,
                         pData->step - 1, pData->step - 1};
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
  for (const auto &body : pData->snakeBody) {
    const SDL_Rect bodyRect = {body.x, body.y, pData->step - 1,
                               pData->step - 1};
    SDL_RenderFillRect(pRenderer, &bodyRect);
  }
}

extern "C" {

void firstInitialization(GameData *pData) {
  const std::string jsonFileName = "scene.json";
  std::ifstream inputStream(jsonFileName, std::ios::ate);
  const auto fileSize = inputStream.tellg();
  inputStream.seekg(0, std::ios::beg);
  std::string buffer;
  buffer.resize(static_cast<size_t>(fileSize)+1);
  inputStream.read(buffer.data(), buffer.size());

  rapidjson::Document d;
  d.Parse(buffer.data());

  const auto& apples = d["root"]["apples"];
  const auto pRenderer = pData->pRenderer;
  const auto pAppleTexture = loadImage(apples["texture"].GetString(), pRenderer);
  if (pAppleTexture == nullptr) {
    throw std::runtime_error("Can not create Texture \"Apple\"!");
  }
  pData->pApple = pAppleTexture;

  const auto& snake = d["root"]["snake"]["head"];
  const auto pTexture = loadImage(snake["texture"].GetString(), pRenderer);
  if (pTexture == nullptr) {
    throw std::runtime_error("Can not create Texture \"Snake head\"!");
  }
  pData->pSnake = pTexture;
  pData->snakeBody.reserve(8);

  std::mt19937 rng(16);
  std::uniform_int_distribution<int> gen(
      0, pData->width / pData->step); // uniform, unbiased

  for (auto &apple : pData->apples) {
    const auto index = gen(rng) * pData->step;
    apple = {
        index,
        1,
    };
  }

  pData->pFont = nullptr;
  const std::string fontfile = "ubuntu.ttf";
  if (!(pData->pFont = TTF_OpenFont(fontfile.c_str(), 16))) {
    std::cerr << "Error: Unable to open font\n";
  }
}

void initialize(GameData* /*pData*/) {}

void cleanup(GameData* /*pData*/) {}

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
        // pData->snakeHead = {1, 1};
        pData->snakeBody.clear();
        pData->snakeBody.reserve(8);
        break;
      }
    }
    }
  }
}

void update(GameData *pData) {
  if (pData->movement != Movement::None) {
    auto lastRect = pData->snakeHead;
    if (pData->movement == Movement::Right) {
      pData->snakeHead.x += pData->step;
    } else if (pData->movement == Movement::Left) {
      pData->snakeHead.x -= pData->step;
    } else if (pData->movement == Movement::Forward) {
      pData->snakeHead.y -= pData->step;
    } else if (pData->movement == Movement::Backword) {
      pData->snakeHead.y += pData->step;
    }

    pData->movement = Movement::None;

    for (auto &rect : pData->snakeBody) {
      std::swap(rect, lastRect);
    }
  }

  const SDL_Rect snakeRect = {pData->snakeHead.x, pData->snakeHead.y,
                              pData->step - 1, pData->step - 1};
  for (auto &apple : pData->apples) {
    const SDL_Rect appleRect = {apple.x, apple.y, pData->step - 1,
                                pData->step - 1};
    if (SDL_HasIntersection(&snakeRect, &appleRect)) {
      if (pData->snakeBody.size() < 8) {
        if (pData->snakeBody.empty()) {
          pData->snakeBody.push_back(pData->snakeHead);
        } else {
          pData->snakeBody.push_back(pData->snakeBody.back());
        }
      }
      ++pData->score;

      static std::mt19937 rng(16);
      static std::uniform_int_distribution<int> gen(
          0, pData->width / pData->step); // uniform, unbiased
      apple = {gen(rng) * pData->step, gen(rng) * pData->step};
    }
  }
}

void render(const GameData *pData) {
  const auto pRenderer = pData->pRenderer;
  drawBackground(pRenderer, pData);
  drawGrid(pRenderer, pData);
  drawApples(pRenderer, pData);
  drawSnake(pRenderer, pData);

  const SDL_Color White = {255, 255, 255, 255};
  {
    char buffer[256];
    sprintf(buffer, "Score: %lu", pData->score);
    SDL_Rect textRect = {0, 32, 0, 0};
    const auto pTextSurface = TTF_RenderText_Solid(pData->pFont, buffer, White);
    TTF_SizeText(pData->pFont, buffer, &textRect.w, &textRect.h);
    auto pTexture = SDL_CreateTextureFromSurface(pRenderer, pTextSurface);
    SDL_RenderCopy(pRenderer, pTexture, nullptr, &textRect);
    SDL_FreeSurface(pTextSurface);
    SDL_DestroyTexture(pTexture);
  }

  {
    const auto& [pTexture, rect] = pData->debugInfo;
    SDL_RenderCopy(pRenderer, pTexture, nullptr, &rect);
  }

  SDL_RenderPresent(pRenderer);
}

} // extern "C"
