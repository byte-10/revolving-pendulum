#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <vector>
#include <string>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "gl_debug.h"
#include "wavefront_obj.h"
#include "node.h"
#include "banner.h"
#include "font.h"
#include "simulation.h"
#include "help.h"

#define arrsize(x) (sizeof(x) / sizeof(x[0]))

model_node nodes_hierarchy[] = {
  model_node(-1, "node0", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node1", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node2", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(2,  "node3", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node4", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3, "node5", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(0,  "node6", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node7", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(3,  "node8", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(1,  "node9_display", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(1,  "button1", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(1,  "button2", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(1,  "button3", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }),
  model_node(1,  "node13_text", { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f })
};

class pendulum_LCD
{
    int    m_width;
    int    m_height;
    GLuint m_display_texture;
public:
    pendulum_LCD(int w, int h) :
        m_width(w), m_height(h), m_display_texture(0) {
    }

    bool init() {
        GLenum error;
        assert(m_display_texture == 0 && "already created texture");
        glGenTextures(1, &m_display_texture);
        glBindTexture(GL_TEXTURE_2D, m_display_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            SDL_LogError(0, "pendulum_LCD::init(): glTexImage2D() failed with error %d (0x%x)!", error, error);
            return false;
        }
        printf("pendulum_LCD::init(): OK\n");
        return true;
    }

    void shutdown() {
        glDeleteTextures(1, &m_display_texture);
    }

    GLuint get_texid() const { return m_display_texture; }

    void update(gl_font& ss_font, gl_font& text_font, float sim_time) {
        char      buf[128];
        glm::vec2 metrics;
        glm::vec2 textpos;
        glm::vec2 region_size(m_width, m_height);
        constexpr glm::vec4 BACKGROUND_PANEL_COLOR(0.1f, 0.1f, 0.1f, 1.f);
        constexpr glm::vec4 BORDER_COLOR(0.5f, 0.5f, 0.5f, 1.f);

        glm::vec4  old_clear;
        glm::ivec4 old_viewport;
        glGetIntegerv(GL_VIEWPORT, glm::value_ptr(old_viewport));
        glViewport(0, 0, m_width, m_height);
        gl_font::begin_text(m_width, m_height);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, glm::value_ptr(old_clear));
        glClearColor(BACKGROUND_PANEL_COLOR.x, BACKGROUND_PANEL_COLOR.y, BACKGROUND_PANEL_COLOR.z, BACKGROUND_PANEL_COLOR.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ss_font.line_feed_mode(LF_X);
        int time_sec = static_cast<int>(sim_time);
        SDL_snprintf(buf, sizeof(buf), "%d:%.2d", time_sec / 60, time_sec % 60);
        ss_font.line_boundsf(metrics.x, metrics.y, "%s", buf);
        textpos.x = (region_size.x - metrics.x) / 2.f;
        textpos.y = (region_size.y - metrics.y) / 2.f;
        ss_font.move_to(textpos.x, textpos.y - metrics.y / 2.f);
        ss_font.draw_text(buf);

        glBindTexture(GL_TEXTURE_2D, m_display_texture);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, m_width, m_height, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        gl_font::end_text();
        glViewport(old_viewport.x, old_viewport.y, old_viewport.z, old_viewport.w);
        glClearColor(old_clear.x, old_clear.y, old_clear.z, old_clear.w);
    }


    void debug_draw(glm::vec2 pos) {
        static const struct vert_s {
            glm::vec3 pos;
            glm::vec2 uv;
        } verts[] = {
          { { pos.x,          pos.y,             0.f }, { 0.f, 1.f } },
          { { pos.x + m_width, pos.y,            0.f }, { 1.f, 1.f } },
          { { pos.x + m_width, pos.y + m_height, 0.f }, { 1.f, 0.f } },
          { { pos.x,          pos.y + m_height,  0.f }, { 0.f, 0.f } }
        };
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindTexture(GL_TEXTURE_2D, m_display_texture);
        glVertexPointer(3, GL_FLOAT, sizeof(vert_s), &verts[0].pos);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vert_s), &verts[0].uv);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
};

// animator.h
class animator
{
public:
  enum : uint32_t {
    AS_LIFT = 0,
    AS_SLIDE_RIGHT,
    AS_ROTATION,
    AS_SLIDE_LEFT,
    AS_DROP,
    AS_IDLE,
    AS_COUNT = AS_IDLE
  };

  animator(int pendulumNodeIndex=-1)
    : m_state(AS_IDLE)
    , m_pendulumID(pendulumNodeIndex)
    , m_animStart(0.0f)
    , m_animDuration(0.0f)
    , m_progress(0.0f)
  {
    // задаём длительности фаз (в сек)
    set_duration(AS_LIFT, 0.2f);
    set_duration(AS_SLIDE_RIGHT, 0.2f);
    set_duration(AS_ROTATION, 0.4f);
    set_duration(AS_SLIDE_LEFT, 0.2f);
    set_duration(AS_DROP, 0.2f);
  }

  // Запустить анимацию «переворота»
  void start() {
    if (m_state == AS_IDLE) {
      transitionTo(AS_LIFT, 0.0f);
    }
  }

  inline void set_pendulum_node_id(int id) {
    m_pendulumID = id;
  }

  // Вызывать каждый кадр, до draw_recursive
  void update(float now) {
    assert(m_pendulumID != -1 && "m_pendulumID is not set!");
    if (m_state == AS_IDLE)
      return;

    float t = now - m_animStart;
    if (t >= m_animDuration) {
      float overflow = t - m_animDuration;
      if (m_state == AS_DROP) {
        m_state = AS_IDLE;
        m_progress = 0.0f;
        return;
      }
      transitionTo(m_state + 1, overflow);
      t = now - m_animStart;
    }
    m_progress = (m_animDuration > 0.0f ? t / m_animDuration : 1.0f);
  }

  // Главный метод: применяет все нужные glTranslatef/glRotatef
  // для данного узла. После этого нужно сразу рисовать меш.
  void applyTransforms(int nodeID, model_node& node) const {
    // 1) обычный сдвиг относительно родителя
    const glm::vec3& pos = node.get_pos();
    glTranslatef(pos.x, pos.y, pos.z);

    // 2) если это наш маятник и он анимируется → подъём/сдвиг
    if (nodeID == m_pendulumID && m_state != AS_IDLE) {
      glTranslatef(slide(), lift(), 0.0f);
    }

    // 3) поворот: основной угол из node + анимированный дополнительный
    float baseZ = node.get_rotation().z;
    float extra = (nodeID == m_pendulumID ? rotation() : 0.0f);
    glRotatef(baseZ + extra, 0.0f, 0.0f, 1.0f);

    // 4) «откат» подъёма/слайда, чтобы меш рисовался из своего barycenter
    if (nodeID == m_pendulumID && m_state != AS_IDLE) {
      glTranslatef(-slide(), -lift(), 0.0f);
    }
  }

  bool isAnimating() const { return m_state != AS_IDLE; }

private:
  uint32_t m_state;
  int     m_pendulumID;   // индекс node3
  float   m_animStart;
  float   m_animDuration;
  float   m_progress;
  float   m_durations[AS_COUNT];

  static constexpr float kLiftAmount = 2.0f;
  static constexpr float kSlideAmount = 5.0f;
  static constexpr float kRotateAmount = 180.0f;

  void set_duration(uint32_t s, float d) {
    if (s >= 0 && s < AS_COUNT) m_durations[s] = d;
  }

  void transitionTo(uint32_t s, float overflow) {
    m_state = s;
    m_animDuration = (s == AS_IDLE ? 0.0f : m_durations[s]);
    m_animStart = getTime() - overflow;
    m_progress = 0.0f;
  }

  static float getTime() {
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 cnt = SDL_GetPerformanceCounter();
    return float(cnt) / float(freq);
  }
public:
  // вспомогательные для applyTransforms
  float lift() const {
    switch (m_state) {
    case AS_LIFT:        return   kLiftAmount * m_progress;
    case AS_SLIDE_RIGHT:
    case AS_ROTATION:
    case AS_SLIDE_LEFT:  return   kLiftAmount;
    case AS_DROP:        return   kLiftAmount * (1.0f - m_progress);
    default:             return 0.0f;
    }
  }
  float slide() const {
    switch (m_state) {
    case AS_SLIDE_RIGHT: return   kSlideAmount * m_progress;
    case AS_ROTATION:    return   kSlideAmount;
    case AS_SLIDE_LEFT:  return   kSlideAmount * (1.0f - m_progress);
    default:             return 0.0f;
    }
  }
  float rotation() const {
    switch (m_state) {
    case AS_ROTATION:    return   kRotateAmount * m_progress;
    case AS_SLIDE_LEFT:
    case AS_DROP:        return   kRotateAmount;
    default:             return 0.0f;
    }
  }
};

enum SIM_STATE : uint32_t {
    SIM_STATE_IDLE = 0,
    SIM_STATE_SUMULATING,
    SIM_STATE_STOPPED
};

enum DEBUG_DRAW {
  DD_NONE = 0,
  DD_BOUNDS = 1 << 0
};

enum {
  PIVOT_TOP=0,
  PIVOT_BOTTOM,
  PIVOTS_COUNT
};

SDL_Window*    g_pwindow = nullptr;
SDL_GLContext  g_ctx = nullptr;
obj_importer   g_obj;
float          g_current_time;
float          g_last_time;
float          g_delta_time;
float          g_FPS;
float          g_frametime_begin;
float          g_frametime;
float          g_simtime_begin;
float          g_simtime;
float          g_simulation_time = 0.f;
float          g_simulation_start_time = 0.f;
float          g_deviation_deg;
glm::ivec2     g_mouse;
glm::ivec2     g_moused;
glm::ivec4     g_viewport;
glm::vec3      g_centers[arrsize(nodes_hierarchy)];
GLUquadric*    g_pquadratic = nullptr;
gl_font        g_msg_font;
gl_font        g_panel_font;
pendulum_LCD   g_LCD(200, 100);
int            g_node6_measure_idx;
int            g_pendulum_node_idx;
int            g_load0idx;
int            g_load1idx;
int            g_display_idx;
int            g_button1_idx;
int            g_button2_idx;
int            g_button3_idx;
SDL_Cursor*    g_ppointer_cursor = nullptr;
SDL_Cursor*    g_parrow_cursor = nullptr;
SDL_Cursor*    g_phand_cursor = nullptr;
SDL_Cursor*    g_pcurr_cursor = nullptr;
bool           g_mouse_pressed = false;
bool           g_node_selected = false;
int            g_pendulum_reversed = 0;
pendulum_physp g_pendulum_phys;
SIM_STATE      g_sim_state = SIM_STATE_IDLE;
int            g_debug_draw = DD_NONE;
glm::vec3      g_pivot_offset;
animator       g_animator;
bool           g_bis_reversed = false;
bool           g_bwas_animating = false;

template<int light_index>
class gl_light
{
public:
    void enable() { glEnable(GL_LIGHT0 + light_index); }
    void disable() { glDisable(GL_LIGHT0 + light_index); }
    void set_colors(glm::vec4 light_diffuse, glm::vec4 light_specular) {
        glLightfv(GL_LIGHT0 + light_index, GL_DIFFUSE, glm::value_ptr(light_diffuse));
        glLightfv(GL_LIGHT0 + light_index, GL_SPECULAR, glm::value_ptr(light_specular));
    }
    void set_pos(glm::vec3& pos) { glLightfv(GL_LIGHT0 + light_index, GL_POSITION, glm::value_ptr(pos)); }
    void set_pos(glm::vec4 pos) { glLightfv(GL_LIGHT0 + light_index, GL_POSITION, glm::value_ptr(pos)); }
};

glm::vec3 barycenter(bbox_t& dst, const glm::vec3* pverts, size_t count)
{
    glm::vec3 total(0.f, 0.f, 0.f);
    dst.init();
    for (size_t i = 0; i < count; i++) {
        total += pverts[i];
        dst.update(pverts[i]);
    }
    return total / float(count);
}

void transform_verts(glm::vec3* pverts, size_t count, const glm::mat4x4& mat)
{
    for (size_t i = 0; i < count; i++)
        pverts[i] = mat * glm::vec4(pverts[i], 1.f);
}

model_node* find_node(const char* pname)
{
    for (auto& pnode : nodes_hierarchy) {
        if (!strcmp(pnode.get_name(), pname)) {
            return &pnode;
        }
    }
    return nullptr;
}

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

void prepare_nodes(model_node* pnodes, int count)
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

void change_coord_system(model_node* pnodes, int count, const obj_importer* pimp)
{
  // 1) Считаем barycenter всех узлов до трансформаций
  for (int i = 0; i < count; i++) {
    obj_importer::mesh* pmesh = pimp->get_mesh(i);
    g_centers[i] = barycenter(pnodes[i].get_bbox(), pmesh->get_verts(), pmesh->get_num_verts());
  }

  int pi = get_node_index("node3");
  int parent = nodes_hierarchy[pi].get_parent_id();
  bbox_t& pb = nodes_hierarchy[parent].get_bbox();
  bbox_t& mb = nodes_hierarchy[pi].get_bbox();

  glm::vec3 inter_min(glm::max(pb.vec_min, mb.vec_min));
  glm::vec3 inter_max(glm::min(pb.vec_max, mb.vec_max));
  glm::vec3 v_pivot(
    (inter_min.x + inter_max.x) * 0.5f,
    inter_max.y,
    (inter_min.z + inter_max.z) * 0.5f
  );

  // вектора от barycenter маятника до центров этих двух узлов
  g_pivot_offset = v_pivot - g_centers[pi];

  // 3) Применяем позиционирование и сдвиг вершин
  for (int i = 0; i < count; i++) {
    model_node* pnode = &pnodes[i];
    obj_importer::mesh* pmesh = pimp->get_mesh(i);
    int pid = pnode->get_parent_id();
    glm::vec3 delta = (pid >= 0) ? (g_centers[i] - g_centers[pid]) : glm::vec3(0.0f);
    pnode->set_position(delta);
    // Сдвигаем вершины так, чтобы pivot (g_centers[i]) оказался в локальном (0,0,0)
    transform_verts(
      pmesh->get_verts(),
      pmesh->get_num_verts(),
      glm::translate(glm::mat4(1.0f), -g_centers[i])
    );
    // Пересчитаем bbox под новые вершины
    bbox_t newbb;
    barycenter(newbb, pmesh->get_verts(), pmesh->get_num_verts());
    pnode->get_bbox() = newbb;
  }
}

float get_time()
{
    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 counter = SDL_GetPerformanceCounter();
    return counter / (double)freq;
}

void update_viewport(int width, int height)
{
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

bool get_coord_from_depth(glm::vec3& dst, glm::ivec4 viewport, glm::ivec2 coord)
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

int get_id_from_coord(glm::vec3& world_pos, glm::ivec4 viewport, glm::ivec2 coord)
{
    GLubyte stencil_value;
    coord.y = (viewport.w - 1) - coord.y;
    if (get_coord_from_depth(world_pos, viewport, coord)) {
        glReadPixels(coord.x, coord.y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencil_value);
        return static_cast<int>(stencil_value);
    }
    return -1;
}

void handle_keys(const SDL_Keysym& key)
{
    printf("key.sym=%d\n", key.sym);
    switch (key.sym)
    {
    case SDLK_RETURN:
        if (g_sim_state != SIM_STATE_SUMULATING) {
            nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, g_deviation_deg });
            printf("state changed to SIM_STATE_SUMULATING with deviation %.2f deg\n", g_deviation_deg);
            pendulum_phys_init_default(g_pendulum_phys, g_deviation_deg);
            g_simulation_start_time = get_time();
            g_simulation_time = 0.f;
            g_sim_state = SIM_STATE_SUMULATING;
        }
        break;

    case SDLK_ESCAPE:
        if (g_sim_state == SIM_STATE_SUMULATING) {
            printf("state changed to SIM_STATE_STOPPED\n");
            g_sim_state = SIM_STATE_STOPPED;
            return;
        }
        if (g_sim_state == SIM_STATE_STOPPED) {
            printf("state changed to SIM_STATE_IDLE\n");
            g_sim_state = SIM_STATE_IDLE;
            g_deviation_deg = 0.f;
            g_simulation_time = 0.f;
            nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, g_deviation_deg });
            pendulum_phys_init_default(g_pendulum_phys, g_deviation_deg);
            return;
        }
        break;

    case SDLK_F1:
        show_help();
        break;

    case SDLK_F2:
        g_debug_draw ^= DD_BOUNDS;
        printf("Debug bounds %d\n", !!((g_debug_draw & DD_BOUNDS) == DD_BOUNDS));
        break;

    case SDLK_r:
      if (g_sim_state == SIM_STATE_IDLE && !g_animator.isAnimating()) {
        printf("Animation started\n");
        g_bwas_animating = true;
        g_animator.start();
      }
      break;

    default:
        break;
    }
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
                b_active = false;
                break;

            default:
                break;
            }
            break;
        }

        case SDL_MOUSEMOTION:
            g_mouse.x = event.motion.x;
            g_mouse.y = event.motion.y;
            g_moused.x = event.motion.xrel;
            g_moused.y = event.motion.yrel;
            printf("mouse(%d %d)  moused(%d %d)\n",
                g_mouse.x, g_mouse.y,
                g_moused.x, g_moused.y);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
                g_mouse_pressed = true;
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
                g_mouse_pressed = false;
            break;

        case SDL_KEYDOWN:
            handle_keys(event.key.keysym);
            break;

        default:
            break;
        }
    }
    return b_active;
}

