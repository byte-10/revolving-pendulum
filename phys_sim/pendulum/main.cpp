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

#pragma pack(push, 1)
struct triangle_data {
  glm::vec3 normal;
  glm::vec3 verts[3];
  glm::u16  byte_count;
};
#pragma pack(pop)

void flip_zy(glm::vec3 &vec)
{
  float tmp = vec.z;
  vec.z = vec.y;
  vec.y = tmp;
}


/**
* source: https://en.wikipedia.org/wiki/STL_(file_format)
*/
class stl
{
public:
  struct triangle {
    glm::vec3 normal;
    glm::vec3 verts[3];
  };

  std::vector<triangle> triangles;

public:
  bool load(const char* ppath) {
    FILE* fp;
    char  buf[90];
    uint32_t count_triangles;
#ifdef WIN32
    fopen_s(&fp, ppath, "rb");
#else
    fp = fopen(ppath, "rb");
#endif
    if (!fp) {
      printf("failed to open file\n");
      return false;
    }

    constexpr size_t STL_header_size = 80;
    if (fread(buf, 1, STL_header_size, fp) != STL_header_size) {
      printf("read header error\n");
      fclose(fp);
      return false;
    }

    if (!memcmp(buf, "solid", sizeof("solid") - 1)) {
      printf("this is ASCII format!\n");
      fclose(fp);
      return false;
    }

    if (fread(&count_triangles, 1, sizeof(count_triangles), fp) != sizeof(count_triangles)) {
      printf("read error\n");
      fclose(fp);
      return false;
    }

    printf("count_triangles=%u\n", count_triangles);
    if (!count_triangles) {
      printf("no triangles\n");
      fclose(fp);
      return false;
    }

    triangles.reserve((size_t)count_triangles);
    for (uint32_t i = 0; i < count_triangles; i++) {
      triangle_data tri_data;
      if (fread(&tri_data, 1, sizeof(tri_data), fp) != sizeof(tri_data)) {
        printf("triangle read error\n");
        fclose(fp);
        return false;
      }
      printf("tri{(%f %f %f), (%f %f %f), (%f %f %f)} normal(%f %f %f) bytes_count: %hu\n",
        tri_data.verts[0].x, tri_data.verts[0].y, tri_data.verts[0].z,
        tri_data.verts[1].x, tri_data.verts[1].y, tri_data.verts[1].z,
        tri_data.verts[2].x, tri_data.verts[2].y, tri_data.verts[2].z,
        tri_data.normal.x, tri_data.normal.y, tri_data.normal.z,
        tri_data.byte_count);

      triangle tri;
      const float scale = 0.1f;
      tri.verts[0] = tri_data.verts[0] * scale;
      tri.verts[1] = tri_data.verts[1] * scale;
      tri.verts[2] = tri_data.verts[2] * scale;
      flip_zy(tri.verts[0]);
      flip_zy(tri.verts[1]);
      flip_zy(tri.verts[2]);

      tri.normal = tri_data.normal;
      triangles.push_back(tri);
    }
    fclose(fp);
    printf("Model loaded!  %zu triangles\n", triangles.size());
    return true;
  }

  void draw() {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < triangles.size(); i++) {
      triangle& tri = triangles[i];
      glNormal3fv(glm::value_ptr(tri.normal));
      glVertex3fv(glm::value_ptr(tri.verts[0]));
      glVertex3fv(glm::value_ptr(tri.verts[1]));
      glVertex3fv(glm::value_ptr(tri.verts[2]));
    }
    glEnd();
  }
};

SDL_Window* pwindow;
SDL_GLContext ctx;

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

stl model;
float curr_time;

int main(int argc, char *argv[])
{
  int width, height;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL_Init() failed! Error: \"%s\"", SDL_GetError());
    return 1;
  }

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

  if (!model.load("assembly_pendulum.stl")) {
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(pwindow);
    SDL_Quit();
    return 1;
  }
  
  int num_keys;
  const Uint8* pbuttons = SDL_GetKeyboardState(&num_keys);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glm::value_ptr(glm::vec4(0.8f, 0.8f, 0.8f, 1.f)));
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(glm::vec3(0.f, 500.f, 0.f)));
  //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(glm::vec3(0.f, -1.f, 0.f)));


  while (handle_events()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    curr_time = SDL_GetTicks() * 0.01f;
    glTranslatef(0.f, -10.f, -100.f);
    glRotatef(curr_time, 0.f, 1.f, 0.f);
    model.draw();

    SDL_GL_SwapWindow(pwindow);
  }
  SDL_GL_DeleteContext(ctx);
  SDL_DestroyWindow(pwindow);
  SDL_Quit();
  return 0;
}