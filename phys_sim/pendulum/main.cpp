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

#define arrsize(x) (sizeof(x) / sizeof(x[0]))

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

SDL_Window*   pwindow;
SDL_GLContext ctx;
obj_importer  obj;
float         current_time;
glm::ivec2    g_mouse;
glm::ivec4    g_viewport;
glm::vec3     centers[arrsize(nodes_hierarchy)];
GLUquadric*   g_pquadratic = nullptr;

/**
* barycenter
* 
*/
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

/**
* transform_verts
*
*/
void transform_verts(glm::vec3* pverts, size_t count, const glm::mat4x4 &mat)
{
  for (size_t i = 0; i < count; i++)
    pverts[i] = mat * glm::vec4(pverts[i], 1.f);
}

/**
* find_node
*
*/
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
* get_node_index
*
*/
int get_node_index(const char* pname)
{
  for (int i = 0; i < arrsize(nodes_hierarchy); i++) {
    model_node* pnode = &nodes_hierarchy[i];
    if (!strcmp(pnode->get_name(), pname)) {
      return i;
    }
  }
  return -1;
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

/**
* change_coord_system
*
*/
void print_nodes_barycenter(const obj_importer* pimp)
{
  bbox_t AABB;
  glm::vec3 null_center;
  obj_importer::mesh* pmesh;
  model_node* pnode;
  assert(pimp->get_num_meshes() == arrsize(nodes_hierarchy) && "nodes hierarchy incompatible for this model!");
  printf("=== print_nodes_barycenter ===\n");
  for (size_t i = 0; i < pimp->get_num_meshes(); i++) {
    pmesh = pimp->get_mesh(i);
    pnode = &nodes_hierarchy[i];
    glm::vec3 null_center = barycenter(AABB, pmesh->get_verts(), pmesh->get_num_verts());
    printf("node %zd (%s)  local_center_after_shift(%.3f, %.3f, %.3f)  AABB{ min( %f %f %f ); max( %f %f %f ) }\n", 
      i, pnode->get_name(),
      null_center.x, null_center.y, null_center.z,
      AABB.vec_min.x, AABB.vec_min.y, AABB.vec_min.z, 
      AABB.vec_max.x, AABB.vec_max.y, AABB.vec_max.z);
  }
  printf("============================\n\n");
}

/**
* change_coord_system
*
*/
void change_coord_system(model_node* pnodes, int count, const obj_importer* pimp)
{
  for (int i = 0; i < count; i++) {
    model_node* pnode = &pnodes[i];
    obj_importer::mesh* pmesh = pimp->get_mesh(i);
    centers[i] = barycenter(pnode->get_bbox(), pmesh->get_verts(), pmesh->get_num_verts());
  }

  for (int i = 0; i < count; i++) {
    model_node* pnode = &pnodes[i];
    obj_importer::mesh* pmesh = pimp->get_mesh(i);
    int parent_id = pnode->get_parent_id();
    model_node* pparent = (parent_id >= 0 ? &pnodes[parent_id] : nullptr);
    if (strcmp(pnode->get_name(), "node3") == 0 && pparent) {
      glm::vec3 parent_min = pparent->get_bbox().vec_min;
      glm::vec3 parent_max = pparent->get_bbox().vec_max;
      glm::vec3 pend_min = pnode->get_bbox().vec_min;
      glm::vec3 pend_max = pnode->get_bbox().vec_max;
      // compute intersection
      glm::vec3 inter_min(
        glm::max(parent_min.x, pend_min.x),
        glm::max(parent_min.y, pend_min.y),
        glm::max(parent_min.z, pend_min.z)
      );

      glm::vec3 inter_max(
        glm::min(parent_max.x, pend_max.x),
        glm::min(parent_max.y, pend_max.y),
        glm::min(parent_max.z, pend_max.z)
      );

      // no empty intersection?
      if (inter_min.x <= inter_max.x && inter_min.y <= inter_max.y && inter_min.z <= inter_max.z) {
        centers[i] = glm::vec3(
          (inter_min.x + inter_max.x) * 0.5f, //X center of intersection
          inter_max.y, //intersection bound
          (inter_min.z + inter_max.z) * 0.5f //Z center of intersection
        );
      }
      else {
        /* print error */
        printf("Warning: %s bbox not intersects with %s!!!\n", pnode->get_name(), pparent->get_name());
        __debugbreak();
      }
    }

    // compute position of parent. (0,0,0) coord for root node
    pnode->set_position(pparent ? (centers[i] - centers[parent_id]) : glm::vec3(0.f, 0.f, 0.f));
    transform_verts(pmesh->get_verts(), pmesh->get_num_verts(), glm::translate(glm::mat4x4(1.f), -centers[i]));
  }
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

  g_viewport.x = 0;
  g_viewport.y = 0;
  g_viewport.z = width;
  g_viewport.w = height;
}

/**
* get_coord_from_depth
* 
*/
bool get_coord_from_depth(glm::vec3 &dst, glm::ivec4 viewport, glm::ivec2 coord)
{
  float depth;
  glm::mat4x4 modelview;
  glm::mat4x4 projection;
  glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
  glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));
  glReadPixels(coord.x, coord.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  if (depth == 1.f) 
    return false;

  glm::dvec3   dcoord;
  glm::dmat4x4 dmodelview(modelview);
  glm::dmat4x4 dprojection(projection);
  gluUnProject((double)coord.x, (double)coord.y, depth,
    glm::value_ptr(dmodelview),
    glm::value_ptr(dprojection),
    glm::value_ptr(viewport),
    &dcoord.x, &dcoord.y, &dcoord.z);
  dst = glm::vec3(dcoord);
  return true;
}