void draw_bbox(const bbox_t& src, glm::vec3 color)
{
  bool light_enabled = glIsEnabled(GL_LIGHTING);
  if (light_enabled)
    glDisable(GL_LIGHTING);

  glPushAttrib(GL_CURRENT_BIT);
  glColor3fv(glm::value_ptr(color));

  const glm::vec3& vmin = src.vec_min;
  const glm::vec3& vmax = src.vec_max;

  // 8 вершин коробки
  glm::vec3 c[8] = {
    {vmin.x, vmin.y, vmin.z},
    {vmax.x, vmin.y, vmin.z},
    {vmax.x, vmax.y, vmin.z},
    {vmin.x, vmax.y, vmin.z},
    {vmin.x, vmin.y, vmax.z},
    {vmax.x, vmin.y, vmax.z},
    {vmax.x, vmax.y, vmax.z},
    {vmin.x, vmax.y, vmax.z},
  };

  // рёбра (каждое ребро — пара индексов в массиве c)
  static const int edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // нижнее основание
    {4,5}, {5,6}, {6,7}, {7,4}, // верхнее основание
    {0,4}, {1,5}, {2,6}, {3,7}  // вертикальные рёбра
  };

  glBegin(GL_LINES);
  for (int i = 0; i < 12; ++i) {
    const glm::vec3& a = c[edges[i][0]];
    const glm::vec3& b = c[edges[i][1]];
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
  }
  glEnd();

  glPopAttrib();
  if (light_enabled)
    glEnable(GL_LIGHTING);
}

