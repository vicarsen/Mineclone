#include "io.h"
#include "GLFW/glfw3.h"

DEFINE_LOG_CATEGORY(IO);

namespace mc
{
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    input_t *input = reinterpret_cast<input_t*>(glfwGetWindowUserPointer(window));
    
    switch(action) {
    case GLFW_PRESS:
      if (!input->key_pressed[key]) {
        input->key_pressed[key] = true;
        input->key_just_pressed[key] = true;
      }
      break;

    case GLFW_RELEASE:
      if (input->key_pressed[key]) {
        input->key_pressed[key] = false;
        input->key_just_released[key] = true;
      }
      break;
    }
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
  {
    input_t *input = reinterpret_cast<input_t*>(glfwGetWindowUserPointer(window));
    
    switch(action) {
    case GLFW_PRESS:
      if (!input->button_pressed[button]) {
        input->button_pressed[button] = true;
        input->button_just_pressed[button] = true;
      }
      break;
    
    case GLFW_RELEASE:
      if (input->button_pressed[button]) {
        input->button_pressed[button] = false;
        input->button_just_released[button] = true;
      }
      break;
    }
  }

  static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
  {
    input_t *input = reinterpret_cast<input_t*>(glfwGetWindowUserPointer(window));

    input->last_cursor_pos = input->cursor_pos;
    input->cursor_pos = { xpos, ypos };
    input->cursor_delta = input->cursor_pos - input->last_cursor_pos;
  }

  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
  {
    input_t *input = reinterpret_cast<input_t*>(glfwGetWindowUserPointer(window));

    input->scroll = { xoffset, yoffset };
  }

  void input_create(input_t *input, window_t *wnd)
  {
    LOG_ASSERT(input != nullptr, IO, "Input pointer is null!");
    LOG_ASSERT(input->window == nullptr, IO, "Input is already created!");
    LOG_ASSERT(wnd != nullptr, IO, "Window pointer is null!");
    LOG_ASSERT(wnd->window != nullptr, IO, "Window was never created!");

    GLFWwindow *window = wnd->window;
    glfwSetWindowUserPointer(window, input);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    input->window = wnd;
    input->key_pressed.reset();
    input->key_just_pressed.reset();
    input->key_just_released.reset();
    input->button_pressed.reset();
    input->button_just_pressed.reset();
    input->button_just_released.reset();

    double cursor_x, cursor_y;
    glfwGetCursorPos(window, &cursor_x, &cursor_y);

    input->last_cursor_pos = { cursor_x, cursor_y };
    input->cursor_pos = { cursor_x, cursor_y };
    input->cursor_delta = { 0.0f, 0.0f };

    input->scroll = { 0.0f, 0.0f };

    input->last_time = glfwGetTime();
    input->time = input->last_time;
    input->delta_time = 0.0f;
  }

  void input_preupdate(input_t *input)
  {
    input->key_just_pressed.reset();
    input->key_just_released.reset();
    
    input->button_just_pressed.reset();
    input->button_just_released.reset();
  }

  void input_update(input_t *input)
  {
    input->last_time = input->time;
    input->time = glfwGetTime();
    input->delta_time = input->time - input->last_time;
  }

  void input_destroy(input_t *input)
  {
    LOG_ASSERT(input != nullptr, IO, "Input pointer is null!");
    LOG_ASSERT(input->window != nullptr, IO, "Input was never created!");

    GLFWwindow *window = input->window->window;
    glfwSetKeyCallback(window, nullptr);
    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetCursorPosCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
  }
};

