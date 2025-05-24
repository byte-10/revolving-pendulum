#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <vector>
#include <string>
#include <glm/glm.hpp> //glm::vec3, glm::vec2
#include <glm/gtc/matrix_transform.hpp> //glm::translate, glm::rotate, glm::scale and more..
#include <glm/gtc/type_ptr.hpp> //glm::value
#include "wavefront_obj.h"

SDL_Window* pwindow;
SDL_GLContext ctx;
obj_importer obj;

void update_viewport(int width, int height)
{
  /* division by zero pewvwntion */
  if (height == 0)
    height = 1;

  printf("window resized: %dx%d\n", width, height);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.f, width / (double)height, 0.1, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

bool handle_events()
{
  SDL_Event event;
  static bool b_active = true;
  while (SDL_PollEvent(&event)) {
    switch (event.type)
    {
    case SDL_WINDOWEVENT: {
      SDL_WindowEvent& we = event.window;
      switch (we.event)
      {
      case SDL_WINDOWEVENT_RESIZED:
        update_viewport(we.data1, we.data2);
        break;

      case SDL_WINDOWEVENT_CLOSE:
        b_active = false; //for finish draw cycle in the next frame
        break;

      default:
        break;
      }
      break;
    }

    default:
      break;
    }
  }
  return b_active;
}

float curr_time;

int main(int argc, char *argv[])
{
  int width, height;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL_Init() failed! Error: \"%s\"", SDL_GetError());
    return 1;
  }
  printf("loading model...\n");
  if (!obj.load("pendulum.obj")) {
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(pwindow);
    SDL_Quit();
    return 1;
  }
  printf("model loaded!\n");

  pwindow = SDL_CreateWindow("pendulum graphics", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 1024, 
    SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
  if (!pwindow) {
    printf("SDL_CreateWindow() failed! Error: \"%s\"", SDL_GetError());
    return 1;
  }

  ctx = SDL_GL_CreateContext(pwindow);
  if (!ctx) {
    printf("SDL_GL_CreateContext() failed! Error: \"%s\"", SDL_GetError());
    return 1;
  }
  if (SDL_GL_MakeCurrent(pwindow, ctx) < 0) {
    printf("SDL_GL_MakeCurrent() failed! Error: \"%s\"", SDL_GetError());
    return 1;
  }
  SDL_GetWindowSize(pwindow, &width, &height);
  update_viewport(width, height); //update viewport for current window size

  int num_keys;
  const Uint8* pbuttons = SDL_GetKeyboardState(&num_keys);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glm::value_ptr(glm::vec4(0.8f, 0.8f, 0.8f, 1.f)));
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(glm::vec3(0.f, 500.f, 0.f)));
  //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(glm::vec3(0.f, -1.f, 0.f)));

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  while (handle_events()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    curr_time = SDL_GetTicks() * 0.01f;
    glTranslatef(0.f, -10.f, -100.f);
    glRotatef(curr_time, 0.f, 1.f, 0.f);
    for (size_t i = 0; i < obj.get_num_meshes(); i++) {
      obj_importer::mesh* pmesh = obj.get_mesh(i);
      glVertexPointer(3, GL_FLOAT, 0, pmesh->get_verts());
      glNormalPointer(GL_FLOAT, 0, pmesh->get_normals());
      glDrawArrays(GL_TRIANGLES, 0, pmesh->get_num_verts());
    }
    SDL_GL_SwapWindow(pwindow);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  SDL_GL_DeleteContext(ctx);
  SDL_DestroyWindow(pwindow);
  SDL_Quit();
  return 0;
}