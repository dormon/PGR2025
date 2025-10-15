#include<SDL3/SDL.h>

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2025",1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  using GLCLEARCOLOR = void(*)(float,float,float,float);
  using GLCLEAR      = void(*)(int);

  GLCLEARCOLOR glClearColor = (GLCLEARCOLOR)SDL_GL_GetProcAddress("glClearColor");
  GLCLEAR      glClear      = (GLCLEAR     )SDL_GL_GetProcAddress("glClear"     );
 
#define GL_COLOR_BUFFER_BIT			0x00004000

  bool running = true;
  while(running){ // main loop
    SDL_Event event;
    while(SDL_PollEvent(&event)){ // event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
    }
  
    //rendering
    glClearColor(0,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window); // for double buffering / swap front and back buffer
  }

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  return 0;
}