void draw_recursive(int myid)
{
    GLenum error;
    if (myid == -1)
        return;
    if (myid == g_node6_measure_idx)
        return;

    glPushMatrix();

    model_node* pnode = &nodes_hierarchy[myid];
    const glm::vec3& pos = pnode->get_pos();
    const glm::vec3  angles = pnode->get_rotation();

    if (myid == g_pendulum_node_idx) {
      // 1) смещаемся в позицию pendulum относительно parent
      glTranslatef(pos.x, pos.y, pos.z);

      // 2) поднимаем–опускаем pivot (верхний или нижний) 
      const glm::vec3& pivot = g_pivot_offset;
      glTranslatef(pivot.x, pivot.y, pivot.z);

      // 3) анимационный подъём/слайд
      if (g_animator.isAnimating()) {
        float lift = g_animator.lift();
        float slide = g_animator.slide();
        glTranslatef(slide, lift, 0.0f);
      }

      // 3) вращаем ВСЕГДА вокруг этой же точки: 
      //    базовый угол из physics + anim-угол + 180° при flipped
      float baseZ = angles.z;
      float extra = g_animator.isAnimating() ? g_animator.rotation() : 0.0f;
      float flip = g_bis_reversed ? 180.0f : 0.0f;
      glRotatef(baseZ + extra + flip, 0.0f, 0.0f, 1.0f);

      // 5) откатываем подъём/слайд
      if (g_animator.isAnimating()) {
        float lift = g_animator.lift();
        float slide = g_animator.slide();
        glTranslatef(-slide, -lift, 0.0f);
      }

      // 6) возвращаемся из pivot
      glTranslatef(-pivot.x, -pivot.y, -pivot.z);

      // 7) если перевёрнут — смещаем весь стержень вниз,
      //    чтобы он снова свисал из точки подвеса
      if (g_bis_reversed) {
        // именно 2 * pivot.y, потому что после flip
        // центр модели поднялся ровно на pivot.y
        glTranslatef(0.0f, 2.0f * g_pivot_offset.y, 0.0f);
      }
    }
    else {
      // остальная иерархия рендерится без этих спец-смещений
      glTranslatef(pos.x, pos.y, pos.z);
      glRotatef(angles.x, 1, 0, 0);
      glRotatef(angles.y, 0, 1, 0);
      glRotatef(angles.z, 0, 0, 1);
    }

    obj_importer::mesh* pmesh = g_obj.get_mesh(myid);

    const model_node_material& mat = pnode->get_material();
    glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(mat.get_ambient()));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(mat.get_diffuse()));
    glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(mat.get_specular()));
    glMaterialf(GL_FRONT, GL_SHININESS, mat.get_shininess());

    if (g_debug_draw & DD_BOUNDS)
      draw_bbox(pnode->get_bbox(), { 0.f, 1.f, 0.f });

    glStencilFunc(GL_ALWAYS, myid, 0xFF);

    glVertexPointer(3, GL_FLOAT, 0, pmesh->get_verts());
    glNormalPointer(GL_FLOAT, 0, pmesh->get_normals());
    if (pnode->is_texture_available()) {
        glBindTexture(GL_TEXTURE_2D, pnode->get_texid());
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, pmesh->get_texcoords());
    }

    glDrawArrays(GL_TRIANGLES, 0, pmesh->get_num_verts());
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("glGetError returned error: %d (0x%x)!\n",
            error, error);
    }

    if (pnode->is_texture_available()) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    for (int i = 0; i < pnode->get_num_childs(); i++) {
        int child_id = pnode->get_child(i);
        assert(child_id != -1 && "child loop");
        draw_recursive(child_id);
    }
    glPopMatrix();
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
    GLint old_depth_mode;
    GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
    if (lighting_enabled)
        glDisable(GL_LIGHTING);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glGetIntegerv(GL_DEPTH_FUNC, &old_depth_mode);
    glDepthFunc(GL_ALWAYS);
    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(0.5f, 0.5f, 0.5f);
    for (size_t i = 0; i < arrsize(g_centers); i++) {
        glm::vec3& center = g_centers[i];
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
    printf("stencil_buffer_available(): process checking stencil...\n");
    glReadPixels(0, 0, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencil_value);
    status = glGetError();
    if (status != GL_NO_ERROR) {
        if (status == GL_INVALID_OPERATION) {
            printf("stencil buffer is not available\n");
            return false;
        }
        printf("stencil_buffer_available(): glReadPixels() returned error: %d\n", status);
        return false;
    }
    return true;
}

