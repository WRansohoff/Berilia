#ifndef BRLA_PHYSICS_H
#define BRLA_PHYSICS_H

#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <limits>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "game.h"
#include "math3d.h"
#include "mesh.h"
#include "stb_image.h"

using std::set;
using std::pair;
using std::unordered_map;
using std::vector;

class mesh;
class game;

typedef pair<const btRigidBody*, const btRigidBody*> collision_pair;

/**
 * Constants representing different types of
 * primitive physics shapes.
 */
enum physics_primitives {
  BRLA_PHYS_SPH = 0,
  BRLA_PHYS_BOX = 1,
  BRLA_PHYS_CYL = 2,
  BRLA_PHYS_CAP = 3,
  BRLA_PHYS_CON = 4,
  BRLA_PHYS_BVH_TRI = 5,
  BRLA_PHYS_STATIC_SPH = 6,
  BRLA_PHYS_STATIC_BOX = 7,
  BRLA_PHYS_STATIC_CYL = 8,
  BRLA_PHYS_STATIC_CAP = 9,
  BRLA_PHYS_STATIC_CON = 10,
  BRLA_PHYS_STATIC_BVH_TRI = 11
};

/**
 * Collision masks. 'NOCOL' means 'no collision'.
 * The 'NONE' values will not be detected by raycasts.
 */
enum collision_masks {
  B_CT_NORMAL = 1,
  B_CT_NOCOL = 2,
  B_CT_NONE = 0,
  B_CM_NONE = 0,
  B_CM_ALL = ( B_CT_NORMAL )
};

/**
 * Constants for the primary X/Y/Z physics axes.
 */
enum PHYSICS_AXES {
  BRLA_PHYS_X = 0,
  BRLA_PHYS_Y = 1,
  BRLA_PHYS_Z = 2
};

/**
 * Maximum number of substeps in each step of the Bullet
 * physics simulation. TODO: Apparently 0 or 1 might be
 * a better default value for this setting. I should experiment.
 */
const int BRLA_PHYS_MAX_STEPS = 6;
/**
 * Fixed time step resolution for the Bullet physics simulation.
 * I think that this should come out to 120fps, but I could be wrong.
 */
const float BRLA_PHYS_TIME_STEP = ( 1.0f / 120.0f );

/**
 * Hashing function for the 3-Vector data type used by the
 * Bullet physics simulation.
 * TODO: This is just a stopgap hashing function,
 * there's probably a better way to do this.
 */
namespace std {
  template<>
  struct hash<btVector3> {
    typedef btVector3 argument_type;
    typedef size_t result_type;
    result_type operator()( const argument_type& x ) const {
      return ( hash<float>()( x.getX() ) +
           4 * hash<float>()( x.getY() ) +
           8 * hash<float>()( x.getZ() ) );
    }
  };
}

/**
 * Wrapper class for a super-simple implementation of a
 * 'motion state' in the Bullet physics simulation.
 * Mostly, this is a quick way to retrieve the
 * position / rotation of a physics object.
 */
class basic_motion_state : public btDefaultMotionState {
public:
  basic_motion_state( const btTransform &transform ) :
    btDefaultMotionState( transform ) {}
  void get_world_transform( btScalar* transform );
};

// Physics simulation 'step' callback.
void world_step_callback( btDynamicsWorld* p_world, btScalar dt );

/**
 * Physics debug drawer. This class implements Bullet's
 * "btIDebugDraw" interface, so its basic organization is
 * pre-ordained. This implementation draws a simple wireframe
 * outline of the data provided by the physics simulation.
 */
class phys_debug_draw : public btIDebugDraw {
protected:
  /**
   * The 'mode' for physics debug drawing.
   * This value is a collection of flags for various options.
   */
  int debug_mode;
  /** The number of points to draw in total. */
  int num_points;
  /** Vertex Buffer Object for the debug drawing point locations. */
  GLuint points_vbo = 0;
  /** Vertex Buffer Object for the debug drawing point colors. */
  GLuint colors_vbo = 0;
  /** Vertex Attribute Object for the debug drawing. */
  GLuint phys_vao = 0;

public:
  /** Array of point positions to draw. */
  vector<GLfloat> points;
  /** Array of point colors to draw. */
  vector<GLfloat> colors;

  phys_debug_draw();
  ~phys_debug_draw();

  virtual void setDebugMode( int mode ) override;
  virtual int getDebugMode() const override;

  virtual void drawContactPoint( const btVector3 &point,
                                 const btVector3 &normal,
                                 btScalar dist,
                                 int time,
                                 const btVector3 &color ) override;
  virtual void drawLine( const btVector3 &from,
                         const btVector3 &to,
                         const btVector3 &color ) override;
  virtual void flushLines() override;

  virtual void reportErrorWarning( const char* warning ) override {}
  virtual void draw3dText( const btVector3 &location,
                           const char* text ) override {}

  void toggleDebugFlag( int flag );
  void enableDebugFlag( int flag );
  void disableDebugFlag( int flag );
};

/**
 * 'Physics object' class. This provides an interface between
 * the game engine and the representation of an object in the
 * Bullet physics simulation. Usually, a game object ('unity')
 * will have a pointer to one of these objects along with
 * its mesh, texture, scripts, etc.
 * This is also a semi-abstract class; usually the actual
 * 'phys_obj' object will be one of the subclasses listed
 * below, such as 'box_p_obj' or 'bvh_tri_p_obj', depending
 * on the shape of the object that it represents.
 */
