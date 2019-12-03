#ifndef BRLA_UNITY_H
#define BRLA_UNITY_H

#include <GL/glew.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.h"
#include "math3d.h"
#include "mesh.h"
#include "physics.h"
#include "script.h"

using std::function;
using std::string;
using std::unordered_map;
using std::vector;

class game;
class mesh;
class phys_obj;
class script;

/**
 * A 'unity' is a game object.
 *
 * Unity. Noun. The state of being united or joined as a whole.
 * It's a good name for something that ties together
 * physics objects, meshes, scripts, textures, etc.
 *
 * To simplify things, I am going to collapse imported meshes into
 * a single set of vertices, normals, and texture coordinates.
 * They are saved and imported in a more complex tree structure by
 * the 'Ass[et]Imp[ort]' library, so I lose a few features with
 * this simplification. For example, I can only use one texture
 * per object for now.
 * But it should make things easier to manage, since there aren't
 * too many moving parts to worry about.
 *
 * TODO: Currently, most of these methods only work if the game
 * object has an associated physics object. I should either
 * make that a hard requirement, or adjust things to work
 * without physics objects.
 */
class unity {
protected:
  void gen_unity( v3 u_pos, quat u_rot );

public:
  /** File path to import the game object's mesh data from. */
  string mesh_fn = "";
  /** File path to import the game object's texture data from. */
  string texture_fn = "";
  /** Pointer to this object's 3D mesh in the game world. */
  mesh* m = 0;
  /**
   * This variable determines which sort of collision shape
   * should be used for the game object's physics object.
   * See 'physics_primitives' in 'physics.h' for options.
   */
  int phys_type = -1;
  /**
   * Pointer to this object's representation in
   * the Bullet physics simulation.
   */
  phys_obj* p_obj = 0;
  /** Script to execute if / when this object is 'used'. */
  script* use_script = 0;
  /** Array of scripts for this object to run continually. */
  vector<script*> scripts;
  /** String describing the type of game object. */
  string type = "";
  /** String for a name to identify this particular game object. */
  string name = "";
  /** Current X / Y / Z scale of this game object. */
  v3 cur_scale;
  /** Quaternion representing the object's current rotation. */
  quat rot;
  /** Current X / Y / Z position of the object's center. */
  v3 cur_center;
  /**
   * The maximum velocity that this object should be
   * allowed to travel at.
   */
  float max_velocity = 20.0f;

  virtual ~unity();

  void run_scripts();
  void update();
  void draw();

  void make_use_script( script* s );
  void set_phys_velocity( btVector3 vel );
  void dv( v3 delta );
  void dv( float dx, float dy, float dz );
  void move( v3 delta );
  void move( float dx, float dy, float dz );
  void translate( v3 pos );
  void translate( float x, float y, float z );
  void set_rotation( quat new_rotation );
  void rotate_by( quat dr );
  void scale( v3 new_scale );
  void scale( float s_x, float s_y, float s_z );
};

/**
 * 'Unity manager' class, which keeps track of game objects
 * and takes care of their 'update' and 'draw' steps.
 */
class unity_manager {
public:
  /** Array tracking active game objects managed locally. */
  vector<unity*> unities;
  /** Hash map associating physics objects with game objects. */
  unordered_map<btRigidBody*, unity*> phys_map;
  /**
   * Array of active child 'unity_manager' objects.
   * This sort of organization should make it easier to design
   * large and performant game worlds by omitting areas that
   * are not relevant at a given time. Like, think about how
   * Bethesda's NetImmerse games divide their worlds into a
   * grid of individual outdoor 'cells', and further separate
   * detailed indoor areas into their own spaces.
   */
  vector<unity_manager*> children;
  /**
   * Boolean tracking whether this object has finished
   * loading all of its resources.
   */
  bool done_loading = false;

  unity_manager();
  ~unity_manager();

  void add_unity( unity* u );
  unity* add_unity( string type, v3 pos = v3(), quat rot = quat() );
  void evict_unity( unity* u );
  unity* get( btRigidBody* rb );
  void set( btRigidBody* rb, unity* u );
  void clear_unities();

  void update();
  void draw();
  void for_each( function<void( unity* u )> action,
                 bool include_children );
};

/** 'Flowerpot' game object. */
class u_flowerpot : public unity {
public:
  u_flowerpot( v3 pos = v3(), quat rot = quat() );
};

/**
 * A test terrain game object, to figure out how to
 * handle that special case.
 */
class u_test_terrain : public unity {
public:
  u_test_terrain( v3 pos = v3(), quat rot = quat() );
};

/** A test mesh to use for the player character. */
class u_player_mesh : public unity {
public:
  u_player_mesh( v3 pos = v3(), quat rot = quat() );
};

/**
 * A simple sphere mesh to inciate where lighting objects
 * are in the game world, and make them easier to select.
 */
class u_light_ind : public unity {
public:
  u_light_ind( v3 pos = v3(), quat rot = quat() );
};

/** 'Microscope' game object. */
class u_c_microscope : public unity {
public:
  u_c_microscope( v3 pos = v3(), quat rot = quat() );
};

/** 'Circuit board' game object. */
class u_c_circuit_1 : public unity {
public:
  u_c_circuit_1( v3 pos = v3(), quat rot = quat() );
};

/** 'Toolbox' game object. */
class u_c_toolbox : public unity {
public:
  u_c_toolbox( v3 pos = v3(), quat rot = quat() );
};

#endif