void center_load_disks()
{
    printf("center_load_disks(): preparing centering...\n");
    model_node* pldisk1 = find_node("node4");
    model_node* pldisk2 = find_node("node5");
    assert(pldisk1 && "node4 not found");
    assert(pldisk2 && "node5 not found");
    assert(pldisk1->get_parent_id() == pldisk2->get_parent_id() && "disks parent is different!");
    model_node* pparent = &nodes_hierarchy[pldisk2->get_parent_id()];
}

void setup_pendulum_materials()
{
    printf("setup_pendulum_materials(): preparing material parameters...\n");
    nodes_hierarchy[0].set_material({ {0.08f, 0.08f, 0.08f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, {0.50f, 0.50f, 0.50f, 1.0f}, 40.0f });
    nodes_hierarchy[1].set_material({ {0.05f, 0.05f, 0.20f, 1.0f}, {0.15f, 0.15f, 0.50f, 1.0f}, {0.45f, 0.45f, 0.90f, 1.0f}, 95.0f });
    nodes_hierarchy[2].set_material({ {0.12f, 0.12f, 0.10f, 1.0f}, {0.35f, 0.35f, 0.30f, 1.0f}, {0.65f, 0.65f, 0.60f, 1.0f}, 70.0f });
    nodes_hierarchy[3].set_material({ {0.15f, 0.15f, 0.15f, 1.0f}, {0.50f, 0.50f, 0.50f, 1.0f}, {0.85f, 0.85f, 0.85f, 1.0f}, 120.0f });
    nodes_hierarchy[4].set_material({ {0.15f, 0.12f, 0.10f, 1.0f}, {0.45f, 0.40f, 0.35f, 1.0f}, {0.70f, 0.65f, 0.60f, 1.0f}, 80.0f });
    nodes_hierarchy[5].set_material({ {0.10f, 0.08f, 0.06f, 1.0f}, {0.35f, 0.30f, 0.25f, 1.0f}, {0.65f, 0.60f, 0.55f, 1.0f}, 85.0f });
    nodes_hierarchy[6].set_material({ {0.20f, 0.20f, 0.20f, 1.0f}, {0.45f, 0.45f, 0.45f, 1.0f}, {0.35f, 0.35f, 0.35f, 1.0f}, 50.0f });
    nodes_hierarchy[7].set_material({ {0.10f, 0.10f, 0.10f, 1.0f},{0.50f, 0.50f, 0.50f, 1.0f},{0.95f, 0.95f, 0.95f, 1.0f},160.0f });
    nodes_hierarchy[8].set_material({ {0.10f, 0.10f, 0.10f, 1.0f}, {0.30f, 0.30f, 0.30f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, 25.0f });
    nodes_hierarchy[10].set_material({ {0.5f, 0.1f, 0.1f, 1.0f}, {0.5f, 0.1f, 0.1f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, 25.0f });
    nodes_hierarchy[11].set_material({ {0.5f, 0.1f, 0.1f, 1.0f}, {0.5f, 0.1f, 0.1f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, 25.0f });
    nodes_hierarchy[12].set_material({ {0.5f, 0.1f, 0.1f, 1.0f}, {0.5f, 0.1f, 0.1f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, 25.0f });
    nodes_hierarchy[13].set_material({ {0.5f, 0.1f, 0.1f, 1.0f}, {0.5f, 0.1f, 0.1f, 1.0f}, {0.25f, 0.25f, 0.25f, 1.0f}, 25.0f });
}

bool init_fonts()
{
    printf("init_fonts(): loading fonts...\n");
    bool status = true;
    status &= g_msg_font.load_ttf("Courier-Bold.ttf", 18.f);
    status &= g_panel_font.load_ttf("sevensegment.ttf", 60.f);
    return status;
}

void update_time()
{
  g_current_time = get_time();
  g_delta_time = g_current_time - g_last_time;
  g_last_time = g_current_time;

  constexpr float delta_time_min_threshold = 0.00001f;
  if (g_delta_time < delta_time_min_threshold)
      g_delta_time = delta_time_min_threshold;

  g_FPS = 1.f / g_delta_time;

  if (g_sim_state == SIM_STATE_SUMULATING) {
      g_simulation_time = g_current_time - g_simulation_start_time;
  }

  g_animator.update(g_current_time);
  if (g_bwas_animating && !g_animator.isAnimating()) {
    g_bis_reversed = !g_bis_reversed;
  }
  g_bwas_animating = g_animator.isAnimating();
}

struct DiskDistances {
    float center_distance;
    float end_distance;
};

DiskDistances calculate_disks_distance()
{
    glm::vec3 pos_load0 = nodes_hierarchy[g_load0idx].get_pos() + g_centers[g_load0idx];
    glm::vec3 pos_load1 = nodes_hierarchy[g_load1idx].get_pos() + g_centers[g_load1idx];

    // Get bounding boxes to find disk ends (assuming y-axis is the vertical axis)
    bbox_t bbox_load0 = nodes_hierarchy[g_load0idx].get_bbox();
    bbox_t bbox_load1 = nodes_hierarchy[g_load1idx].get_bbox();

    // Transform bbox min/max to world coordinates
    glm::vec3 load0_min = pos_load0 + bbox_load0.vec_min;
    glm::vec3 load0_max = pos_load0 + bbox_load0.vec_max;
    glm::vec3 load1_min = pos_load1 + bbox_load1.vec_min;
    glm::vec3 load1_max = pos_load1 + bbox_load1.vec_max;

    // Assuming disks are aligned along y-axis, use bottom ends (min y)
    glm::vec3 load0_end = glm::vec3(pos_load0.x, load0_min.y, pos_load0.z);
    glm::vec3 load1_end = glm::vec3(pos_load1.x, load1_min.y, pos_load1.z);

    DiskDistances distances;
    distances.center_distance = glm::distance(pos_load0, pos_load1);
    distances.end_distance = glm::distance(load0_end, load1_end);
    return distances;
}

float calculate_deviation_angle()
{
    return nodes_hierarchy[g_pendulum_node_idx].get_rotation().z;
}

void draw_deviation_angle()
{
    GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
    if (lighting_enabled)
        glDisable(GL_LIGHTING);

    glPushMatrix();
    glm::vec3 pivot = g_centers[g_pendulum_node_idx] + g_pivot_offset;
    glTranslatef(pivot.x, pivot.y, pivot.z);

    // Draw vertical reference line (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -5.0f, 0.0f);
    glEnd();

    // Draw pendulum line (blue)
    float angle_deg = calculate_deviation_angle();
    float angle_rad = glm::radians(angle_deg);
    float len = 5.0f;
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(len * sinf(angle_rad), -len * cosf(angle_rad), 0.0f);
    glEnd();

    // Draw angle arc (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    const int segments = 20;
    float radius = 2.0f;
    float start_angle = 0.0f;
    float end_angle = angle_rad;
    if (angle_deg < 0.0f) {
        start_angle = angle_rad;
        end_angle = 0.0f;
    }
    for (int i = 0; i <= segments; ++i) {
        float theta = start_angle + (end_angle - start_angle) * (float)i / segments;
        glVertex3f(radius * sinf(theta), -radius * cosf(theta), 0.0f);
    }
    glEnd();

    // Draw angle value near the arc
    gl_font::begin_text(g_viewport.z, g_viewport.w);
    g_msg_font.set_color(255, 255, 255, 255);
    float text_x = pivot.x + radius * 1.2f * sinf(angle_rad * 0.5f);
    float text_y = pivot.y - radius * 1.2f * cosf(angle_rad * 0.5f);
    g_msg_font.move_to(text_x * 10.0f + g_viewport.z * 0.5f, -text_y * 10.0f + g_viewport.w * 0.5f); // Scale to screen coords
    g_msg_font.draw_textf("%.2f deg", fabs(angle_deg));
    gl_font::end_text();

    glPopMatrix();

    if (lighting_enabled)
        glEnable(GL_LIGHTING);
}

void draw_disks_distances()
{
    GLboolean lighting_enabled = glIsEnabled(GL_LIGHTING);
    if (lighting_enabled)
        glDisable(GL_LIGHTING);

    glm::vec3 pos_load0 = nodes_hierarchy[g_load0idx].get_pos() + g_centers[g_load0idx];
    glm::vec3 pos_load1 = nodes_hierarchy[g_load1idx].get_pos() + g_centers[g_load1idx];

    bbox_t bbox_load0 = nodes_hierarchy[g_load0idx].get_bbox();
    bbox_t bbox_load1 = nodes_hierarchy[g_load1idx].get_bbox();

    glm::vec3 load0_min = pos_load0 + bbox_load0.vec_min;
    glm::vec3 load1_min = pos_load1 + bbox_load1.vec_min;

    glm::vec3 load0_end = glm::vec3(pos_load0.x, load0_min.y, pos_load0.z);
    glm::vec3 load1_end = glm::vec3(pos_load1.x, load1_min.y, pos_load1.z);

    DiskDistances distances = calculate_disks_distance();

    // Draw line between centers (yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3fv(glm::value_ptr(pos_load0));
    glVertex3fv(glm::value_ptr(pos_load1));
    glEnd();

    // Draw center distance text
    glm::vec3 mid_center = (pos_load0 + pos_load1) * 0.5f;
    gl_font::begin_text(g_viewport.z, g_viewport.w);
    g_msg_font.set_color(255, 255, 255, 255);
    g_msg_font.move_to(mid_center.x * 10.0f + g_viewport.z * 0.5f, -mid_center.y * 10.0f + g_viewport.w * 0.5f);
    g_msg_font.draw_textf("%.2f units", distances.center_distance);
    gl_font::end_text();

    // Draw line between ends (cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3fv(glm::value_ptr(load0_end));
    glVertex3fv(glm::value_ptr(load1_end));
    glEnd();

    // Draw end distance text
    glm::vec3 mid_end = (load0_end + load1_end) * 0.5f;
    gl_font::begin_text(g_viewport.z, g_viewport.w);
    g_msg_font.set_color(255, 255, 255, 255);
    g_msg_font.move_to(mid_end.x * 10.0f + g_viewport.z * 0.5f, -mid_end.y * 10.0f + g_viewport.w * 0.5f);
    g_msg_font.draw_textf("%.2f units", distances.end_distance);
    gl_font::end_text();

    if (lighting_enabled)
        glEnable(GL_LIGHTING);
}

void draw_debug_overlay(float width, float height)
{
    DiskDistances distances = calculate_disks_distance();
    g_msg_font.line_feed_mode(LF_X);
    g_msg_font.move_to(10.f, 0.f);
    g_msg_font.set_color(255, 255, 255, 255);
    g_msg_font.draw_textf("Debug overlay enabled");
    g_msg_font.draw_textf("FPS: %.1f (last frametime: %f | curr frametime: %f)",
        g_FPS, g_frametime_begin, g_frametime);
    g_msg_font.draw_textf("delta time: %f", g_delta_time);
    g_msg_font.draw_textf("simulation time: %.2f s", g_simulation_time);
    g_msg_font.draw_textf("deviation angle: %.2f deg", calculate_deviation_angle());
    g_msg_font.draw_textf("disks center distance: %.2f units", distances.center_distance);
    g_msg_font.draw_textf("disks end distance: %.2f units", distances.end_distance);
    g_msg_font.move_rel_to(0.f, 10.f);

    g_msg_font.draw_text("------ physics simulation ------");
    pendulum_physics_print(g_pendulum_phys, g_msg_font);
}

void draw_pendulum()
{
    g_frametime_begin = g_current_time;
    glEnable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glPushAttrib(GL_CURRENT_BIT);
    glStencilMask(0xFF);
    draw_recursive(0);
    glStencilMask(0x00);
    glPopAttrib();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LIGHTING);

    draw_deviation_angle();
    draw_disks_distances();

    g_frametime = g_current_time - g_frametime_begin;
}

bool init_node_indices_and_cursors()
{
    printf("init_movable_nodes(): finding movable nodes...\n");
    g_pendulum_node_idx = get_node_index("node3");
    g_load0idx = get_node_index("node4");
    g_load1idx = get_node_index("node5");
    g_display_idx = get_node_index("node9_display");
    g_node6_measure_idx = get_node_index("node6");
    g_button1_idx = get_node_index("button1");
    g_button2_idx = get_node_index("button2");
    g_button3_idx = get_node_index("button3");

    g_parrow_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    g_phand_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    g_ppointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND); // Указатель

    g_animator.set_pendulum_node_id(g_pendulum_node_idx);

    if (!(g_parrow_cursor && g_phand_cursor && g_ppointer_cursor)) {
        printf("init_movable_nodes(): cursors initializing failed!");
        return false;
    }

    return g_pendulum_node_idx != -1 &&
        g_load0idx != -1 &&
        g_display_idx != -1 &&
        g_node6_measure_idx != -1 &&
        g_load1idx != -1 &&
        g_button1_idx != -1 &&
        g_button2_idx != -1 &&
        g_button3_idx != -1;
}

void set_cursor(SDL_Cursor* pcur)
{
    if (g_pcurr_cursor != pcur) {
        SDL_SetCursor(pcur);
        g_pcurr_cursor = pcur;
    }
}

constexpr float LOAD0_MIN_REL_Y = -10.0f;
constexpr float LOAD0_MAX_REL_Y = 0.0f;
constexpr float LOAD1_MIN_REL_Y = -10.0f;
constexpr float LOAD1_MAX_REL_Y = 0.f;
constexpr float DRAG_SENSITIVITY = 0.5f;
constexpr float PENDULUM_MIN_ANGLE = -90.0f;
constexpr float PENDULUM_MAX_ANGLE = 90.0f;
constexpr float ANGLE_SENSITIVITY = 0.1f;

void update_hover_cursor()
{
    if (g_sim_state == SIM_STATE_SUMULATING) {
        set_cursor(g_parrow_cursor);
        return;
    }

    glm::vec3 world_pos;
    int id = get_id_from_coord(world_pos, g_viewport, g_mouse);

    // Проверяем наведение на интерактивные элементы
    if (id == g_pendulum_node_idx ||
        id == g_load0idx ||
        id == g_load1idx ||
        id == g_button1_idx ||
        id == g_button2_idx ||
        id == g_button3_idx) {
        set_cursor(g_ppointer_cursor);
    }
    else {
        set_cursor(g_parrow_cursor);
    }
}

void process_movable_nodes()
{
    static bool dragging = false;
    static int dragging_id = -1;
    static glm::vec2 mouse_down;
    static glm::vec3 node_down_pos;
    static float pendulum_down_angle = 0.f;

    // Обновляем курсор при наведении
    if (!dragging) {
        update_hover_cursor();
    }

    if (g_mouse_pressed && !dragging) {
        glm::vec3 world_pos;
        int id = get_id_from_coord(world_pos, g_viewport, g_mouse);

        // Обработка кликов по всей области кнопок
        if (id == g_button1_idx) {
            // Кнопка "Start" - работает при клике в любом месте текстуры
            if (g_sim_state == SIM_STATE_IDLE) {
                nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, g_deviation_deg });
                printf("Start button pressed\n");
                pendulum_phys_init_default(g_pendulum_phys, g_deviation_deg);
                g_simulation_start_time = get_time();
                g_simulation_time = 0.f;
                g_sim_state = SIM_STATE_SUMULATING;
            }
            return;
        }
        else if (id == g_button2_idx) {
            // Кнопка "Stop"
            if (g_sim_state == SIM_STATE_SUMULATING) {
                printf("Stop button pressed\n");
                g_sim_state = SIM_STATE_STOPPED;
            }
            return;
        }
        else if (id == g_button3_idx) {
            // Кнопка "Reset"
            printf("Reset button pressed\n");
            g_sim_state = SIM_STATE_IDLE;
            g_deviation_deg = 0.f;
            g_simulation_time = 0.f;
            nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, g_deviation_deg });
            pendulum_phys_init_default(g_pendulum_phys, g_deviation_deg);
            return;
        }

        // Обработка перемещаемых элементов
        if (id == g_load0idx || id == g_load1idx || id == g_pendulum_node_idx) {
            dragging = true;
            dragging_id = id;
            mouse_down = glm::vec2(g_mouse);
            if (id == g_load0idx || id == g_load1idx) {
                node_down_pos = nodes_hierarchy[id].get_pos();
            }
            else if (id == g_pendulum_node_idx) {
                pendulum_down_angle = g_deviation_deg;
            }
            set_cursor(g_phand_cursor);
        }
    }
    else if (!g_mouse_pressed && dragging) {
        if (dragging_id == g_pendulum_node_idx) {
            g_deviation_deg = nodes_hierarchy[g_pendulum_node_idx].get_rotation().z;
        }
        dragging = false;
        dragging_id = -1;
    }

    // Остальная логика перемещения узлов...
    if (dragging) {
        if (dragging_id == g_load0idx || dragging_id == g_load1idx) {
            model_node* pnode = &nodes_hierarchy[dragging_id];
            glm::vec3 newpos = node_down_pos;

            int parent_id = pnode->get_parent_id();
            glm::vec3 parentPos(0.0f);
            float rangeMin, rangeMax;
            if (dragging_id == g_load0idx) {
                rangeMin = LOAD0_MIN_REL_Y;
                rangeMax = LOAD0_MAX_REL_Y;
            }
            else if (dragging_id == g_load1idx) {
                rangeMin = LOAD1_MIN_REL_Y;
                rangeMax = LOAD1_MAX_REL_Y;
            }
            else {
                return;
            }
            float initialRel = 0.0f;
            if (parent_id >= 0) {
                parentPos = nodes_hierarchy[parent_id].get_pos();
                initialRel = node_down_pos.y - parentPos.y;
            }

            float screenH = static_cast<float>(g_viewport.w);
            float normDelta = (mouse_down.y - g_mouse.y) / screenH;
            float relY = initialRel + normDelta * (rangeMax - rangeMin);
            relY = glm::clamp(relY, rangeMin, rangeMax);

            newpos.y = parentPos.y + relY;
            pnode->set_position(newpos);
        }
        else if (dragging_id == g_pendulum_node_idx) {
            float deltaX = g_mouse.x - mouse_down.x;
            float new_angle = pendulum_down_angle - deltaX * ANGLE_SENSITIVITY;
            new_angle = glm::clamp(new_angle, PENDULUM_MIN_ANGLE, PENDULUM_MAX_ANGLE);
            nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, new_angle });
        }
    }
}

