
#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <stdio.h>

#ifdef PLATFORM_EMCC
#include <emscripten.h>
#endif

//
// GL
#ifdef PLATFORM_MINGW
#include <glew.h>
#if defined(GLEW_EGL)
#include <eglew.h>
#elif defined(GLEW_OSMESA)
#define GLAPI extern
#include <osmesa.h>
#elif defined(_WIN32)
#include <wglew.h>
#elif !defined(__APPLE__) && !defined(__HAIKU__) || defined(GLEW_APPLE_GLX)
#include <glxew.h>
#endif
#else
#include <SDL_opengl.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int _isQuit = 0;
typedef struct {
  int dummy;
} Context;
Context ctx;

GLuint cglu_loadShader(GLenum type, const char *shaderSrc);

void main_loop(void*args) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        _isQuit = 1;
    }
  }
}


int fps;
int frgShader;
int verShader;
int program;
int width = 400;
int height = 300;

GLfloat vertexBufferData[] = {
  -0.5f, 0.5f, 0.0f,  0.0, 0.0,
  0.5f, 0.5f, 0.0f,   1.0, 0.0,
  0.5f, -0.5f, 0.0f,  1.0, 1.0,
  -0.5f, -0.5f, 0.0f, 0.0, 1.0,
};

GLshort indexData[] = {
  0,1,2,  0,2,3
};

void _onInit() {
  printf("## onInit\r\n");

  glEnable(GL_DEPTH_TEST);

  //glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, width, height);
  glClearColor(1.0, 0.7, 0.7, 1.0);//rgba
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  //

  frgShader = cglu_loadShader(GL_FRAGMENT_SHADER,
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "uniform sampler2D texture;\n"
    "varying vec2 textureCoord;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(texture, textureCoord)* vec4(1.0,1.0,1.0,1.0);\n"
//    "  gl_FragColor = vec4(1.0,1.0,1.0,1.0);\n"
    "}\n");
  verShader = cglu_loadShader(GL_VERTEX_SHADER,
    "attribute vec4 position;\n"
    "attribute vec2 texCoord;\n"
    "varying vec2 textureCoord;\n"
    "void main() {\n"
    "  textureCoord = texCoord;\n"
    "  gl_Position = position;\n"
    "}\n");

  program = glCreateProgram();
  glAttachShader(program, frgShader);
  glAttachShader(program, verShader);
  glLinkProgram(program);

}

void _onDisplay() {
  //
  //
  int texture;
  SDL_Surface *image = IMG_Load("./assets/icon.png");
  if (!image)
  {
     printf("Failed at IMG_Load: %s\n", IMG_GetError());
     return ;
  }


  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // buffer
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint textureBuffer;
  glGenBuffers(1, &vertexBuffer);
  glGenBuffers(1, &indexBuffer);
  glGenTextures(1,&textureBuffer);

  // texture
  GLenum data_fmt;
  GLint nOfColors = image->format->BytesPerPixel;
  if (nOfColors == 4) {
    if (image->format->Rmask == 0x000000ff) {
      data_fmt = GL_RGBA;
    } else {
      data_fmt = GL_BGRA;
    }
  } else if (nOfColors == 3) {
    if (image->format->Rmask == 0x000000ff){
      data_fmt = GL_RGBA;
    } else {
      data_fmt = GL_BGRA;
    }
  } else {
    printf("warning: the image is not truecolor..  this will probably break\n");
  }
  glBindTexture(GL_TEXTURE_2D, textureBuffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, data_fmt,//GL_RGBA,//data_fmt,
      image->w, image->h, 0, data_fmt, GL_UNSIGNED_BYTE, image->pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
//  printf("%d %d %d\r\n", image->w, image->h, test);
  SDL_FreeSurface(image);
  //
  // shader
  glUseProgram(program);
  int positionLoc = glGetAttribLocation(program, "position");
  int texCoordLoc = glGetAttribLocation(program, "texCoord");
  glEnableVertexAttribArray(positionLoc);
  glEnableVertexAttribArray(texCoordLoc);


  //
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*5*4, vertexBufferData, GL_STATIC_DRAW);

  glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
  glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3* sizeof(GLfloat)));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLshort)*6, indexData, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &indexBuffer);


}

int main( int argc, char** args )
{
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window;
 #ifdef PLATFORM_MINGW
  SDL_GLContext glContext;
  window = SDL_CreateWindow("sdlglshader", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
  glContext = SDL_GL_CreateContext(window);
#else
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(600, 400, 0, &window, &renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_RenderClear(renderer);
#endif

#ifdef PLATFORM_MINGW
  glewExperimental = GL_TRUE; 
  GLenum glewError = glewInit();
  if( glewError != GLEW_OK ) {
      printf( "Failed at glewInit! %s\n", glewGetErrorString( glewError ) );
  }
#endif
  //
  //
  _onInit();
  _onDisplay();
  //
  //
#ifdef PLATFORM_MINGW
  SDL_GL_SwapWindow(window);
#else
  SDL_RenderPresent(renderer);
#endif
#ifdef PLATFORM_EMCC
  emscripten_set_main_loop_arg(main_loop, &ctx, 60, 1);
#else
  do {
    main_loop(&ctx);
  } while(_isQuit == 0);
#endif
  return 0;
}




GLuint cglu_loadShader(GLenum type, const char *shaderSrc) {
  GLuint shader;
  GLint compiled;
  shader = glCreateShader(type);
  if(shader == 0) {
    return 0;
  }

  glShaderSource(shader, 1, &shaderSrc, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if(!compiled) {
    printf("Error compiling shader:\n");
    printf("src:\n%s\n", shaderSrc);
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if(infoLen > 0) {
      char* infoLog = malloc(sizeof(char) * infoLen);
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      printf("infolog:\n%s\n", infoLog);
      free(infoLog);
    }
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}
