#ifndef BRLA_LIGHTING_H
#define BRLA_LIGHTING_H

#include <GL/glew.h>

#include <stdio.h>
#include <functional>

#include "camera.h"
#include "game.h"
#include "unity.h"
#include "util.h"

/**
 * Number of GLFloat values used to store data for one phong light.
 */
#define BRLA_PHONG_LIGHT_SIZE 24
/**
 * Maximum number of phong lights which can be sent to the shaders.
 * The size of the shader buffer is ( # of lights ) * ( light size ).
 */
#define BRLA_MAX_PHONG_LIGHTS 100

/**
 * Value indicating that a light is a point light which
 * emits illumination in every direction.
 */
#define BRLA_LIGHT_PHONG_POINT 0
/**
 * Value indicating that a light is a spot light which
 * emits illumination in one direction, with a given radius.
 */
#define BRLA_LIGHT_PHONG_SPOT  1

using std::function;

// Forward declarations.
class camera;
class game;
class unity;

/**
 * 'Depth pass' framebuffer object. Holds various OpenGL
 * IDs and a camera for drawing to a framebuffer instead
 * of the main display. Used for effects like casting
 * shadows from light sources.
 */
struct fb_depth_pass {
  /** OpenGL framebuffer ID for the object. */
  GLuint fbuf = 0;
  /** OpenGL depth buffer texture ID for the object. */
  GLuint fbuf_depth_tex = 0;
  // We usually will not want to draw the GUI(s) for retrieving a depth
  // or texture buffer. We may also want to avoid drawing certain unities,
  // like a light's indicator.
  /**
   * Boolean controlling whether GUI elements should
   * be drawn on this depth-pass framebuffer.
   */
  bool draw_gui = false;
  /** OpenGL texture index to use for the depth-pass framebuffers. */
  int depth_tex_ind = 3;
  /** X-axis resolution of the depth map texture. */
  int res_x;
  /** Y-axis resolution of the depth map texture. */
  int res_y;
  /**
   * Array of game objects not to draw on the framebuffer.
   * For example, lights have small 'indicator' spheres to
   * show their locations and make them selectable, but if
   * those spheres are drawn then from the light's perspective,
   * everything will be shadowed because it is inside of a sphere.
   */
  vector<unity*> do_not_draw;
  /** Vertical field of vision for the framebuffer camera. */
  float cam_fov = 90.0f;
  /**
   * Pointer to the shadow-casting camera, or the camera to
   * draw the framebuffer from.
   */
  camera* shadow_cam = 0;
  /** OpenGL shadow UBO ID for this framebuffer. */
  GLuint shadow_ubo = 0;
  /** OpenGL shadow block buffer ID for this framebuffer. */
  GLuint shadow_block_buffer = 0;
  /** OpenGL shadow UBO index for this framebuffer. */
  GLuint shadow_ubo_index = 0;
  /**
   * Buffer for the shadow UBO. TODO: Use a constant for the array
   * length, but g++ seemed to dislike that last time I tried it.
   */
  float shadow_ubo_buf[ 48 ];

  fb_depth_pass( int map_res_x, int map_res_y, float falloff );
  fb_depth_pass( int map_res_x, int map_res_y,
                 float falloff, unity* first_ignore );
  ~fb_depth_pass();

  void gen_self( int map_res_x, int map_res_y,
                 float falloff, unity* first_ignore );
  void draw();
};

/**
 * Class representing a phong light source.
 */
class phong_light {
public:
  // argument order is 'PADS' - Position, Ambient, Diffuse, Specular.
  /** Vector-4 representing a light's position in the game world. */
  v4 pos;
  /** Vector-4 representing the phong light's 'ambient' color. */
  v4 a;
  /** Vector-4 representing the phong light's 'diffuse' color. */
  v4 d;
  /** Vector-4 representing the phong light's 'specular' color. */
  v4 s;
  /** Value tracking the type of light: 'point' or 'spotlight'. */
  int type = BRLA_LIGHT_PHONG_POINT;
  /**
   * Vector-3 representing the direction that a
   * 'spotlight' is pointing in.
   */
  v3 dir;
  /**
   * Float representing the angular extent
   * of a 'spotlight', in radians.
   */
  float spot_rads = PI * 2;
  /** The 'specular exponent' for a phong light. */
  float specular_exp = 100.0f;
  /** The falloff distance for a phong light. */
  float falloff = 50.0f;
  /**
   * Pointer to a game object which helps represent and
   * anchor the light in the game world. This is a small
   * sphere which makes it easier to select and interact
   * with the light in 'level editor' and debugging mode.
   */
  unity* indicator = 0;
  /** Boolean value tracking if this light should cast shadows. */
  bool cast_shadows = false;
  /** Pointer to the shadow depth map framebuffer. */
  fb_depth_pass* shadow_depth_fb = 0;

  phong_light( v4 position, v4 amb, v4 dif, v4 spec );
  phong_light( v4 position, v4 amb, v4 dif, v4 spec,
               v3 spot_dir, float spotlight_rads );
  ~phong_light();

  void make_shadow_caster();
};

/**
 * Lighting manager class, which keeps track of lights
 * in the scene, processes effects like shadows, and
 * coordinates sending teh relevant data to the shaders.
 */
class lighting_manager {
public:
  /** Size of the phong light Uniform Buffer Object. */
  int phong_ubo_size;
  /** GLfloat array backing the phong light Uniform Buffer Object. */
  GLfloat* phong_ubo_buf = 0;
  /** Array of active phong lights. */
  vector<phong_light*> phong_lights;
  /** Phong light UBO ID number. */
  GLuint phong_ubo = 2;
  /** Phong light block buffer index. */
  GLuint phong_block_buffer = 0;
  /** Phong light UBO index. */
  GLuint phong_ubo_index = 0;

  lighting_manager();
  ~lighting_manager();

  void add_phong_light( phong_light* light );
  phong_light* add_phong_light( v4 pos, v4 amb, v4 dif, v4 spec );
  void evict_phong_light( phong_light* light );
  phong_light* get_by_indicator( unity* u );
  void clear_phong_lights();

  void init_lighting_ubo();
  void update();
  void draw();
  void draw_shadow_casters();
  void write_lighting_ubo();
};

#endif