gl_light<0> main_light;

struct text_texture
{
    text_texture() {}
    glm::vec3  background_color;
    glm::vec3  foreground_color;
    glm::ivec2 size;
};

GLuint create_text_texture(const text_texture& srcinfo, gl_font& font, const char* pformat, ...)
{
    char       buf[128];
    GLenum     error;
    GLuint     texid;
    va_list    argptr;
    glm::ivec4 viewport;
    va_start(argptr, pformat);
    SDL_vsnprintf(buf, sizeof(buf), pformat, argptr);
    va_end(argptr);

    const glm::ivec2& size = srcinfo.size;

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        glBindTexture(GL_TEXTURE_2D, 0);
        SDL_LogError(0, "create_text_texture: glTexImage2D() failed with error %d (0x%x)!", error, error);
        return 0;
    }

    bool is_enabled_light = glIsEnabled(GL_LIGHTING);
    if (is_enabled_light)
        glDisable(GL_LIGHTING);

    bool is_enabled_depth_test = glIsEnabled(GL_DEPTH_TEST);
    if (is_enabled_depth_test)
        glDisable(GL_DEPTH_TEST);

    bool is_enabled_stencil_test = glIsEnabled(GL_STENCIL_TEST);
    if (is_enabled_stencil_test)
        glDisable(GL_STENCIL_TEST);

    glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewport));
    glViewport(0, 0, size.x, size.y);

    const glm::vec3& background = srcinfo.background_color;
    const glm::vec3& foreground = srcinfo.foreground_color;
    glClearColor(background.r, background.g, background.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    gl_font::begin_text(size.x, size.y);
    glColor3fv(glm::value_ptr(foreground));
    font.move_to(0.f, 0.f);
    font.draw_text(buf);
    gl_font::end_text();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        glBindTexture(GL_TEXTURE_2D, 0);
        SDL_LogError(0, "create_text_texture: srcinfo.font.draw_text() failed with error %d (0x%x)!", error, error);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texid);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, size.x, size.y, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (is_enabled_light)
        glEnable(GL_LIGHTING);

    if (is_enabled_depth_test)
        glEnable(GL_DEPTH_TEST);

    if (is_enabled_stencil_test)
        glEnable(GL_STENCIL_TEST);

    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
    printf("create_text_texture(): OK\n");
    return texid;
}

