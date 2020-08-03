// STL
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
// SDL2
#include <SDL2/SDL.h>
// Internal
#include "watchFile.hpp"

namespace SDL {
constexpr int SUCCESS = 0;
} // namespace SDL
constexpr auto cWidth = 800;
constexpr auto cHeight = 600;

void (*processInput)(bool &bRunning, bool &bReload);

void (*update)();

void (*render)(SDL_Renderer *pRenderer);

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
  processInput = reinterpret_cast<void(*)(bool&, bool&)>(processInputFunc);
  update       = reinterpret_cast<void(*)()>(updateFunc);
  render       = reinterpret_cast<void(*)(SDL_Renderer*)>(renderFunc);

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

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != SDL::SUCCESS) {
    std::cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  bool bReload  = true;
  WatchFile file({
      {"/home/ahussein/Documents/sourceCode/cpp/SIDE/hotreload/src/lib/library.cpp", [&bReload](){
        if(std::system("ninja") == 0) {
          bReload = true;
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

  constexpr auto FPS = 60.F;
  constexpr auto FrameDelay = 1000.F / FPS;
  uint32_t frameStart = 0;
  int32_t frameTime = 0;

  bool bRunning = true;
  while (bRunning) {
    frameStart = SDL_GetTicks();
    try {
      if(bReload) {
        updateDLL();
        bReload = false;
      }
    } catch(const std::runtime_error& exp) {
      std::cerr << exp.what() << '\n';
    }
    processInput(bRunning, bReload);
    update();
    render(pRenderer);
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
