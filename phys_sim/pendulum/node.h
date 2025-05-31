#pragma once
#include <glm/glm.hpp> //glm::vec3, glm::vec2
#include <cassert>

static constexpr int MAX_CHILDS = 16;

struct bbox_t {
  glm::vec3 vec_min;
  glm::vec3 vec_max;

  void reset() { vec_max = vec_min = glm::vec3(0.f, 0.f, 0.f); }

  bbox_t() {
    reset();
  }

  inline void init() {
    vec_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    vec_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  }
  inline void update(glm::vec3 coord) {
    vec_min = glm::min(vec_min, coord);
    vec_max = glm::max(vec_max, coord);
  }

  inline glm::vec3 get_size() const {
    return glm::abs(vec_max - vec_min);
  }
};

class model_node
{
  int         m_parent;
  const char* m_pname;
  glm::vec3   m_pos;
  glm::vec3   m_rotation;
  int         m_num_childs;
  int         m_childs[MAX_CHILDS]{};
  bbox_t      m_bbox;
public:
  model_node(int parent, const char* pname,
    glm::vec3 pos, glm::vec3 rotation) :
    m_parent(parent), m_pname(pname),
    m_pos(pos), m_rotation(rotation), m_num_childs(0) {}

  inline bool is_root() const { return m_parent == -1; }
  inline int  get_parent_id() const { return m_parent; }
  inline const char* get_name() const { return m_pname; }
  inline glm::vec3& get_pos() { return m_pos; }
  inline glm::vec3& get_rotation() { return m_rotation; }

  inline int get_num_childs() const { return m_num_childs; }
  inline int get_child(int idx) const {
    assert(idx < m_num_childs && "idx out of bounds");
    return m_childs[idx];
  }

  inline void add_child(int idx) {
    assert(m_num_childs < MAX_CHILDS && "MAX_CHILDS limit exceeded");
    m_childs[m_num_childs++] = idx;
  }

  inline void set_position(glm::vec3 pos) { m_pos = pos; }
  inline void set_rotation(glm::vec3 angles) { m_rotation = angles; }
  inline bbox_t& get_bbox() { return m_bbox; }
};