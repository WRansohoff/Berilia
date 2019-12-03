#include "physics.h"

/**
 * Get the current transformation (position / rotation) of
 * an object from the physics simulation. Bullet includes
 * a special 'getOpenGLMatrix' method on its 'btTransform'
 * class, so it's easy to get a pointer to 16 scalar values
 * configured as a 4x4 matrix.
 */
void basic_motion_state::get_world_transform(btScalar* transform) {
  btTransform trans;
  getWorldTransform( trans );
  trans.getOpenGLMatrix( transform );
}

/**
 * Callback which the Bullet physics simulation calls when
 * an internal simulation tick / step occurs. This method
 * checks for new collisions, and existing collisions which
 * are no longer colliding. When it finds them, it triggers
 * a collision or separation event in the associated game objects.
 */
void world_step_callback( btDynamicsWorld* p_world, btScalar dt ) {
  // Keep track of colliding object pairs.
  set<collision_pair> manifolds_in_update;
  // Process each manifold, and send collision callbacks
  // for new collisions which did not exist in the last update.
  for ( int i = 0;
        i < g->p_man->collision_dispatch->getNumManifolds();
        ++i ) {
    // Get the 'collision manifold' object pointer from the
    // Bullet physics simulation.
    btPersistentManifold* c_manifold =
      g->p_man->collision_dispatch->getManifoldByIndexInternal( i );

    // 0 contact points means that the AABBs collide,
    // but the objects don't. So only register collisions
    // between game objects when there are > 0 contact points.
    if ( c_manifold->getNumContacts() > 0 ) {
      // Get the two rigid bodies involved in this collision manifold.
      const btRigidBody* p_rb0 =
        static_cast<const btRigidBody*>( c_manifold->getBody0() );
      const btRigidBody* p_rb1 =
        static_cast<const btRigidBody*>( c_manifold->getBody1() );
      // Avoid making two entries for A:B and B:A by always
      // placing the pointer with the higher memory address
      // as the first entry in the pair.
      bool swap_bodies = p_rb0 > p_rb1;
      const btRigidBody* p_srb0 = swap_bodies ? p_rb0 : p_rb1;
      const btRigidBody* p_srb1 = swap_bodies ? p_rb1 : p_rb0;

      // Make a 'collision_pair' object and add it to the set.
      collision_pair manifold_pair = std::make_pair(p_srb0, p_srb1);
      manifolds_in_update.insert(manifold_pair);

      // If the pair wasn't already colliding, send a collision event.
      if ( g->p_man->col_manifolds.find( manifold_pair ) ==
           g->p_man->col_manifolds.end() ) {
        g->p_man->collision_event( manifold_pair );
      }
    }
  }

  // Find the set of manifolds whose bodies are no longer colliding.
  set<collision_pair> removed_manifolds;
  std::set_difference( g->p_man->col_manifolds.begin(),
                       g->p_man->col_manifolds.end(),
                       manifolds_in_update.begin(),
                       manifolds_in_update.end(),
                       std::inserter( removed_manifolds,
                                      removed_manifolds.end() ) );
  // Send a 'separation event' to each pair of game objects
  // which are no longer colliding.
  for ( auto iter = removed_manifolds.begin();
        iter != removed_manifolds.end();
        ++iter ) {
    collision_pair sep_p = *iter;
    g->p_man->separation_event( sep_p );
  }

  g->p_man->col_manifolds = manifolds_in_update;
}

/**
 * Physics debug drawing interface constructor.
 * Initializes the OpenGL vertex array and buffer objects.
 */
phys_debug_draw::phys_debug_draw() {
  glGenVertexArrays( 1, &phys_vao );
  glGenBuffers( 1, &points_vbo );
  glGenBuffers( 1, &colors_vbo );
}

/**
 * Physics debug drawing interface destructor.
 * Deletes the OpenGL vertex array and buffer objects.
 */
phys_debug_draw::~phys_debug_draw() {
  if (phys_vao) { glDeleteVertexArrays( 1, &phys_vao ); }
  if (points_vbo) { glDeleteBuffers( 1, &points_vbo ); }
  if (colors_vbo) { glDeleteBuffers( 1, &colors_vbo ); }
}