/**
* get_id_from_coord
* 
*/
int get_id_from_coord(glm::vec3 &world_pos, glm::ivec4 viewport, glm::ivec2 coord)
{
  GLubyte stencil_value;
#if 1
  /* inverse Y coord */
  coord.y = (viewport.w - 1) - coord.y;
#endif
  if (get_coord_from_depth(world_pos, viewport, coord)) {
    glReadPixels(coord.x, coord.y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencil_value);
    return static_cast<int>(stencil_value);
  }
  return -1;
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

    case SDL_MOUSEMOTION:
      g_mouse.x = event.motion.x;
      g_mouse.y = event.motion.y;
      printf("mouse motion: %d %d\n", g_mouse.x, g_mouse.y);
      break;

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
  const glm::vec3& pos = pnode->get_pos();
  glTranslatef(pos.x, pos.y, pos.z);
  const glm::vec3 angles = pnode->get_rotation();
  glRotatef(angles.x, 1.f, 0.f, 0.f);
  glRotatef(angles.y, 0.f, 1.f, 0.f);
  glRotatef(angles.z, 0.f, 0.f, 1.f);

  obj_importer::mesh* pmesh = obj.get_mesh(myid);
  glStencilFunc(GL_ALWAYS, myid, 0xFF);
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
  glStencilMask(0xFF); //enable stencil writing
  draw_recursive(0);
  glStencilMask(0x00); //disable writing
}