class phys_obj {
protected:
  void gen_phys_obj( float mass, btVector3 pos, btQuaternion rot );

public:
  /** The collision shape associated with this object. */
  btCollisionShape* c_shape = 0;
  /** The rigid body associated with this object. */
  btRigidBody* rigid_body = 0;
  /**
   * The 'motion state' associated with this object.
   * This class is an easy way to get the physics object's
   * current transformation matrix from the simulation.
   */
  basic_motion_state* motion_state = 0;
  /**
   * Array containing constraints attached to this physics object.
   */
  vector<btTypedConstraint*> constraints;
  /**
   * 'Global angle limit' constraint, mostly used to keep
   * the 'player camera' object upright in the camera's
   * 'X/Y' rotation mode.
   * TODO: Generalize this into the 'constraints' array?
   */
  btGeneric6DofConstraint* global_angle_limit = 0;
  
  /**
   * Collision type to associate with this physics object.
   * This is a way for the engine to figure out if this
   * object is able to collide with other game objects.
   */
  int collision_type = B_CT_NORMAL;
  /**
   * Collision mask for this physics object. This is a way
   * for the engine to decide which types of collision types
   * should be able to collide with this object.
   */
  int collision_mask = B_CM_ALL;

  virtual ~phys_obj();

  btGeneric6DofConstraint* add_phys_constraint(
    v3 local_pos,
    btRigidBody* body,
    float p_strength = 1.0f,
    float a_limit = 0.0f );
  btGeneric6DofConstraint* add_phys_constraint(
    v3 first_c_pos,
    btRigidBody* first,
    v3 second_c_pos,
    btRigidBody* second,
    float p_strength = 1.0f,
    float a_limit = 0.0f );
  void remove_phys_constraint( btGeneric6DofConstraint* c );
  void set_global_angle_limit( v3 angles_lower, v3 angles_upper );
  void remove_global_angle_limit();
  void change_collision_mask( int type, int mask );
};

/**
 * Implementation of the 'phys_obj' class for a sphere shape.
 */
class sphere_p_obj : public phys_obj {
public:
  sphere_p_obj( float radius,
                float mass,
                btVector3 pos = btVector3( 0, 0, 0 ),
                btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Implementation of the 'phys_obj' class for a box shape,
 * or a rectangular prism.
 */
class box_p_obj : public phys_obj {
public:
  box_p_obj( float width,
             float height,
             float depth,
             float mass,
             btVector3 pos = btVector3( 0, 0, 0 ),
             btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Implementation of the 'phys_obj' class for a cylinder shape.
 */
class cylinder_p_obj : public phys_obj {
public:
  cylinder_p_obj( float radius,
                  float height,
                  float mass,
                  btVector3 pos = btVector3( 0, 0, 0 ),
                  btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Implementation of the 'phys_obj' class for a capsule shape,
 * or a cylinder capped by half-spheres. This is a pretty good
 * shape to use for characters moving over terrain, if you add
 * a constraint to keep them upright.
 */
class capsule_p_obj : public phys_obj {
public:
  capsule_p_obj( float radius,
                 float height,
                 float mass,
                 int axis,
                 btVector3 pos = btVector3( 0, 0, 0 ),
                 btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Implementation of the 'phys_obj' class for a cone shape.
 */
class cone_p_obj : public phys_obj {
public:
  cone_p_obj( float radius,
              float height,
              float mass,
              btVector3 pos = btVector3( 0, 0, 0 ),
              btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Implementation of the 'phys_obj' class for a
 * 'Triangle Bounding Volume Hierarchy' shape. This
 * is a good shape for complex or concave geometries,
 * because it uses a custom triangle mesh instead of a
 * primitive shape. It is more expensive than the basic
 * implementations, but it's not as bad as you might think
 * if you use a low-polygon mesh for the physics object.
 */
class bvh_tri_p_obj : public phys_obj {
public:
  bvh_tri_p_obj( mesh* m,
                 float mass,
                 btVector3 pos = btVector3( 0, 0, 0 ),
                 btQuaternion rot = btQuaternion( 0, 0, 0, 1 ) );
};

/**
 * Physics manager class, which manages the Bullet physics
 * simulation. It keeps track of the core simulation pointers,
 * steps the simulation during the game's 'update' step, and
 * optionally draws debugging information during the 'draw' step.
 */
class physics_manager {
public:
  /**
   * Pointer to the Bullet 'broadphase' interface. The broadphase
   * is a first rough pass which determines where collisions
   * might be able to occur, which lets the simulation avoid
   * a lot of calculations.
   */
  btBroadphaseInterface* broadphase;
  /**
   * Pointer to the Bullet collision configuration.
   * Currently, only the default configuration constructor is used.
   */
  btCollisionConfiguration* collision_config;
  /**
   * Pointer to the Bullet collision dispatcher.
   * Currently, only the default configuration constructor is used
   * with the 'collision_config' object.
   */
  btCollisionDispatcher* collision_dispatch;
  /**
   * Pointer to the bullet constraints solver.
   * Currently, the default 'btSequentialImpulseConstraintSolver'
   * implementation is used.
   */
  btConstraintSolver* solver;
  /**
   * Pointer to the Bullet physics simulation's view of the
   * game world. Currently uses the 'btDiscreteDynamicsWorld'
   * implementation with the above 'collision_dispatch',
   * 'broadphase', 'solver', and 'collision_config' values.
   */
  btDiscreteDynamicsWorld* phys_world;
  /**
   * Pointer to the physics debug drawing implementaion.
   */
  phys_debug_draw* phys_debug;
  /**
   * Pointer to the physics clock object, used to figure out
   * how much time the next simulation step should account for.
   */
  btClock phys_clock;

  /**
   * Set to track ongoing collisions in the physics simulation.
   */
  set<collision_pair> col_manifolds;

  physics_manager();
  ~physics_manager();

  void update();
  void draw();

  // TODO: Should these 'physics event' methods be in the unity class?
  void collision_event( collision_pair p );
  void separation_event( collision_pair p );
};

#endif