/**
 * Helper method to set the physics debug drawing mode
 * to a specific value.
 */
void phys_debug_draw::setDebugMode( int mode ) { debug_mode = mode; }

/** Helper method to get the current debug drawing mode / flags. */
int phys_debug_draw::getDebugMode() const { return debug_mode; }

/**
 * Draw a 'contact point', represented by a line showing the
 * location and surface normal.
 */
void phys_debug_draw::drawContactPoint( const btVector3 &point,
                                        const btVector3 &normal,
                                        btScalar dist,
                                        int time,
                                        const btVector3 &color ) {
  const btVector3 p1 = point;
  const btVector3 p2 = point + normal * dist;
  drawLine( p1, p2, color );
}

/**
 * Schedule a line to be drawn between two X/Y/Z points.
 */
void phys_debug_draw::drawLine( const btVector3 &from,
                                const btVector3 &to,
                                const btVector3 &color ) {
  points.push_back( from.getX() );
  points.push_back( from.getY() );
  points.push_back( from.getZ() );
  points.push_back( to.getX() );
  points.push_back( to.getY() );
  points.push_back( to.getZ() );
  colors.push_back( color.getX() );
  colors.push_back( color.getY() );
  colors.push_back( color.getZ() );
  colors.push_back( color.getX() );
  colors.push_back( color.getY() );
  colors.push_back( color.getZ() );
}

/**
 * Update the OpenGL VAO / VBOs if necessary, and draw the
 * lines representing the physics debug wireframes.
 */
void phys_debug_draw::flushLines() {
  // First, find the number of points; if it's changed since the
  // last frame, we need to reallocate the VBOs and VAO.
  bool size_dif = false;
  if ( num_points != points.size() ) { size_dif = true; }
  num_points = points.size();

  // Bind the physics debug drawing VAO.
  glBindVertexArray( phys_vao );

  // Buffer or re-buffer the point locations.
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  if ( size_dif ) {
    glBufferData( GL_ARRAY_BUFFER,
                  points.size() * sizeof( GLfloat ),
                  &points[ 0 ],
                  GL_STATIC_DRAW );
  }
  else {
    glBufferSubData( GL_ARRAY_BUFFER,
                     0,
                     points.size() * sizeof( GLfloat ),
                     &points[ 0 ] );
  }
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( 0 );

  // Buffer or re-buffer the point colors.
  glBindBuffer( GL_ARRAY_BUFFER, colors_vbo );
  if ( size_dif ) {
    glBufferData( GL_ARRAY_BUFFER,
                  colors.size() * sizeof( GLfloat ),
                  &colors[0],
                  GL_STATIC_DRAW );
  }
  else {
    glBufferSubData( GL_ARRAY_BUFFER,
                     0,
                     colors.size() * sizeof( GLfloat ),
                     &colors[ 0 ] );
  }
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( 1 );

  // Draw the lines as a wireframe, then switch back to 'fill' mode.
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDrawArrays( GL_LINES, 0, points.size() );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  // Clear the 'points' and 'colors' arrays in
  // preparation for the next frame.
  points.clear();
  colors.clear();
}

/** Toggle an individual physics debug drawing mode flag. */
void phys_debug_draw::toggleDebugFlag( int flag ) {
  debug_mode ^= flag;
}

/** Enable an individual physics debug drawing mode flag. */
void phys_debug_draw::enableDebugFlag( int flag ) {
  debug_mode |= flag;
}

/** Disable an individual physics debug drawing mode flag. */
void phys_debug_draw::disableDebugFlag( int flag ) {
  debug_mode &= ~( flag );
}

