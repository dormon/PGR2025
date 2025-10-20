#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>
#include<cmath>

using namespace ge::gl;

GLuint createShader(GLenum type,std::string const&src){
  auto vs = glCreateShader(type);
  char const*srcs[]={
    src.c_str(),
  };
  glShaderSource(vs,1,srcs,0);
  glCompileShader(vs);
  
  GLint status;
  glGetShaderiv(vs,GL_COMPILE_STATUS,&status);
  if(status != GL_TRUE){
    char buf[10000];
    glGetShaderInfoLog(vs,10000,0,buf);
    std::cerr << "ERROR: " << buf << std::endl;
  }

  return vs;
}

GLuint createProgram(std::vector<GLuint>const&shaders){
  auto prg = glCreateProgram();
  for(auto const&x:shaders)
    glAttachShader(prg,x);
  glLinkProgram(prg);

  GLint status;
  glGetProgramiv(prg,GL_LINK_STATUS,&status);
  if(status != GL_TRUE){
    char buf[10000];
    glGetProgramInfoLog(prg,10000,0,buf);
    std::cerr << "ERROR: " << buf << std::endl;
  }

  return prg;
}

void matrixMultiplication(float* O, float* A, float* B) {
    for (int c = 0; c < 4; ++c) // column
        for (int r = 0; r < 4; ++r) { // row
            O[c * 4 + r] = 0;
            for (int i = 0; i < 4; ++i)
                O[c * 4 + r] += A[i*4+r] * B[c*4+i];
        }
}

void matrixIdentity(float* O) {
    for (int c = 0; c < 4; ++c) // column
        for (int r = 0; r < 4; ++r) // row
            O[c * 4 + r] = (float)(c == r);
}

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2025",1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  ge::gl::init(); // initialization of OpenGL / function pointers

  auto vsSrc = R".(
  #version 460

  out vec3 vColor;

  uniform mat4 modelMatrix = mat4(1);

  void main(){
    if(gl_VertexID==0){gl_Position = modelMatrix*vec4(0,0,0,1);vColor = vec3(1,0,0);}
    if(gl_VertexID==1){gl_Position = modelMatrix*vec4(1,0,0,1);vColor = vec3(0,1,0);}
    if(gl_VertexID==2){gl_Position = modelMatrix*vec4(0,1,0,1);vColor = vec3(0,0,1);}
  }
  ).";

  auto fsSrc = R".(
  #version 460

  in vec3 vColor;
  out vec4 fColor;

  void main(){
    fColor = vec4(vColor,1);
  }
  ).";


  auto vs = createShader(GL_VERTEX_SHADER,vsSrc);
  auto fs = createShader(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = createProgram({vs,fs});

  auto modelMatrixL = glGetUniformLocation(prg,"modelMatrix");

  float angle = 0.f;
  float Sx = 1.f;
  float Sy = 1.f;
  float Sz = 1.f;

  float modelMatrix[16];
  float translateMatrix[16];
  float scaleMatrix[16];
  float rotateZMatrix[16];
  matrixIdentity(modelMatrix);
  matrixIdentity(translateMatrix);
  matrixIdentity(scaleMatrix);
  matrixIdentity(rotateZMatrix);


  bool running = true;
  while(running){ // main loop
    SDL_Event event;
    while(SDL_PollEvent(&event)){ // event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
      if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_D)translateMatrix[12] += 0.01;
        if(event.key.key == SDLK_A)translateMatrix[12] -= 0.01;
        if(event.key.key == SDLK_W)translateMatrix[13] += 0.01;
        if(event.key.key == SDLK_S)translateMatrix[13] -= 0.01;
        if (event.key.key == SDLK_O)scaleMatrix[0] += 0.01;
        if (event.key.key == SDLK_P)scaleMatrix[0] -= 0.01;
        if (event.key.key == SDLK_K)scaleMatrix[5] += 0.01;
        if (event.key.key == SDLK_L)scaleMatrix[5] -= 0.01;
        if (event.key.key == SDLK_M)angle += 0.01;
      }
    }
  
    rotateZMatrix[0] = +cos(angle);
    rotateZMatrix[1] = -sin(angle);
    rotateZMatrix[4] = +sin(angle);
    rotateZMatrix[5] = +cos(angle);

    float C[16];
    matrixMultiplication(C, rotateZMatrix, scaleMatrix);
    matrixMultiplication(modelMatrix, translateMatrix, C);

    //rendering
    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(10);
    glUseProgram(prg);
    glProgramUniformMatrix4fv(prg,modelMatrixL,1,GL_FALSE,modelMatrix);
    glDrawArrays(GL_TRIANGLES,0,3);

    SDL_GL_SwapWindow(window); // for double buffering / swap front and back buffer
  }

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  return 0;
}
