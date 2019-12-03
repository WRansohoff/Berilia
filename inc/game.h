#ifndef BRLA_GAME_H
#define BRLA_GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <btBulletDynamicsCommon.h>

#include <string>
#include <unordered_map>
#include <vector>

#include <assert.h>
#include <math.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>

#include "camera.h"
#include "gui.h"
#include "lighting.h"
#include "math3d.h"
#include "physics.h"
#include "script.h"
#include "shaders.h"
#include "util.h"
#include "texture.h"
#include "tinydir.h"
#include "unity.h"

/** OpenGL major version used; currently, use v4.2 */
#define BRLA_MAJOR_VERSION 4
/** OpenGL minor version used; currently, use v4.2 */
#define BRLA_MINOR_VERSION 2
/** Anti-aliasing samples. TODO: Make this a configurable option. */
#define BRLA_AA_SAMPLES 8

/** Number of floats in the 'game' Uniform Buffer Object. */
#define BRLA_GAME_UBO_SIZE 8
/** Size of the C-string buffer which holds the window title. */
#define BRLA_TITLE_BUF_SIZE 128

using std::mt19937;
using std::string;
using std::unordered_map;
using std::vector;

// Forward declarations.
class phong_light;
class phys_debug_draw;
class unity;
// Manager classes.
class camera_manager;
class gui_manager;
class lighting_manager;
class physics_manager;
class shader_manager;
class texture_manager;
class unity_manager;

/**
 * Main 'game' class, which contains the application's global state.
 */
class game {
public:
  /** When set to true, the application should clean up and exit. */
  bool should_quit = false;
  /** Counter variable to help calculate framerate. */
  int fps_frame_count = 0;
  /** Current window width, in pixels. */
  int g_win_w;
  /** Current window height, in pixels. */
  int g_win_h;
  /** 'Near' clipping plane distance from camera. */
  float near = 0.1f;
  /** 'Far' clipping plane distance from camera. */
  float far = 1000.0f;
  /**
   * Vertical field-of-vision angle, in degress.
   * 67.0 ~= 90 @ 4:3. 50.625 = 90 @ 16:9.
   */
  float fov = 50.625f;
  /**
   * Current display aspect ratio. This should be the
   * window's width divided by its height.
   */
  float aspect_ratio;
  /** 'Previous seconds' timer count, for calculating framerate. */
  double prev_seconds;
  /** 'Elapsed seconds' timer count, for calculating framerate. */
  double elapsed_sec;
  /**
   * Buffer to hold the window's current title.
   * Must be a null-terminated C-string.
   */
  char win_title_buf[ BRLA_TITLE_BUF_SIZE ];
  /** Pointer to the application's GLFW window object. */
  GLFWwindow* window = 0;
  /**
   * String representing the name of the 'normal' camera,
   * which is generally used to display the player's perspective.
   * There needs to be at least one camera object.
   */
  string cam_normal = "Normal Camera";

