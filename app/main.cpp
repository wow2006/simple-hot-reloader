// STL
#include <string>
#include <vector>
#include <random>
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
static ProcessInputFunc processInput;

using UpdateFunc = void (*)(GameData *pData);
static UpdateFunc update;

using RendererFunc = void (*)(const GameData *pData);
static RendererFunc renderer;

using InitializeFunc = void (*)(GameData *pData);
static InitializeFunc initialize;

using CleanFunc = void (*)(GameData *pData);
static CleanFunc clean;

static void updateDLL(GameData *pGameData) {
  static void *pDLL = nullptr;
  if(pDLL != nullptr) {
    clean(pGameData);
  }
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

  auto initializeFunc = SDL_LoadFunction(pObject, "initialize");
  if(initializeFunc == nullptr) {
    throw std::runtime_error("Can not load \"initialize\"");
  }
  auto cleanFunc = SDL_LoadFunction(pObject, "clean");
  if(cleanFunc == nullptr) {
    throw std::runtime_error("Can not load \"clean\"");
  }

  pDLL = pObject;
  processInput = reinterpret_cast<ProcessInputFunc>(processInputFunc);
  update       = reinterpret_cast<UpdateFunc>(updateFunc);
  renderer     = reinterpret_cast<RendererFunc>(renderFunc);
  initialize   = reinterpret_cast<InitializeFunc>(initializeFunc);
  clean        = reinterpret_cast<CleanFunc>(cleanFunc);

  initialize(pGameData);

  const time_t curr_time = time(nullptr);
  const tm *tm_local = localtime(&curr_time);
  std::cout << tm_local->tm_hour << ":" << tm_local->tm_min << ":" << tm_local->tm_sec <<
    " Load DLL successfully\n";
}

static void LogOutputFunction(void*           userdata,
                       int             category,
                       SDL_LogPriority priority,
                       const char*     message) {
  (void)userdata;
  (void)category;
  (void)priority;
  (void)message;
  std::cerr << "SDL_Message: " << message << '\n';
}

static auto loadImage(const char* path, SDL_Renderer *pRenderer) -> SDL_Texture* {
  const auto pImage = IMG_Load(path);
  if (pImage == nullptr) {
    fprintf(stderr, "Could not load image: %s\n", IMG_GetError());
    return nullptr;
  }

  const auto pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage);
  if(pTexture == nullptr) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
  }

  SDL_FreeSurface(pImage);
  return pTexture;
}

static bool initializeOpenGL(GameData *pGameData) {
  const auto pglGenBuffers = SDL_GL_GetProcAddress("glGenBuffers");
  if(pglGenBuffers == nullptr) {
    std::cerr << "ERROR: Can not load glGenBuffers\n";
    return false;
  }
  pGameData->gl.glGenBuffers = reinterpret_cast<glGenBuffersFunc>(pglGenBuffers);

  const auto pglBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
  if(pglBindBuffer == nullptr) {
    std::cerr << "ERROR: Can not load glBindBuffer\n";
    return false;
  }
  pGameData->gl.glBindBuffer = reinterpret_cast<glBindBufferFunc>(pglBindBuffer);

  const auto pglBufferData = SDL_GL_GetProcAddress("glBufferData");
  if(pglBufferData == nullptr) {
    std::cerr << "ERROR: Can not load glBufferData\n";
    return false;
  }
  pGameData->gl.glBufferData = reinterpret_cast<glBufferDataFunc>(pglBufferData);

  const auto pglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffers");
  if(pglDeleteBuffers == nullptr) {
    std::cerr << "ERROR: Can not load glDeleteBuffers\n";
    return false;
  }
  pGameData->gl.glDeleteBuffers = reinterpret_cast<glDeleteBuffersFunc>(pglDeleteBuffers);

  const auto pglDrawArrays = SDL_GL_GetProcAddress("glDrawArrays");
  if(pglDrawArrays == nullptr) {
    std::cerr << "ERROR: Can not load glDeleteBuffers\n";
    return false;
  }
  pGameData->gl.glDrawArrays = reinterpret_cast<glDrawArraysFunc>(pglDrawArrays);

  const auto pglEnableVertexAttribArray = SDL_GL_GetProcAddress("glEnableVertexAttribArray");
  if(pglEnableVertexAttribArray == nullptr) {
    std::cerr << "ERROR: Can not load glEnableVertexAttribArray\n";
    return false;
  }
  pGameData->gl.glEnableVertexAttribArray = reinterpret_cast<glEnableVertexAttribArrayFunc>(pglEnableVertexAttribArray);

  const auto pglDisableVertexAttribArray = SDL_GL_GetProcAddress("glDisableVertexAttribArray");
  if(pglDisableVertexAttribArray == nullptr) {
    std::cerr << "ERROR: Can not load glDisableVertexAttribArray\n";
    return false;
  }
  pGameData->gl.glDisableVertexAttribArray = reinterpret_cast<glDisableVertexAttribArrayFunc>(pglDisableVertexAttribArray);

  const auto pglVertexAttribPointer = SDL_GL_GetProcAddress("glVertexAttribPointer");
  if(pglVertexAttribPointer == nullptr) {
    std::cerr << "ERROR: Can not load glVertexAttribPointer\n";
    return false;
  }
  pGameData->gl.glVertexAttribPointer = reinterpret_cast<glVertexAttribPointerFunc>(pglVertexAttribPointer);

  return true;
}

