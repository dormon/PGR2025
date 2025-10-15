#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>

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


  float modelMatrix[16] = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,
  };

  bool running = true;
  while(running){ // main loop
    SDL_Event event;
    while(SDL_PollEvent(&event)){ // event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
      if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_D)modelMatrix[12] += 0.01;
        if(event.key.key == SDLK_A)modelMatrix[12] -= 0.01;
        if(event.key.key == SDLK_W)modelMatrix[13] += 0.01;
        if(event.key.key == SDLK_S)modelMatrix[13] -= 0.01;
      }
    }
  
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