  // Shader filenames.
  /** File containing the 'normal' vertex shader. */
  string normal_vert_shader_fn = "shaders/vert/normal.vert";
  /** File containing the 'normal' fragment shader. */
  string normal_frag_shader_fn = "shaders/frag/normal.frag";
  /** File containing the 'single-color' fragment shader. (TODO: ?) */
  string color_frag_shader_fn = "shaders/frag/color.frag";
  /** File containing the 'physics debug' vertex shader. */
  string phys_debug_vert_shader_fn = "shaders/vert/phys_debug.vert";
  /** File containing the 'physics debug' fragment shader. */
  string phys_debug_frag_shader_fn = "shaders/frag/phys_debug.frag";
  /** File containing the '2D GUI' vertex shader. */
  string gui_vert_shader_fn = "shaders/vert/gui.vert";
  /** File containing the '2D GUI' fragment shader. */
  string gui_frag_shader_fn = "shaders/frag/gui.frag";
  /** File containing the 'world GUI' fragment shader. (TODO: ?) */
  string world_gui_frag_shader_fn = "shaders/frag/world_gui.frag";
  /** File containing the 'depth edge detection' fragment shader. (TODO: ?) */
  string pp_edge_detect_shader_fn = "shaders/frag/pp_edge_detect.frag";
  /** File containing the 'post-processing image shim' fragment shader. (TODO: ?) */
  string pp_img_shim_frag_shader_fn = "shaders/frag/img_shim.frag";
  /** File containing the 'particle effect' vertex shader. */
  string particles_vert_shader_fn = "shaders/vert/particles.vert";
  /** File containing the 'particle effect' fragment shader. */
  string particles_frag_shader_fn = "shaders/frag/particles.frag";
  /** File containing the 'depth mask' fragment shader. */
  string depth_frag_shader_fn = "shaders/frag/depth.frag";
  /** String key for the 'normal' shader program. */
  string normal_shader_key = "normal shader";
  /** String key for the 'physics debug' shader program. */
  string phys_debug_shader_key = "phys debug shader";
  /** String key for the '2D GUI' shader program. */
  string gui_shader_key = "GUI shader";
  /** String key for the 'world GUI' shader program. (TODO: ?) */
  string world_gui_shader_key = "World-space GUI shader";
  /** String key for the 'single-color' shader program. */
  string color_shader_key = "Color shader";
  /** String key for the 'depth edge detection' shader program. (TODO: ?) */
  string pp_edge_detect_shader_key = "Edge detection shader";
  /** String key for the 'post-processing image shim' shader program. (TODO: ?) */
  string pp_img_shim_shader_key = "Image clearing shim";
  /** String key for the 'particle effect' shader program. */
  string particles_shader_key = "Particle effects";
  /** String key for the 'depth mask' shader program. */
  string depth_shader_key = "Depth buffer";

  /** Pointer to the global 'lighting manager' object. */
  lighting_manager* l_man = 0;
  /** Pointer to the global 'texture manager' object. */
  texture_manager* t_man = 0;
  /** Pointer to the global 'physics manager' object. */
  physics_manager* p_man = 0;
  /** Pointer to the global 'shader manager' object. */
  shader_manager* s_man = 0;
  /** Pointer to the global 'camera manager' object. */
  camera_manager* c_man = 0;
  /** Pointer to the global 'game object manager' object. */
  unity_manager* u_man = 0;
  /** Pointer to the global 'GUI manager' object. */
  gui_manager* g_man = 0;

  /** File containing a simple monospace font atlas. */
  string f_mono = "textures/png/fonts/monospace.png";

  /**
   * Buffer to hold values which are sent to shaders in an
   * OpenGL 'uniform buffer object' which is associated with
   * the global game world.
   */
  float world_ubo_buf[ BRLA_GAME_UBO_SIZE ];
  /** 'Game world' UBO ID. */
  int world_ubo = 0;
  /** 'Game world' UBO block buffer index. */
  GLuint world_block_buffer;
  /** 'Game world' UBO index. */
  GLuint world_ubo_index;

  /** Array of global scripts to run during gameplay. */
  vector<script*> g_scripts;

  /**
   * Hash map of keyboard presses. Each key has a boolean
   * value tracking whether it is pressed or not, since key
   * presses are detected in asynchronous callbacks.
   */
  unordered_map<char, bool> pressed_keys;

  /**
   * Global state value: when set to true, the game will render
   * a 'physics debugging' overlay, showing the shapes used
   * by the Bullet physics simulation.
   */
  bool draw_phys_debug = false;
  /**
   * Global state value: when set to true, the application
   * will act as a 'level editor' instead of a game.
   */
  bool editor = false;
  /**
   * Global state value: when set to true, this indicates
   * that the player has lost the current game. Maybe you
   * show a 'game over / reload' screen or a final score.
   */
  bool lost = false;
  /** Global state value: when set to true, the projection
   * matrix should be updated. This usually only happens when
   * the window is resized or the FOV is modified, so there's
   * no need to re-calculate it every frame.
   */
  bool update_proj_matrix = true;
  /**
   * Global state value: when set to true, the game should
   * be paused.
   */
  bool paused = false;