void sort_meshes()
{
  auto& meshes = g_obj.get_meshes();
  std::sort(meshes.begin(), meshes.end(),
    [](obj_importer::mesh* a, obj_importer::mesh* b) {
      const char* nameA = a->get_name();
      const char* nameB = b->get_name();
      auto itA = std::find_if(std::begin(nodes_hierarchy), std::end(nodes_hierarchy),
          [&](const model_node& n) { return std::strcmp(n.get_name(), nameA) == 0; });
      auto itB = std::find_if(std::begin(nodes_hierarchy), std::end(nodes_hierarchy),
          [&](const model_node& n) { return std::strcmp(n.get_name(), nameB) == 0; });
      size_t idxA = (itA != std::end(nodes_hierarchy)) ? std::distance(std::begin(nodes_hierarchy), itA) : SIZE_MAX;
      size_t idxB = (itB != std::end(nodes_hierarchy)) ? std::distance(std::begin(nodes_hierarchy), itB) : SIZE_MAX;
      if (idxA != idxB)
          return idxA < idxB;
      return std::strcmp(nameA, nameB) < 0;
    }
  );
}

void init_textures()
{
    GLuint       texid;
    text_texture text_tex;
    text_tex.size = { 100, 50 };
    text_tex.background_color = glm::vec3(1.f, 0.1f, 0.1f);
    text_tex.foreground_color = glm::vec3(1.f, 1.f, 1.f);

    model_node* pLCD = find_node("node9_display");
    assert(g_LCD.get_texid() && "g_LCD texture not initialized!");
    pLCD->set_texid(g_LCD.get_texid());

    model_node* pbtn1 = find_node("button1");
    texid = create_text_texture(text_tex, g_msg_font, "Start");
    assert(texid != 0 && "failed to create texture");
    pbtn1->set_texid(texid);

    model_node* pbtn2 = find_node("button2");
    texid = create_text_texture(text_tex, g_msg_font, "Stop");
    assert(texid != 0 && "failed to create texture");
    pbtn2->set_texid(texid);

    model_node* pbtn3 = find_node("button3");
    texid = create_text_texture(text_tex, g_msg_font, "Reset");
    assert(texid != 0 && "failed to create texture");
    pbtn3->set_texid(texid);
}