int main() {
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

  const auto libraryFilePath = std::filesystem::path(BASE_DIR) / "lib" / "library.cpp";
  WatchFile file({
      {libraryFilePath.c_str(), [&gameData](){
        if(std::system("ninja") == 0) {
          gameData.reload = true;
        }
      }}
  });

  SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,          1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);

  auto pWindow = SDL_CreateWindow(
      "Hotreload",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      cWidth+1, cHeight+1, SDL_WINDOW_OPENGL);
  if (pWindow == nullptr) {
    std::cerr << "Unable to create SDL window: " << SDL_GetError() << '\n';
    return EXIT_FAILURE;
  }

  const auto pContext = SDL_GL_CreateContext(pWindow);
  if(pContext == nullptr) {
    std::cerr << "Unable to create OpenGL Context!\n";
    return EXIT_FAILURE;
  }

  if(!initializeOpenGL(&gameData)) {
    std::cerr << "Unable to initialize OpenGL!\n";
    return EXIT_FAILURE;
  }

  GLuint &vbo = gameData.vbo;
  gameData.gl.glGenBuffers(1, &vbo);
  gameData.gl.glBindBuffer(GL_ARRAY_BUFFER, vbo);
  gameData.gl.glBufferData(GL_ARRAY_BUFFER, gameData.borders.size() * sizeof(float),
                           gameData.borders.data(), GL_STATIC_DRAW);
  gameData.gl.glEnableVertexAttribArray(0);
  gameData.gl.glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, nullptr);
  gameData.gl.glDisableVertexAttribArray(0);
  gameData.gl.glBindBuffer(GL_ARRAY_BUFFER, 0);

  //auto pRenderer = SDL_CreateRenderer(pWindow, 1, SDL_RENDERER_SOFTWARE);
  //if(pRenderer == nullptr) {
  //  std::cerr << "Unable to create SDL Renderer: " << SDL_GetError() << '\n';
  //  return EXIT_FAILURE;
  //}

  //const auto pAppleTexture = loadImage("apple.png", pRenderer);
  //if(pAppleTexture == nullptr) {
  //  return EXIT_FAILURE;
  //}
  //gameData.pApple = pAppleTexture;

  //const auto pTexture = loadImage("snakeHead.png", pRenderer);
  //if(pTexture == nullptr) {
  //  return EXIT_FAILURE;
  //}
  //gameData.pSnake = pTexture;

  std::mt19937 rng(16);
  std::uniform_int_distribution<int> gen(0, gameData.width/gameData.step); // uniform, unbiased

  for(auto& rect : gameData.apples) {
    const auto index    = gen(rng) * gameData.step;
    rect = {
      index, 1,
      gameData.step - 1, gameData.step - 1
    };
  }

  constexpr auto FPS = 60.F;
  constexpr auto FrameDelay = static_cast<int>(1000.F / FPS);
  uint32_t frameStart = 0;
  uint32_t frameTime = 0;

  gameData.running = true;
  while (gameData.running) {
    frameStart = SDL_GetTicks();
    try {
      if(gameData.reload) {
        updateDLL(&gameData);
        gameData.reload = false;
      }
    } catch(const std::runtime_error& exp) {
      std::cerr << exp.what() << '\n';
    }

    processInput(&gameData);
    update(&gameData);
    renderer(&gameData);

    SDL_GL_SwapWindow(pWindow);
    frameTime = SDL_GetTicks() - frameStart;
    if(FrameDelay > frameTime) {
      SDL_Delay(FrameDelay - frameTime);
    }
    //frameTime = SDL_GetTicks() - frameStart;
    //std::cout << "\rFrame time : " << frameTime << " ms, " << 1000.F / static_cast<float>(frameTime);
  }

  //SDL_DestroyTexture(gameData.pApple);
  //SDL_DestroyTexture(gameData.pSnake);
  //SDL_DestroyRenderer(pRenderer);
  gameData.gl.glDeleteBuffers(1, &vbo);
  SDL_GL_DeleteContext(pContext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();

  return EXIT_SUCCESS;
}
