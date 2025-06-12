#include "simulation.h"

// Добавь эти константы в начало кода (после include)
constexpr float GRAVITY = 9.81f; // ускорение свободного падения
constexpr float AIR_RESISTANCE = 0.9f; // коэффициент сопротивления воздуха
constexpr float PIVOT_FRICTION = 0.05f; // трение в точке крепления

void pendulum_phys_init_default(pendulum_physp& dst, float start_angle_deg)
{
  dst.d.start_angle = start_angle_deg;
  dst.length = 20.0f; // примерная длина стержня
  dst.rod_mass = 5.0f;
  dst.load_mass = 10.0f;
  dst.angle = glm::radians(start_angle_deg); // начальный угол 15 градусов
  dst.angular_vel = 0.0f;
  dst.damping = 0.1f;
  dst.pivot_node = 7; // по умолчанию крепим за node7
}

// Функция для расчета периода колебаний
float calculate_period(const pendulum_physp& params)
{
  // Для математического маятника: T = 2pi*sqrt(L/g)
  // Для физического маятника учитываем момент инерции
  float effective_length = params.length;

  // Если крепим за верхний узел (node7)
  if (params.pivot_node == 7) {
    // Момент инерции стержня относительно конца: I = (1/3)*m*L^2
    float rod_inertia = (1.0f / 3.0f) * params.rod_mass * params.length * params.length;
    // Момент инерции груза: I = m*L^2
    float load_inertia = params.load_mass * params.length * params.length;
    float total_inertia = rod_inertia + load_inertia;
    float total_mass = params.rod_mass + params.load_mass;
    effective_length = total_inertia / (total_mass * params.length);
  }
  // Если крепим за середину (node8)
  else if (params.pivot_node == 8) {
    // Момент инерции стержня относительно центра: I = (1/12)*m*L^2
    float rod_inertia = (1.0f / 12.0f) * params.rod_mass * params.length * params.length;
    // Момент инерции груза: I = m*(L/2)^2
    float load_inertia = params.load_mass * (params.length / 2) * (params.length / 2);
    float total_inertia = rod_inertia + load_inertia;
    float total_mass = params.rod_mass + params.load_mass;
    effective_length = total_inertia / (total_mass * (params.length / 2));
  }
  return 2.0f * M_PI * glm::sqrt(effective_length / GRAVITY);
}

// Функция обновления физики маятника (добавь в main перед рисованием)
void update_pendulum_physics(pendulum_physp& params, float delta_time)
{
  delta_time = glm::clamp(delta_time, 0.f, 0.1f);

  // Рассчитываем момент инерции в зависимости от точки крепления
  float inertia, torque_arm;

  if (params.pivot_node == 7) { // Крепление за верхний узел
    // Момент инерции стержня относительно конца: I = (1/3)*m*L^2
    float rod_inertia = (1.0f / 3.0f) * params.rod_mass * params.length * params.length;
    // Момент инерции груза: I = m*L^2
    float load_inertia = params.load_mass * params.length * params.length;
    inertia = rod_inertia + load_inertia;
    torque_arm = params.length;
  }
  else { // Крепление за середину (node8)
    // Момент инерции стержня относительно центра: I = (1/12)*m*L^2
    float rod_inertia = (1.0f / 12.0f) * params.rod_mass * params.length * params.length;
    // Момент инерции груза: I = m*(L/2)^2
    float load_inertia = params.load_mass * (params.length / 2) * (params.length / 2);
    inertia = rod_inertia + load_inertia;
    torque_arm = params.length / 2.0f;
  }

  // Суммарная масса (для силы тяжести)
  float total_mass = params.rod_mass + params.load_mass;

  // Крутящий момент от силы тяжести: tau = -m*g*L*sin(omega)
  float torque = -total_mass * GRAVITY * torque_arm * glm::sin(params.angle);

  // Добавляем демпфирование (трение и сопротивление воздуха)
  torque -= params.angular_vel * (params.damping + AIR_RESISTANCE * torque_arm);

  // Угловое ускорение: alpha = tau/I
  float angular_accel = torque / inertia;

  // Интегрируем уравнения движения
  params.angular_vel += angular_accel * delta_time;
  params.angle += params.angular_vel * delta_time;

  // Ограничиваем угловую скорость (чтобы не было бесконечных колебаний)
  const float max_angular_vel = 5.0f;
  if (glm::abs(params.angular_vel) > max_angular_vel) {
    params.angular_vel = glm::sign(params.angular_vel) * max_angular_vel;
  }
}

void pendulum_physics_print(const pendulum_physp& params, gl_font& font)
{
  font.draw_textf("Angle: %.2f deg (start: %.2f)", 
    glm::degrees(params.angle), params.d.start_angle);
  font.draw_textf("Angular vel: %.2f rad/s", params.angular_vel);
  font.draw_textf("Period: %.2f s", calculate_period(params));
}