/** Shared physics object destructor. */
phys_obj::~phys_obj() {
  // Remove and delete any constraints on the physics object.
  for ( int i = 0; i < constraints.size(); ++i ) {
    if ( constraints[i] ) {
      g->p_man->phys_world->removeConstraint( constraints[ i ] );
      delete constraints[ i ];
    }
  }

  // Remove and delete any pointers to physics objects / attributes.
  if ( global_angle_limit ) {
    g->p_man->phys_world->removeConstraint( global_angle_limit );
    delete global_angle_limit;
    global_angle_limit = 0;
  }
  if ( c_shape ) {
    delete c_shape;
    c_shape = 0;
  }
  if ( rigid_body ) {
    g->p_man->phys_world->removeRigidBody( rigid_body );
    delete rigid_body;
    rigid_body = 0;
  }
  if ( motion_state ) {
    delete motion_state;
    motion_state = 0;
  }
}

/**
 * Generate a representation of a physics object in the
 * Bullet physics simulation, as long as its collision shape exists.
 */
void phys_obj::gen_phys_obj( float mass,
                             btVector3 pos,
                             btQuaternion rot ) {
  // Return early if the collision shape does not exist.
  if ( !c_shape ) { return; }

  // Create the initial physics transforms.
  btTransform p_transform;
  p_transform.setIdentity();
  p_transform.setOrigin( pos );
  p_transform.setRotation( rot );

  // Create the motion state to associate with the shape.
  motion_state = new basic_motion_state( p_transform );

  // Calculate local inertia for the shape and its desired mass.
  btVector3 local_inertia( 0, 0, 0 );
  if ( mass != 0.0 ) {
    c_shape->calculateLocalInertia( mass, local_inertia );
  }

  // Create the rigid body.
  btRigidBody::btRigidBodyConstructionInfo
    rigid_body_info( mass, motion_state, c_shape, local_inertia );
  rigid_body = new btRigidBody( rigid_body_info );

  // Add the rigid body to the physics simulation.
  g->p_man->phys_world->addRigidBody( rigid_body,
                                      collision_type,
                                      collision_mask );
}

/**
 * Add a physics constraint to a single mesh.
 */
btGeneric6DofConstraint* phys_obj::add_phys_constraint(
    v3 local_pos,
    btRigidBody* body,
    float p_strength,
    float a_limit) {
  // Create the pivot in local mesh space.
  btTransform pivot;
  pivot.setIdentity();
  pivot.setOrigin(
    btVector3( local_pos.v[ 0 ], local_pos.v[ 1 ], local_pos.v[ 2 ] )
  );

  // Create the constraint.
  btGeneric6DofConstraint* constraint =
    new btGeneric6DofConstraint( *body, pivot, true );

  // Limit angular motion
  float a_r = a_limit / 2;
  constraint->setAngularLowerLimit( btVector3( -a_r, -a_r, -a_r ) );
  constraint->setAngularUpperLimit( btVector3( a_r, a_r, a_r ) );

  // Define the constraint strength on each axis.
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 0 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 1 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 2 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 3 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 4 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 5 );

  // Define the constraint's error reduction on each axis.
  float c_err = 0.9f;
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 0 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 1 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 2 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 3 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 4 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 5 );

  // Add the constraint to the physics simulation and the
  // array of active constraints held by the physics object.
  g->p_man->phys_world->addConstraint( constraint, true );
  constraints.push_back( constraint );
  // Return a pointer to the new constraint.
  return constraint;
}

/**
 * Add a physics constraint between two rigid bodies.
 */
