#pragma once
// STL
#include <array>
// OpenGL
#include <GL/gl.h>
// SDL2
#include <SDL2/SDL_render.h>

enum Direction : uint8_t { Up, Down, Left, Right };

using glGenBuffersFunc    = void(*)(GLsizei, GLuint*);
using glDeleteBuffersFunc = void(*)(GLsizei, GLuint*);

using glBindBufferFunc = void(*)(GLenum, GLuint);
using glBufferDataFunc = void(*)(GLenum, GLsizeiptr, const GLvoid*, GLenum);

using glEnableVertexAttribArrayFunc  = void(*)(GLuint);
using glDisableVertexAttribArrayFunc = void(*)(GLuint);
using glVertexAttribPointerFunc      = void(*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);

using glDrawArraysFunc = void(*)(GLenum, GLint, GLsizei);

using glCreateShaderFunc  = GLuint(*)(GLenum);
using glCreateProgramFunc = GLuint(*)();

struct OpenGL {
  glGenBuffersFunc    glGenBuffers;
  glDeleteBuffersFunc glDeleteBuffers;
  glBindBufferFunc    glBindBuffer;
  glBufferDataFunc    glBufferData;

  glEnableVertexAttribArrayFunc  glEnableVertexAttribArray;
  glDisableVertexAttribArrayFunc glDisableVertexAttribArray;
  glVertexAttribPointerFunc      glVertexAttribPointer;

  glDrawArraysFunc    glDrawArrays;
};

struct GameData {
  bool running = false;
  bool reload  = false;

  int step   = 40;
  int width  = 800;
  int height = 600;

  SDL_Color backgroundColor = SDL_Color{16, 16, 32, SDL_ALPHA_OPAQUE};
  SDL_Color forgroundColor  = SDL_Color{128, 128, 128, SDL_ALPHA_OPAQUE};

  OpenGL gl;

  GLuint vbo = 0;

  int currentX = 1;
  int currentY = 1;
  Direction direction = Direction::Up;

  SDL_Texture* pApple = nullptr;
  SDL_Texture* pSnake = nullptr;

  std::array<float, 5*2> borders {
    0, 0,
    static_cast<float>(width), 0,
    static_cast<float>(width), static_cast<float>(height),
    0, static_cast<float>(height),
    0, 0
  };

  std::array<SDL_Rect, 8> apples;

};

