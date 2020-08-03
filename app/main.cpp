// STL
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
// Internal
#include "watchFile.hpp"
#include "../lib/library.hpp"

namespace SDL {
constexpr int SUCCESS = 0;
} // namespace SDL
constexpr auto cWidth = 800;
constexpr auto cHeight = 600;

using ProcessInputFunc = void (*)(GameData *pData);
ProcessInputFunc processInput;

using UpdateFunc = void (*)(GameData *pData);
UpdateFunc update;

using RendererFunc = void (*)(SDL_Renderer *pRenderer, const GameData *pData);
RendererFunc renderer;

static void updateDLL() {
  static void *pDLL = nullptr;
  SDL_UnloadObject(pDLL);
  auto pObject = SDL_LoadObject("lib/libhot-reload-lib.so");
  if(pObject == nullptr) {
    throw std::runtime_error("Can not load so");
  }
  auto processInputFunc = SDL_LoadFunction(pObject, "processInput");
  if(processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"processInput\"");
  }
  auto updateFunc = SDL_LoadFunction(pObject, "update");
  if(updateFunc == nullptr) {
    throw std::runtime_error("Can not load \"update\"");
  }
  auto renderFunc = SDL_LoadFunction(pObject, "render");
  if(processInputFunc == nullptr) {
    throw std::runtime_error("Can not load \"render\"");
  }
  pDLL = pObject;
  processInput = reinterpret_cast<ProcessInputFunc>(processInputFunc);
  update       = reinterpret_cast<UpdateFunc>(updateFunc);
  renderer     = reinterpret_cast<RendererFunc>(renderFunc);

  const time_t curr_time = time(nullptr);
  const tm *tm_local = localtime(&curr_time);
  std::cout << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec <<
    " Load DLL successfully\n";
}

static
void LogOutputFunction(void*           userdata,
                       int             category,
                       SDL_LogPriority priority,
                       const char*     message) {
  std::cerr << "SDL_Message: " << message << '\n';
}

static auto loadImage(const char* path, SDL_Renderer *pRenderer) -> SDL_Texture* {
  auto pImage = IMG_Load(path);
  if (pImage == nullptr) {
    fprintf(stderr, "Could not load image: %s\n", IMG_GetError());
    return nullptr;
  }

  auto pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage);
  if(pTexture == nullptr) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
    return nullptr;
  }

  SDL_FreeSurface(pImage);
  return pTexture;
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != SDL::SUCCESS) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }
  // TODO(Hussein): check for return.
  const auto imgInit = IMG_Init(IMG_INIT_PNG);
  if((imgInit & IMG_INIT_PNG) != IMG_INIT_PNG) {
    std::cerr << "Unable to initialize SDL: " << IMG_GetError() << '\n';
    return EXIT_FAILURE;
  }

  GameData gameData;
  gameData.reload = true;

  WatchFile file({
      {"/home/ahussein/Documents/sourceCode/cpp/SIDE/hotreload/src/lib/library.cpp", [&gameData](){
        if(std::system("ninja") == 0) {
          gameData.reload = true;
        }
      }}
  });

  SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

  auto pWindow = SDL_CreateWindow(
      "Hotreload",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      cWidth+1, cHeight+1, 0);
  if (pWindow == nullptr) {
    std::cerr << "Unable to create SDL window: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  auto pRenderer = SDL_CreateRenderer(pWindow, 1, SDL_RENDERER_ACCELERATED);
  if(pRenderer == nullptr) {
    std::cerr << "Unable to create SDL Renderer: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  const auto pTexture = loadImage("apple.png", pRenderer);
  if(pTexture == nullptr) {
    return EXIT_FAILURE;
  }
  gameData.pApple = pTexture;

  constexpr auto FPS = 60.F;
  constexpr auto FrameDelay = 1000.F / FPS;
  uint32_t frameStart = 0;
  int32_t frameTime = 0;

  bool bRunning = true;
  while (bRunning) {
    frameStart = SDL_GetTicks();
    try {
      if(gameData.reload) {
        updateDLL();
        gameData.reload = false;
      }
    } catch(const std::runtime_error& exp) {
      std::cerr << exp.what() << '\n';
    }

    processInput(&gameData);
    update(&gameData);
    renderer(pRenderer, &gameData);

    frameTime = SDL_GetTicks() - frameStart;
    if(FrameDelay > frameTime) {
      SDL_Delay(FrameDelay - frameTime);
    }
    frameTime = SDL_GetTicks() - frameStart;
    //std::cout << "\rFrame time : " << frameTime << " ms, " << 1000.F / static_cast<float>(frameTime);
  }

  SDL_DestroyRenderer(pRenderer);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();

  return EXIT_SUCCESS;
}