btGeneric6DofConstraint* phys_obj::add_phys_constraint(
    v3 first_c_pos,
    btRigidBody* first,
    v3 second_c_pos,
    btRigidBody* second,
    float p_strength,
    float a_limit ) {
  // Create the pivot points in local mesh space.
  btTransform first_pivot, second_pivot;
  first_pivot.setIdentity();
  first_pivot.setOrigin( btVector3( first_c_pos.v[ 0 ],
                                    first_c_pos.v[ 1 ],
                                    first_c_pos.v[ 2 ] ) );
  second_pivot.setIdentity();
  second_pivot.setOrigin( btVector3( second_c_pos.v[ 0 ],
                                     second_c_pos.v[ 1 ],
                                     second_c_pos.v[ 2 ] ) );
  // Create the constraint.
  btGeneric6DofConstraint* constraint =
    new btGeneric6DofConstraint( *first, *second,
                                 first_pivot, second_pivot,
                                 true );

  // Define the constraint strength on each axis.
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 0 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 1 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 2 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 3 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 4 );
  constraint->setParam( BT_CONSTRAINT_STOP_CFM, p_strength, 5 );

  // Define the constraint's error reduction on each axis.
  float c_err = 0.9f;
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 0 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 1 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 2 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 3 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 4 );
  constraint->setParam( BT_CONSTRAINT_STOP_ERP, c_err, 5 );

  // Add the constraint to the physics simulation and the
  // array of active constraints held by the physics object.
  g->p_man->phys_world->addConstraint( constraint, true );
  constraints.push_back( constraint );
  // Return a pointer to the new constraint.
  return constraint;
}

/**
 * Remove a physics constraint from the simulation and the
 * array of active constraints held by the physics object.
 */
void phys_obj::remove_phys_constraint( btGeneric6DofConstraint* c ) {
  // Return early if the constraint does not exist.
  if ( !c ) { return; }
  // Run through the array of active constraints and remove this one.
  for ( auto con_iter = constraints.begin();
        con_iter != constraints.end();
        ++con_iter ) {
    if ( *con_iter == c ) { constraints.erase( con_iter ); }
  }
  // Remove the constraint from the Bullet physics simulation.
  g->p_man->phys_world->removeConstraint( c );
  // Delete the constraint.
  delete c;
}

/**
 * Add a 'global angle limit' constraint to a physics object.
 * Mostly, this is useful for keeping objects upright or within
 * a certain angular rotation range. I've only tested it with a
 * range of (-0.0, 0.0) to prevent an object from rotating, though.
 */
void phys_obj::set_global_angle_limit( v3 angles_lower,
                                       v3 angles_upper ) {
  // If a 'global angle limit' constraint already exists,
  // remove and delete it before setting up the new one.
  remove_global_angle_limit();

  // Setup the pivot at the center of the local mesh space.
  btTransform pivot;
  pivot.setIdentity();
  pivot.setOrigin( btVector3( 0.0, 0.0, 0.0 ) );
  global_angle_limit = new btGeneric6DofConstraint( *rigid_body,
                                                    pivot,
                                                    true );

  // Set the lower and upper angle limits for each axis.
  global_angle_limit->setAngularLowerLimit( btVector3(
    angles_lower.v[ 0 ],
    angles_lower.v[ 1 ],
    angles_lower.v[ 2 ] )
  );
  global_angle_limit->setAngularUpperLimit( btVector3(
    angles_upper.v[0],
    angles_upper.v[1],
    angles_upper.v[2] )
  );

  // Set linear motion limits to [-max, max], essentially
  // preventing this constraint from affecting linear motion (I hope)
  float l = std::numeric_limits<float>::max();
  global_angle_limit->setLinearLowerLimit(btVector3(-l,-l,-l));
  global_angle_limit->setLinearUpperLimit(btVector3(l,l,l));

  // Set the constraint strength along each axis.
  float p_strength = 0.1f;
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 0 );
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 1 );
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 2 );
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 3 );
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 4 );
  global_angle_limit->setParam( BT_CONSTRAINT_STOP_CFM,
                                p_strength, 5 );

  // Add the constraint to the physics world.
  g->p_man->phys_world->addConstraint( global_angle_limit, true );
}

/**
 * Remove a physics object's 'global angle limit'
 * constraint, if one exists.
 */
void phys_obj::remove_global_angle_limit() {
  if ( global_angle_limit ) {
    g->p_man->phys_world->removeConstraint( global_angle_limit );
    delete global_angle_limit;
    global_angle_limit = 0;
  }
}

/**
 * Helper method to update an object's collision type and mask.
 * 'type' = "what should I collide with?"
 * 'mask' = "what can collide with me?"
 */