void draw_debug_sphere(glm::vec3 pos, glm::vec3 color)
{
  GLint old_depth_mode;
  GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
  if (lighting_enabled)
    glDisable(GL_LIGHTING);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glGetIntegerv(GL_DEPTH_FUNC, &old_depth_mode);
  glDepthFunc(GL_ALWAYS);
  glPushAttrib(GL_CURRENT_BIT);
  glColor3fv(glm::value_ptr(color));
  glTranslatef(pos.x, pos.y, pos.z);
  gluSphere(g_pquadratic, 0.5, 10, 10);
  glTranslatef(-pos.x, -pos.y, -pos.z);
  glPopAttrib();
  glDepthFunc(old_depth_mode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  if (lighting_enabled) {
    glEnable(GL_LIGHTING);
  }
}

void draw_centers()
{
  /* draw centers */
  GLint old_depth_mode;
  GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
  if (lighting_enabled)
    glDisable(GL_LIGHTING);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glGetIntegerv(GL_DEPTH_FUNC, &old_depth_mode);
  glDepthFunc(GL_ALWAYS);
  glPushAttrib(GL_CURRENT_BIT);
  glColor3f(0.5f, 0.5f, 0.5f);
  for (size_t i = 0; i < arrsize(centers); i++) {
    glm::vec3& center = centers[i];
    glTranslatef(center.x, center.y, center.z);
    gluSphere(g_pquadratic, 0.5, 10, 10);
    glTranslatef(-center.x, -center.y, -center.z);   
  }
  glPopAttrib();
  glDepthFunc(old_depth_mode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  if (lighting_enabled) {
    glEnable(GL_LIGHTING);
  }
}

void darw_axises()
{
  static const glm::vec3 lines[] = {
    { 0.f, 0.f, 0.f }, { 1.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f },
    { 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }
  };
  glPushMatrix();
  glLoadIdentity();
  glPushAttrib(GL_CURRENT_BIT);
  glColor3f(1.f, 0.f, 0.f);
  glVertexPointer(3, GL_FLOAT, 0, glm::value_ptr(lines[0]));
  glDrawArrays(GL_LINES, 0, 2);
  glColor3f(0.f, 1.f, 0.f);
  glVertexPointer(3, GL_FLOAT, 0, glm::value_ptr(lines[1]));
  glDrawArrays(GL_LINES, 0, 2);
  glColor3f(0.f, 0.f, 1.f);
  glVertexPointer(3, GL_FLOAT, 0, glm::value_ptr(lines[2]));
  glDrawArrays(GL_LINES, 0, 2);
  glPopAttrib();
  glPopMatrix();
}

bool stencil_buffer_available()
{
  GLenum status;
  GLubyte stencil_value;
  glReadPixels(0, 0, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencil_value);
  status = glGetError();
  if (status != GL_NO_ERROR) {
    if (status == GL_INVALID_OPERATION) {
      /* stencil buffer is invalid */
      printf("stencil buffer is not available\n");
      return false;
    }
    printf("stencil_buffer_available(): glReadPixels() returned error: %d\n", status);
    return false;
  }
  return true;
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

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);

  /* enable stencil buffer */
  printf("enabling stencil buffer...\n");
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glClearStencil(0);
  glClear(GL_STENCIL_BUFFER_BIT);
  if (!stencil_buffer_available()) {
    printf("stencil buffer not created correctly!\n");
    return 1;
  }

  glEnable(GL_LIGHTING);
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
  print_nodes_barycenter(&obj);

  model_node* ppendulum_node = find_node("node3");
  int         pendulum_node_idx = get_node_index("node3");
  assert(pendulum_node_idx != -1 && "node not found!");
  model_node* pload0 = find_node("node4");
  model_node* pload1 = find_node("node5");
  assert(ppendulum_node && "ppendulum_node was nullptr!");

  g_pquadratic = gluNewQuadric();

  glm::vec3 worldpos;
  glEnableClientState(GL_VERTEX_ARRAY);
  
  while (handle_events()) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    current_time = get_time();
    glTranslatef(0.f, -10.f, -60.f);
    glRotatef(180.f, 0.f, 1.f, 0.f);

    constexpr float deg = 5.f;
    float sinv = sinf(current_time);
    ppendulum_node->set_rotation({ 0.f, 0.f, sinv * deg});
    pload0->set_position({ 0.f, sinv * 10.f, 0.f });
    pload1->set_position({ 0.f, sinv * -10.f, 0.f });

    glEnableClientState(GL_NORMAL_ARRAY);
    draw_model();
    glDisableClientState(GL_NORMAL_ARRAY);
    //draw_centers();
    //draw_debug_sphere(centers[pendulum_node_idx], glm::vec3(0.f, 0.f, 1.f));

    //NOTE: K.D.  OBJECT PICKING HERE!  objectid==nodeid
    int objectid = get_id_from_coord(worldpos, g_viewport, g_mouse);
    if (objectid != -1) {
      bool is_load = objectid >= 4 && objectid <= 5;
      if (is_load) {
        draw_debug_sphere(worldpos, glm::vec3(1.f, 1.f, 0.f));
        printf("it is object: %d\n", objectid);
      }
      else if (objectid == pendulum_node_idx) {
        draw_debug_sphere(worldpos, glm::vec3(0.f, 0.f, 0.5f));
      }
    }
    SDL_GL_SwapWindow(pwindow);
  }
  glDisableClientState(GL_VERTEX_ARRAY);
  
  SDL_GL_DeleteContext(ctx);
  SDL_DestroyWindow(pwindow);
  SDL_Quit();
  return 0;
}