int main(int argc, char* argv[])
{
    int width, height;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init() failed! Error: \"%s\"", SDL_GetError());
        return 1;
    }
#ifndef _DEBUG1
    show_banner();
#endif
    printf("loading model...\n");
    if (!g_obj.load("pendulum 0010.obj")) {
        SDL_GL_DeleteContext(g_ctx);
        SDL_DestroyWindow(g_pwindow);
        SDL_Quit();
        return 1;
    }
    sort_meshes();
    printf("model loaded!\n");

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    g_pwindow = SDL_CreateWindow("pendulum graphics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 750,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!g_pwindow) {
        printf("SDL_CreateWindow() failed! Error: \"%s\"", SDL_GetError());
        return 1;
    }

    g_ctx = SDL_GL_CreateContext(g_pwindow);
    if (!g_ctx) {
        printf("SDL_GL_CreateContext() failed! Error: \"%s\"", SDL_GetError());
        return 1;
    }
    if (SDL_GL_MakeCurrent(g_pwindow, g_ctx) < 0) {
        printf("SDL_GL_MakeCurrent() failed! Error: \"%s\"", SDL_GetError());
        return 1;
    }

    SDL_GL_SetSwapInterval(1);
    SDL_GetWindowSize(g_pwindow, &width, &height);
    update_viewport(width, height);

    int num_keys;
    const Uint8* pbuttons = SDL_GetKeyboardState(&num_keys);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    printf("enabling stencil buffer...\n");
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    if (!stencil_buffer_available()) {
        printf("stencil buffer not created correctly!\n");
        return 1;
    }

    main_light.enable();
    main_light.set_colors({ 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f, 1.f });
    main_light.set_pos({ 0.f, 100.f, 100.f, 1.f });

    prepare_nodes(nodes_hierarchy, arrsize(nodes_hierarchy));
    print_childs(nodes_hierarchy, arrsize(nodes_hierarchy));
    change_coord_system(nodes_hierarchy, arrsize(nodes_hierarchy), &g_obj);
    print_nodes_barycenter(&g_obj);
    setup_pendulum_materials();
    init_fonts();
    init_node_indices_and_cursors();
    g_LCD.init();
    init_textures();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

    g_pquadratic = gluNewQuadric();
    pendulum_phys_init_default(g_pendulum_phys, 90.f);
    glClearColor(80 / 255.f, 90 / 255.f, 100 / 255.f, 1.f);
    while (handle_events()) {
        SDL_GetWindowSize(g_pwindow, &width, &height);
        update_time();
        g_LCD.update(g_panel_font, g_msg_font, g_simulation_time);

        // Вставляем обновление курсора здесь - ДО очистки экрана и рендеринга
        if (!g_mouse_pressed) {
            update_hover_cursor();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.f, -15.f, -60.f);
        glRotatef(180.f, 0.f, 1.f, 0.f);

        constexpr float deg = 5.f;
        float sinv = sinf(g_current_time);

        if (g_sim_state == SIM_STATE_SUMULATING) {
            update_pendulum_physics(g_pendulum_phys, g_delta_time);
            nodes_hierarchy[g_pendulum_node_idx].set_rotation({ 0.f, 0.f, glm::degrees(g_pendulum_phys.angle) });
        }

        g_simtime_begin = g_current_time;
        g_simtime = g_current_time - g_simtime_begin;

        draw_pendulum();
        process_movable_nodes();
        gl_font::begin_text(width, height);
        draw_debug_overlay(width, height);
        gl_font::end_text();
        SDL_GL_SwapWindow(g_pwindow);
    }
    g_LCD.shutdown();
    g_obj.unload();

    SDL_GL_DeleteContext(g_ctx);
    SDL_DestroyWindow(g_pwindow);
    // В конце программы, перед SDL_Quit():
    SDL_FreeCursor(g_parrow_cursor);
    SDL_FreeCursor(g_phand_cursor);
    SDL_FreeCursor(g_ppointer_cursor);
    SDL_Quit();
    return 0;
}