void phys_obj::change_collision_mask( int type, int mask ) {
  // Update the physics object's collision type / mask records.
  collision_type = type;
  collision_mask = mask;
  // It's easiest to remove the rigid body from the simulation,
  // then re-add it with the changed 'group' / 'mask' values.
  g->p_man->phys_world->removeRigidBody( rigid_body );
  g->p_man->phys_world->addRigidBody( rigid_body,
                                      collision_type,
                                      collision_mask );
}

/** Physics object constructor: 'sphere' shape. */
sphere_p_obj::sphere_p_obj( float radius,
                            float mass,
                            btVector3 pos,
                            btQuaternion rot ) {
  // Create the 'btSphereShape' collision shape.
  c_shape = new btSphereShape( radius );
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/** Physics object constructor: 'box' / 'rectangular prism' shape. */
box_p_obj::box_p_obj( float width,
                      float height,
                      float depth,
                      float mass,
                      btVector3 pos,
                      btQuaternion rot ) {
  // Create the 'btBoxShape' collision shape.
  c_shape = new btBoxShape( btVector3( width, height, depth ) );
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/** Physics object constructor: 'cylinder' shape. */
cylinder_p_obj::cylinder_p_obj( float radius,
                                float height,
                                float mass,
                                btVector3 pos,
                                btQuaternion rot ) {
  // Create the 'btCylinderShape' collision shape with a
  // circular cross-section aligned along the Y-axis.
  // The constructor takes 'half-extents', so I think you can
  // make cylinders with elliptical cross-sections too.
  c_shape = new btCylinderShape(
    btVector3( radius, height, radius )
  );
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/** Physics object constructor: 'capsule' shape. */
capsule_p_obj::capsule_p_obj( float radius,
                              float height,
                              float mass,
                              int axis,
                              btVector3 pos,
                              btQuaternion rot ) {
  // Create the 'btCapsuleShape' object oriented
  // along the specified axis.
  if ( axis == BRLA_PHYS_X ) {
    c_shape = new btCapsuleShapeX( radius, height );
  }
  else if ( axis == BRLA_PHYS_Y ) {
    // This isn't a typo; I guess Y-axis alignment is the default.
    c_shape = new btCapsuleShape( radius, height );
  }
  else {
    c_shape = new btCapsuleShapeZ( radius, height );
  }
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/** Physics object constructor: 'cone' shape. */
cone_p_obj::cone_p_obj( float radius,
                        float height,
                        float mass,
                        btVector3 pos,
                        btQuaternion rot ) {
  // Create the 'btConeShape' collision shape.
  c_shape = new btConeShape( radius, height );
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/** Physics object constructor: 'bounding volume hierarchy' shape. */
bvh_tri_p_obj::bvh_tri_p_obj( mesh* m,
                              float mass,
                              btVector3 pos,
                              btQuaternion rot) {
  // Create the core 'btTriangleMesh' collision shape, and
  // populate it with the mesh vertex positions.
  // TODO: Where are these memory allocations deleted?
  btTriangleMesh* tri_mesh = new btTriangleMesh();
  for ( int i = 0; i < m->num_vertices * 3; i += 9 ) {
    tri_mesh->addTriangle(
      btVector3( m->points[ i ],
                 m->points[ i + 1 ],
                 m->points[ i + 2 ] ),
      btVector3( m->points[ i + 3 ],
                 m->points[ i + 4 ],
                 m->points[ i + 5 ] ),
      btVector3( m->points[ i + 6 ],
                 m->points[ i + 7 ],
                 m->points[ i + 8 ] ),
      true );
  }
  // Create the 'bounding volume hierarchy' triangle mesh shape
  // from the triangle mesh data.
  btBvhTriangleMeshShape* tri_shape =
    new btBvhTriangleMeshShape( tri_mesh, true, true );
  // Generate data structures associated with the triangle mesh.
  btTriangleInfoMap* tri_norms = new btTriangleInfoMap();
  btGenerateInternalEdgeInfo( tri_shape, tri_norms );
  tri_shape->setUserPointer( tri_norms );
  c_shape = tri_shape;
  // Call the shared 'generate new physics object' method.
  gen_phys_obj( mass, pos, rot );
}

/**
 * Physics manager constructor: initialize the Bullet physics
 * simulation and associated data structures / configurations.
 */
physics_manager::physics_manager() {
  // Initialize physics simulation.
  collision_config = new btDefaultCollisionConfiguration();
  collision_dispatch = new btCollisionDispatcher( collision_config );
  broadphase = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();
  phys_world = new btDiscreteDynamicsWorld(
    collision_dispatch,
    broadphase,
    solver,
    collision_config );
  phys_world->setGravity( btVector3( 0.0, 0.0, 0.0 ) );

  // Setup physics debu drawing.
  phys_debug = new phys_debug_draw();
  phys_debug->setDebugMode( 0 );
  phys_world->setDebugDrawer( phys_debug );
  // Enable wireframe debug drawing, and axis-aligned bounding boxes.
  phys_debug->enableDebugFlag( btIDebugDraw::DBG_DrawWireframe );
  phys_debug->enableDebugFlag( btIDebugDraw::DBG_DrawAabb );
  // Setup the physics callback for processing collisions
  // after each step of the simulation.
  phys_world->setInternalTickCallback( world_step_callback );
}

/**
 * Physics manager destructor: delete the Bullet physics simulation
 * data structures. TODO: I should also probably shut down the
 * simulation or something here - is there a process for that?
 */
physics_manager::~physics_manager() {
  if ( phys_world )         { delete phys_world; }
  if ( solver )             { delete solver; }
  if ( broadphase )         { delete broadphase; }
  if ( collision_dispatch ) { delete collision_dispatch; }
  if ( collision_config )   { delete collision_config; }
  if ( phys_debug )         { delete phys_debug; }
}

/**
 * Physics update step: simply figure out how much time has
 * passed since the last step, then step the simulation by
 * that amount of time.
 */
void physics_manager::update() {
  // Get the elapsed time since the last step.
  float dt = phys_clock.getTimeMilliseconds() / 1000.0;
  // Reset the 'physics clock' timer.
  phys_clock.reset();
  // Step the simulation according to the elapsed time.
  phys_world->stepSimulation( dt,
                              BRLA_PHYS_MAX_STEPS,
                              BRLA_PHYS_TIME_STEP );
}

/**
 * Physics simulation draw step: draw the debugging
 * wireframe information.
 */
void physics_manager::draw() {
  // Swap to the 'physics debug drawing' shader.
  g->s_man->swap_shader( g->phys_debug_shader_key );
  // Call the Bullet 'debugDrawWorld' method. This will
  // call the various 'btIDebugDraw' methods implemented
  // by the 'phys_debug_draw' class.
  phys_world->debugDrawWorld();
}

/**
 * Process a 'collision event' which marks a new collision
 * between two physics objects.
 */
void physics_manager::collision_event( collision_pair p ) {
  // Get the rigid bodies which collided.
  btRigidBody* rb1 = ( btRigidBody* )std::get<0>( p );
  btRigidBody* rb2 = ( btRigidBody* )std::get<1>( p );
  // Get the game objects associated with the physics objects.
  unity* ru1 = g->u_man->get( rb1 );
  unity* ru2 = g->u_man->get( rb2 );
  // If either game object does not exist, return early.
  if ( !ru1 || !ru2 ) { return; }
  // TODO: make collision callbacks on the 'unity' game objects.
}

/**
 * Process a 'separation event' which marks the point at
 * which two physics objects stop colliding.
 */
void physics_manager::separation_event( collision_pair p ) {
  // Get the rigid bodies which collided.
  btRigidBody* rb1 = ( btRigidBody* )std::get<0>( p );
  btRigidBody* rb2 = ( btRigidBody* )std::get<1>( p );
  // Get the game objects associated with the physics objects.
  unity* ru1 = g->u_man->get( rb1 );
  unity* ru2 = g->u_man->get( rb2 );
  // If either game object does not exist, return early.
  if ( !ru1 || !ru2 ) { return; }
  // TODO: make collision callbacks on the 'unity' game objects.
}