  /** Is the left mouse button currently pressed? */
  bool left_mouse_down = false;
  /** Is the right mouse button currently pressed? */
  bool right_mouse_down = false;
  /** Is the middle mouse button currently pressed? */
  bool middle_mouse_down = false;
  /** Is the 'use' key currently pressed? */
  bool use_key_down = false;
  /** Is the 'tab' key currently pressed? */
  bool tab_key_down = false;
  /** Is the 'left arrow' key currently pressed? */
  bool L_arrow_down = false;
  /** Is the 'right arrow' key currently pressed? */
  bool R_arrow_down = false;
  /** Is the 'up arrow' key currently pressed? */
  bool U_arrow_down = false;
  /** Is the 'down arrow' key currently pressed? */
  bool D_arrow_down = false;

  /** The current X-axis location of the mouse cursor, in pixels. */
  double mouse_x = 0.0f;
  /** The current Y-axis location of the mouse cursor, in pixels. */
  double mouse_y = 0.0f;
  /** Change in X-axis mouse cursor location from the last frame. */
  double mouse_dx = 0.0f;
  /** Change in Y-axis mouse cursor location from the last frame. */
  double mouse_dy = 0.0f;

  /**
   * When the game is not in 'zero-gravity' mode,
   * this value tracks the camera's X-axis rotation in degrees.
   */
  float cam_x_theta = 0.0;
  /**
   * When the game is not in 'zero-gravity' mode,
   * this value tracks the camera's Y-axis rotation in degrees.
   */
  float cam_y_theta = 0.0;
  /**
   * When the game is not in 'zero-gravity' mode, this
   * quaternion represents the camera's rotation about
   * the X-axis. The camera's total rotation is calculated
   * as ( cam_x_quat * cam_y_quat ).
   */
  quat cam_x_quat;
  /**
   * When the game is not in 'zero-gravity' mode, this
   * quaternion represents the camera's rotation about
   * the Y-axis. The camera's total rotation is calculated
   * as ( cam_x_quat * cam_y_quat ).
   */
  quat cam_y_quat;
  /**
   * Vector-3 representing a ray which is cast from the current
   * camera perspective when the left mouse button is pressed.
   */
  v3 mouse_ray;
  /**
   * Vector-3 representing a ray which is cast from the current
   * camera perspective when the right mouse button is pressed.
   */
  v3 r_mouse_ray;
  /**
   * Vector-3 representing a ray which is cast from the current
   * camera perspective when the middle mouse button is pressed.
   */
  v3 m_mouse_ray;
  /**
   * Vector-3 representing the current 'up' (Y-axis) direction
   * from the current camera's perspective.
   */
  v3 cam_y_axis;
  /**
   * Pointer to a the game object which was hit by a 'selection'
   * raycast, used when the application wants the user to pick
   * out a single object in the game world.
   */
  unity* picked_body = 0;
  /**
   * Physics Vector-3 representing the exact location where the
   * raycast which selected the 'picked_body' game object hit.
   */
  btVector3 last_picked_pos;
  /**
   * Pointer to the game object which the current camera
   * is looking at, if any.
   */
  unity* looking_at_unity = 0;
  /**
   * Pointer to the game object which the player has currently
   * selected, if any.
   */
  unity* selected_unity = 0;
  /**
   * Pointer to the Phong light source which the player has
   * currently selected, if any.
   */
  phong_light* selected_light = 0;

  game( int w, int h );
  ~game();

  void init();
  int process_game_loop();
  void reload_file( string fn );

  bool check_key_press( int glfw_key, const char c );
  bool check_key_press( int glfw_key, const char c, const char c2 );
  void check_text_input( int glfw_key, const char c );
  void check_text_input( int glfw_key, const char c, const char c2 );

  void log_shader_errors( GLuint shader );
  void write_world_ubo();
  v3 get_mouse_ray( int pix_x, int pix_y );
  void add_script_to_selected( string type );
  void pick_looking_at( v3 dir );
  void pick_looking_at( v3 pos, v3 dir );
  void pick_selection( v3 dir );
};

extern game* g;
extern mt19937 re;

void glfw_log_err( int err, const char* desc );
void glfw_win_resize( GLFWwindow* window, int w, int h );
void glfw_mouse_pos( GLFWwindow* window, double m_x, double m_y );
void glfw_mouse_button( GLFWwindow* window,
                        int button,
                        int action,
                        int mods );

#endif
