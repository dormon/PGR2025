#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>
#include<cmath>
#include<map>

#include<bunny.hpp>

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

void rotateX(float*O,float angle){
  matrixIdentity(O);
  auto cosa = cos(angle);
  auto sina = sin(angle);
  O[5 ] = +cosa;
  O[6 ] = +sina;
  O[9 ] = -sina;
  O[10] = +cosa;
}

void rotateY(float*O,float angle){
  matrixIdentity(O);
  auto cosa = cos(angle);
  auto sina = sin(angle);
  O[0 ] = +cosa;
  O[2 ] = +sina;
  O[8 ] = -sina;
  O[10] = +cosa;
}

void translate(float*O,float x,float y,float z){
  matrixIdentity(O);
  O[12] = x;
  O[13] = y;
  O[14] = z;
}

void frustum(float*O,float L,float R,float B,float T,float n,float f){
  matrixIdentity(O);
  O[0] = 2*n/(R-L);
  O[5] = 2*n/(T-B);
  O[8] = (R+L)/(R-L);
  O[9] = (T+B)/(T-B);
  O[10] = -(f+n)/(f-n);
  O[11] = -1;
  O[14] = -2*n*f/(f-n);
  O[15] = 0;
}

void perspective(float*O,float fovy,float aspect,float n,float f){
  float R = n*tan(fovy/2);
  float L = -R;
  float T = R/aspect;
  float B = -T;
  frustum(O,L,R,B,T,n,f);
}

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2025",1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  ge::gl::init(); // initialization of OpenGL / function pointers

  GLuint vbo;
  glCreateBuffers(1,&vbo);
  glNamedBufferData(vbo,sizeof(bunnyVertices),bunnyVertices,GL_DYNAMIC_DRAW);

  GLuint ebo;
  glCreateBuffers(1,&ebo);
  glNamedBufferData(ebo,sizeof(bunnyIndices),bunnyIndices,GL_DYNAMIC_DRAW);

  GLuint vao;
  glCreateVertexArrays(1,&vao);
  glVertexArrayAttribBinding(vao,0,0);
  glEnableVertexArrayAttrib(vao,0);
  glVertexArrayAttribFormat(vao,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayVertexBuffer(vao,0,vbo,sizeof(float)*0,sizeof(BunnyVertex));
  glVertexArrayAttribBinding(vao,1,1);
  glEnableVertexArrayAttrib(vao,1);
  glVertexArrayAttribFormat(vao,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayVertexBuffer(vao,1,vbo,sizeof(float)*3,sizeof(BunnyVertex));
  glVertexArrayElementBuffer(vao,ebo);


  auto vsSrc = R".(
  #version 460

  layout(location=0)in vec3 position;
  layout(location=1)in vec3 normal  ;
  out vec3 vColor;

  uniform mat4 viewMatrix = mat4(1);
  uniform mat4 projMatrix = mat4(1);

  void main(){
    mat4 pv = projMatrix * viewMatrix;
    gl_Position = pv*vec4(position,1);
    vColor = normal;
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

  auto viewMatrixL = glGetUniformLocation(prg,"viewMatrix");
  auto projMatrixL = glGetUniformLocation(prg,"projMatrix");

  float cameraPosition[3] = {0,0,3};
  float angleX = 0.f;
  float angleY = 0.f;

  float viewMatrix[16];
  float VT[16];
  float VR[16];

  float VRX[16];
  float VRY[16];

  float projMatrix[16];
  matrixIdentity(projMatrix);


  float sensitivity = 0.01;
  float cameraSpeed = 0.1;

  std::map<int,bool>keys;

  glEnable(GL_DEPTH_TEST);
  bool running = true;
  while(running){ // main loop
    SDL_Event event;
    while(SDL_PollEvent(&event)){ // event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
      if(event.type == SDL_EVENT_KEY_UP)
        keys[event.key.key] = false;
      if(event.type == SDL_EVENT_KEY_DOWN)
        keys[event.key.key] = true;
      if(event.type == SDL_EVENT_MOUSE_MOTION){
        if(event.motion.state & SDL_BUTTON_LEFT){
          angleY -= event.motion.xrel * sensitivity;
          angleX += event.motion.yrel * sensitivity;
        }
      }
    }
  

    // (VR[0],VR[4],VR[8 ]) - X axis of camera
    // (VR[1],VR[5],VR[9 ]) - Y
    // (VR[2],VR[6],VR[10]) - Z axis
    float leftRight       = ((int)(keys[SDLK_D])-(int)keys[SDLK_A]) * cameraSpeed;
    float forwardBackward = ((int)(keys[SDLK_S])-(int)keys[SDLK_W]) * cameraSpeed;
    float upDown          = ((int)(keys[SDLK_SPACE])-(int)keys[SDLK_LSHIFT]) * cameraSpeed;
    cameraPosition[0] += VR[0]*leftRight;
    cameraPosition[1] += VR[4]*leftRight;
    cameraPosition[2] += VR[8]*leftRight;
    cameraPosition[0] += VR[2 ]*forwardBackward;
    cameraPosition[1] += VR[6 ]*forwardBackward;
    cameraPosition[2] += VR[10]*forwardBackward;

    cameraPosition[0] += VR[1 ]*upDown;
    cameraPosition[1] += VR[5 ]*upDown;
    cameraPosition[2] += VR[9 ]*upDown;


    translate(VT,-cameraPosition[0],-cameraPosition[1],-cameraPosition[2]);
    rotateX(VRX,angleX);
    rotateY(VRY,angleY);

    matrixMultiplication(VR,VRX,VRY);
    matrixMultiplication(viewMatrix,VR,VT);

    perspective(projMatrix,1024/768.,90./180.*3.1415925,0.1,1000.f);


    //rendering
    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glPointSize(10);
    glUseProgram(prg);
    glProgramUniformMatrix4fv(prg,viewMatrixL,1,GL_FALSE,viewMatrix);
    glProgramUniformMatrix4fv(prg,projMatrixL,1,GL_FALSE,projMatrix);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES,sizeof(bunnyIndices)/sizeof(uint32_t),GL_UNSIGNED_INT,0);

    SDL_GL_SwapWindow(window); // for double buffering / swap front and back buffer
  }

  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  return 0;
}
