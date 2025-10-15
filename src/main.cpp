#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>

using namespace ge::gl;

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2025",1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  ge::gl::init(); // initialization of OpenGL / function pointers

  auto vsSrc = R".(
  #version 460
  void main(){
    if(gl_VertexID==0)gl_Position = vec4(0,0,0,1);
    if(gl_VertexID==1)gl_Position = vec4(1,0,0,1);
    if(gl_VertexID==2)gl_Position = vec4(0,1,0,1);
  }
  ).";

  auto fsSrc = R".(
  #version 460
  out vec4 fColor;
  void main(){
    fColor = vec4(1,0,1,1);
  }
  ).";

  auto vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs,1,&vsSrc,0);
  glCompileShader(vs);

  auto fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs,1,&fsSrc,0);
  glCompileShader(fs);

  auto prg = glCreateProgram();
  glAttachShader(prg,vs);
  glAttachShader(prg,fs);
  glLinkProgram(prg);


  bool running = true;
  while(running){ // main loop
    SDL_Event event;
    while(SDL_PollEvent(&event)){ // event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
    }
  
    //rendering
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(10);
    glUseProgram(prg);
    glDrawArrays(GL_TRIANGLES,0,3);

    SDL_GL_SwapWindow(window); // for double buffering / swap front and back buffer
  }

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  return 0;
}
