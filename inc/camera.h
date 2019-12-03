#ifndef BRLA_CAMERA_H
#define BRLA_CAMERA_H

#include <GL/glew.h>

#include <string>
#include <unordered_map>

#include "game.h"
#include "math3d.h"
#include "physics.h"
#include "unity.h"
#include "util.h"

/**
 * Camera rotation type enumeration.
 * Quaternion rotation lets the camera align itself freely in 3D
 * space, for 'zero-g' type interactions. X/Y rotation places the
 * camera on a single plane, for typical first-person views.
 */
enum cam_rotation_types {
  B_CAM_ROT_QUAT = 0,
  B_CAM_ROT_XY = 1
};

/** Maximum camera speed along any one axis (X/Y/Z). */
const float CAM_SPEED = 5.0f;
/**
  * Camera speed adjustment value, used only if the camera is
  * not attached to a game object with a valid physics object.
  */
const float CAM_REF_SPEED = 0.5f;
/** Rate to rotate the camera at in 'quaternion' mode. */
const float CAM_ROT_SPEED = 100.0f;
/** Y-velocity above which the camera cannot 'jump' in 'X/Y' mode. */
const float CAM_MAX_JUMP = 10.0f;
/** Camera stabilization factor;
    how quickly to 'slow down' in zero-g mode. */
const float CAM_STABILIZE_FACTOR = 0.5f;
/** Velocity below which the camera is considered 'stable'
    in zero-g mode. */
const float CAM_STABILIZE_MIN = 0.02f;
/** Camera UBO size, in number of floats. */
const int CAM_UBO_SIZE = 48;

using std::string;
using std::unordered_map;

class game;
class phys_obj;
class unity;

class camera {
public:
  /** 4x4 matrix representing the camera's translation (position).
      Derived from 'cam_pos'. */
  m4 cam_trans;
  /** 4x4 matrix representing the camera's rotation.
      Derived from 'cam_quat'. */
  m4 cam_rot;
  /** 4x4 matrix representing the camera's "view" matrix. */
  m4 c_view_matrix;
  /** 4x4 matrix representing the camera's "perspective" matrix. */
  m4 persp_matrix;
  /** Vector-3 representing the camera's current position. */
  v3 cam_pos =  v3(0.0f, 0.0f, 0.0f);
  /** Vector-3 representing how far to move the camera in a frame,
      if it does not have an associated physics object. */
  v3 cam_move = v3(0.0f, 0.0f, 0.0f);
  /** Quaternion representing the camera's current rotation. */
  quat cam_quat = quat(0.0f, 0.0f, 1.0f, 0.0f);
  /** Boolean tracking if the camera moved in the last frame. */
  bool cam_moved = false;
  /** Type of camera rotation. */
  int cam_rot_type = B_CAM_ROT_QUAT;
  /** Current camera yaw, in degrees. */
  float cam_yaw =   0.0f;
  /** Current camera pitch, in degrees. */
  float cam_pitch = 0.0f;
  /** Current camera roll, in degrees. */
  float cam_roll =  0.0f;
  /** Optional game object to bind the camera to. */
  unity* cam_obj = 0;

  camera();
  camera( float near, float far, float fov, float aspect_ratio );
  ~camera();

  void update_cam_pos();
  void stabilize( float step );
  void yaw( float yaw );
  void pitch( float pitch );
  void roll( float roll );
  void rotate( quat new_rot );
  void look_at( v3 focus );
  void right( float right );
  void fwd( float fwd );
  void up( float up );
  void delta_v( v3 dir, float dv );
  void jump( float j );
};

class camera_manager {
public:
  /** Hash map holding available camera objects. Keys are strings. */
  unordered_map<string, camera*> cameras;
  /** The active camera, whose perspective is drawn to the screen. */
  camera* active_camera = 0;
  // The camera is very important to the shaders, so
  // it has a uniform buffer object all to itself.
  /** Dedicated 'camera' Uniform Buffer Object buffer index. */
  int cam_ubo = 1;
  /** Dedicated 'camera' Uniform Buffer Object block buffer ID. */
  GLuint cam_block_buffer = 0;
  /** Dedicated 'camera' Uniform Buffer Object index. */
  GLuint cam_ubo_index = 0;
  /** Float buffer backing the 'camera' Uniform Buffer Object */
  float cam_ubo_buf[ CAM_UBO_SIZE ];

  camera_manager();
  ~camera_manager();

  void update_cam_ubo();
  void init_cam_ubo();

  camera* add_camera(string name, camera* c);
  camera* add_camera(string name);
  void evict_camera(string name);
  camera* get_camera(string name);
  camera* switch_camera(string name);
  int get_ubo_index();

  void update();
};

#endif
