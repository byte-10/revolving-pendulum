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
#include "node.h"

SDL_Window* pwindow;
SDL_GLContext ctx;
obj_importer obj;
float current_time;

#define arrsize(x) (sizeof(x) / sizeof(x[0]))

glm::vec3 barycenter(bbox_t &dst, const glm::vec3 *pverts, size_t count)
{
  glm::vec3 total(0.f, 0.f, 0.f);
  dst.init();
  for (size_t i = 0; i < count; i++) {
    total += pverts[i];
    dst.update(pverts[i]);
  }
  return total / float(count);
}

void transform_verts(glm::vec3* pverts, size_t count, const glm::mat4x4 &mat)
{
  for (size_t i = 0; i < count; i++)
    pverts[i] = mat * glm::vec4(pverts[i], 1.f);
}

model_node nodes_hierarchy[] = {
  model_node(-1, "node0", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node1", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node2", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(2,  "node3", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node4", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node5", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node6", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node7", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node8", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f })
};

model_node* find_node(const char *pname)
{
  for (auto &pnode : nodes_hierarchy) {
    if (!strcmp(pnode.get_name(), pname)) {
      return &pnode;
    }
  }
  return nullptr;
}

/**
* prepare_nodes
*/
void prepare_nodes(model_node *pnodes, int count)
{
  for (int i = 0; i < count; i++) {
    model_node* pparent = &pnodes[i];
    for (int j = 0; j < count; j++) {
      model_node* pchild = &pnodes[j];
      if (pchild->get_parent_id() == i) {
        pparent->add_child(j);
      }
    }
  }
}

/**
* print_childs
*/
void print_childs(const model_node* pnodes, int count)
{
  printf("=== print_childs ===\n");
  for (int i = 0; i < count; i++) {
    const model_node* pnode = &pnodes[i];
    printf(" parent: %d   \"%s\"\n", i, pnode->get_name());
    for (int j = 0; j < pnode->get_num_childs(); j++) {
      const model_node* pchild = &pnodes[pnode->get_child(j)];
      printf("   this child \"%s\" (parent: %d)\n",
        pchild->get_name(), pchild->get_parent_id());
    }
  }
  printf("\n");
}

glm::vec3 centers[arrsize(nodes_hierarchy)];

GLUquadric* psphere = nullptr;

void change_coord_system(model_node* pnodes, int count, const obj_importer *pimp)
{
  model_node* pnode, *pparent;
  obj_importer::mesh* pmesh;
  /* compute all nodes center */
  for (int i = 0; i < count; i++) {
    pnode = &pnodes[i];
    pmesh = pimp->get_mesh(i);
    pparent = &pnodes[pnode->get_parent_id()];
    centers[i] = barycenter(pnode->get_bbox(), pmesh->get_verts(), pmesh->get_num_verts());
  }

#if 1
  for (int i = 0; i < count; i++) {
    glm::vec3 delta;
    pnode = &pnodes[i];
    pmesh = pimp->get_mesh(i);
    pparent = &pnodes[pnode->get_parent_id()];
    delta = (pnode->get_parent_id() != -1) ? centers[i] - centers[pnode->get_parent_id()] : glm::vec3(0.f, 0.f, 0.f);
    pnode->set_position(delta);

    /* handle pendulum node */
    //if (!strcmp(pnode->get_name(), "node3")) {
    //  /* find top of the model */
    //  assert(pnode->get_parent_id() != -1 && "parent node is -1! this is root");
    //  model_node* pparent = &pnodes[pnode->get_parent_id()];
    //  const glm::vec3 &vmin = pnode->get_bbox().vec_min;
    //  const glm::vec3 &vmax = pnode->get_bbox().vec_max;
    //  const glm::vec3& pos = pnode->get_pos();
    //  printf("found pendulum node!  pos( %f %f %f )  bbox{ min( %f %f %f ), max( %f %f %f ) }\n", 
    //    pos.x, pos.y, pos.z,
    //    vmin.x, vmin.y, vmin.z,
    //    vmax.x, vmax.y, vmax.z);
    //  glm::vec3 attach_pos = glm::min(pparent->get_bbox().vec_max, pnode->get_bbox().vec_max);
    //}
    transform_verts(pmesh->get_verts(), pmesh->get_num_verts(), glm::translate(glm::mat4x4(1.f), -centers[i]));

    bbox_t govno;
    glm::vec3 null_center = barycenter(govno, pmesh->get_verts(), pmesh->get_num_verts());
    printf("node %d   null_center= ( %f %f %f )\n", i, null_center.x, null_center.y, null_center.z);
    //assert(null_center == glm::vec3(0.f, 0.f, 0.f) && "verts translation incorrect");
  }
#endif
}

class pendulum_vis
{

public:
};

float get_time()
{
  Uint64 freq = SDL_GetPerformanceFrequency();
  Uint64 counter = SDL_GetPerformanceCounter();
  return counter / (double)freq;
}

void update_viewport(int width, int height)
{
  /* division by zero pewvwntion */
  if (height == 0)
    height = 1;

  printf("window resized: %dx%d\n", width, height);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.f, width / (double)height, 1.0, 1000.0);
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

void draw_recursive(int myid)
{
  GLenum error;
  if (myid == -1)
    return;

  glPushMatrix();
  model_node* pnode = &nodes_hierarchy[myid];
  const glm::vec3 angles = pnode->get_rotation();
  glRotatef(angles.x, 1.f, 0.f, 0.f);
  glRotatef(angles.y, 0.f, 1.f, 0.f);
  glRotatef(angles.z, 0.f, 0.f, 1.f);
  const glm::vec3& pos = pnode->get_pos();
  glTranslatef(pos.x, pos.y, pos.z);

  obj_importer::mesh* pmesh = obj.get_mesh(myid);
  glVertexPointer(3, GL_FLOAT, 0, pmesh->get_verts());
  glNormalPointer(GL_FLOAT, 0, pmesh->get_normals());
  glDrawArrays(GL_TRIANGLES, 0, pmesh->get_num_verts());
  error = glGetError();
  if (error != GL_NO_ERROR) {
    printf("glGetError returned error: %d (0x%x)!\n",
      error, error);
  }

  for (int i = 0; i < pnode->get_num_childs(); i++) {
    int child_id = pnode->get_child(i);
    assert(child_id != -1 && "child loop");
    draw_recursive(child_id);
  }
  glPopMatrix();
}

void draw_model()
{
  draw_recursive(0);
}

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

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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
  glEnable(GL_MULTISAMPLE);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
  GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat mat_shininess = 50.0f;
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glm::vec3 position(0.f, 100.f, 0.f);
  GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(position));
  glEnable(GL_LIGHT0);

  prepare_nodes(nodes_hierarchy, arrsize(nodes_hierarchy));
  print_childs(nodes_hierarchy, arrsize(nodes_hierarchy));
  change_coord_system(nodes_hierarchy, arrsize(nodes_hierarchy), &obj);

  model_node* ppendulum_node = find_node("node3");
  model_node* pload0 = find_node("node4");
  model_node* pload1 = find_node("node5");
  assert(ppendulum_node && "ppendulum_node was nullptr!");

  psphere = gluNewQuadric();

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  while (handle_events()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    current_time = get_time();
    glTranslatef(0.f, -10.f, -60.f);
    glRotatef(current_time, 0.f, 1.f, 0.f);

    constexpr float deg = 90.f;
    float sinv = sinf(current_time);
    ppendulum_node->set_rotation({ 0.f, 0.f, sinv * deg});
    pload0->set_position({ 0.f, sinv * 10.f, 0.f });
    pload1->set_position({ 0.f, sinv * -10.f, 0.f });

    draw_model();

    /* draw centers */
    GLint old_depth_mode;
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glGetIntegerv(GL_DEPTH_FUNC, &old_depth_mode);
    glDepthFunc(GL_ALWAYS);
    for (size_t i = 0; i < arrsize(centers); i++) {
      glm::vec3& center = centers[i];
      glPushAttrib(GL_CURRENT_BIT);
      glColor3f(0.f, 1.f, 0.f);
      glTranslatef(center.x, center.y, center.z);
      gluCylinder(psphere, 1.0, 0., 1.0, 10, 10);
      glTranslatef(-center.x, -center.y, -center.z);
      glPopAttrib();
    }
    glDepthFunc(old_depth_mode);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
    SDL_GL_SwapWindow(pwindow);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  SDL_GL_DeleteContext(ctx);
  SDL_DestroyWindow(pwindow);
  SDL_Quit();
  return 0;
}