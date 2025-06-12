#pragma once
#include <glm/glm.hpp>
#include <SDL_stdinc.h>
#include "font.h"

struct pendulum_physp {
  struct {
    float start_angle;
  } d;
  float length;       // длина стержня
  float rod_mass;     // масса стержня
  float load_mass;    // масса груза
  float angle;        // текущий угол отклонения (в радианах)
  float angular_vel;  // угловая скорость
  float damping;      // коэффициент затухания
  int pivot_node;     // attach point (node7 or node8)
};

void  pendulum_phys_init_default(pendulum_physp &dst, float start_angle_deg);
float calculate_period(const pendulum_physp& params);
void  update_pendulum_physics(pendulum_physp& params, float delta_time);
void  pendulum_physics_print(const pendulum_physp& params, gl_font &font);

//glm::degrees(pendulum_params.angle)
