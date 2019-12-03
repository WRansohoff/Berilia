#include "unity.h"

/**
 * Shared destructor for the 'unity' game object class.
 * Delete the physics object, the mesh instance,
 * and any attached scripts.
 */
unity::~unity() {
  if ( m ) {
    delete m;
  }
  if ( p_obj ) {
    g->p_man->phys_world->removeRigidBody( p_obj->rigid_body );
    delete p_obj;
  }
  for ( int i =0; i < scripts.size(); ++i ) {
    if ( scripts[ i ] ) {
      delete scripts[ i ];
    }
  }
}

/**
 * Internal class method to generate a game object's data
 * structures once its core attributes such as the
 * texture / mesh file paths have been set.
 */
void unity::gen_unity( v3 u_pos, quat u_rot ) {
  // Load the object's texture into the texture manager, if necessary.
  if ( texture_fn != "" && !( g->t_man->get( texture_fn ) ) ) {
    g->t_man->add_mapping_by_fn( texture_fn );
  }

  // Load an instance of the mesh data.
  m = load_mesh( mesh_fn.c_str() );
  // Set default position / scale values.
  cur_center = v3( 0, 0, 0 );
  cur_scale = v3( 1, 1, 1 );

  // Generate the physics object.
  float mass = 1.0f;
  // There are 6 types of physics objects. 'Type #N' gets you
  // the normal version, 'Type #(N+6)' gets you a static version
  // which is immobile in the physics simulation. See 'physics.h'
  if ( phys_type > 5 ) {
    mass = 0.0f;
    phys_type -= 6;
  }
  // If the game is in 'level editor' mode, the physics
  // simulation should not cause game objects to move,
  // and all objects are treated as static and immobile.
  else if ( g->editor ) {
    mass = 0.0f;
  }
  // Position and orient the object in the physics simulation.
  btVector3 b_pos =
    btVector3( u_pos.v[ 0 ], u_pos.v[ 1 ], u_pos.v[ 2 ] );
  btQuaternion b_rot = btQuaternion(
    btVector3( u_rot.r[ 1 ], u_rot.r[ 2 ], u_rot.r[ 3 ] ),
    u_rot.r[ 0 ] );

  // If the object uses a 'sphere' collision shape,
  // take the longest dimension between X / Y / Z and
  // set the collision sphere's radius to half of that.
  if ( phys_type == BRLA_PHYS_SPH ) {
    float sph_r = m->bounding_box.x_w / 2;
    if ( m->bounding_box.y_h / 2 > sph_r ) {
      sph_r = m->bounding_box.y_h / 2;
    }
    if (m->bounding_box.z_d / 2 > sph_r) {
      sph_r = m->bounding_box.z_d / 2;
    }
    p_obj = new sphere_p_obj( sph_r, mass, b_pos );
  }
  // If the object uses a 'box' collision shape,
  // simply set its size according to the object's
  // width / length / height. Divide by 2 since the
  // library's constructor expects 'half-extents'.
  else if ( phys_type == BRLA_PHYS_BOX ) {
    float box_w = m->bounding_box.x_w / 2;
    float box_h = m->bounding_box.y_h / 2;
    float box_d = m->bounding_box.z_d / 2;
    p_obj = new box_p_obj( box_w, box_h, box_d, mass, b_pos, b_rot );
  }
  // If the object uses a 'cylinder' shape, take its height along
  // the Y-axis and give it a radius equal to ( X + Z ) / 4.
  else if ( phys_type == BRLA_PHYS_CYL ) {
    float cyl_h = m->bounding_box.y_h / 2;
    float cyl_r = ( m->bounding_box.x_w + m->bounding_box.z_d ) / 4;
    p_obj = new cylinder_p_obj( cyl_r, cyl_h, mass, b_pos, b_rot );
  }
  // If the object uses a 'capsule' shape, set the height along
  // the longest axis, and pick the second-longest axis to
  // set the radius from.
  else if ( phys_type == BRLA_PHYS_CAP ) {
    aabb bb = m->bounding_box;
    float cap_r = 0.0f;
    float cap_h = 0.0f;
    int axis_type = BRLA_PHYS_Y;
    if ( bb.x_w >= bb.y_h && bb.x_w >= bb.z_d ) {
      // X is up, make Y or Z the radius.
      cap_r = m->bounding_box.z_d / 2;
      if ( m->bounding_box.y_h / 2 > cap_r ) {
        cap_r = m->bounding_box.y_h / 2;
      }
      cap_h = m->bounding_box.x_w - cap_r * 2;
      axis_type = BRLA_PHYS_X;
    }
    else if ( bb.y_h >= bb.x_w && bb.y_h >= bb.z_d ) {
      // Y is up, make X or Y the radius.
      cap_r = m->bounding_box.x_w / 2;
      if ( m->bounding_box.z_d / 2 > cap_r ) {
        cap_r = m->bounding_box.z_d / 2;
      }
      cap_h = m->bounding_box.y_h - cap_r * 2;
    }
    else {
      // Z is up, make X or Y the radius.
      cap_r = m->bounding_box.x_w / 2;
      if ( m->bounding_box.y_h / 2 > cap_r ) {
        cap_r = m->bounding_box.y_h / 2;
      }
      cap_h = m->bounding_box.z_d - cap_r * 2;
      axis_type = BRLA_PHYS_Z;
    }
    p_obj = new capsule_p_obj( cap_r, cap_h, mass,
                               axis_type, b_pos, b_rot );
  }
  // If the object has a cone shape...well, do nothing.
  // I never implemented this, I guess. Should probably delete it.
  else if ( phys_type == BRLA_PHYS_CON ) {
    // Create a cone shape. TODO.
  }
  // If the object uses a 'bounding volume hierarchy',
  // set that up from its mesh data.
  else if ( phys_type == BRLA_PHYS_BVH_TRI ) {
    // Create a bounding volume hierarchy triangle mesh shape.
    p_obj = new bvh_tri_p_obj( m, mass, b_pos, b_rot );
  }
}

/** Run each script in the game object's array of active scripts. */
void unity::run_scripts() {
  for ( int i = 0; i < scripts.size(); ++i ) {
    if ( scripts[ i ] != use_script ) {
      scripts[ i ]->call( g->elapsed_sec );
    }
  }
}

/** Perform the game loop 'update' step for this game object. */
void unity::update() {
  // Retrieve position / rotation data from the physics simulation.
  if ( p_obj ) {
    btScalar p_t[ 16 ];
    p_obj->motion_state->get_world_transform( p_t );
    m4 phys_gl_transform = m4(
      p_t[ 0 ], p_t[ 4 ], p_t[ 8 ], p_t[ 12 ],
      p_t[ 1 ], p_t[ 5 ], p_t[ 9 ], p_t[ 13 ],
      p_t[ 2 ], p_t[ 6 ], p_t[ 10 ], p_t[ 14 ],
      p_t[ 3 ], p_t[ 7 ], p_t[ 11 ], p_t[ 15 ]
    );
    m->transform = phys_gl_transform;
    cur_center = v3( m->transform.t_x(),
                     m->transform.t_y(),
                     m->transform.t_z() );
  }
}

/** Draw the game object. */
void unity::draw() {
  // Make sure that there is a valid camera object.
  camera* a_cam = g->c_man->active_camera;
  if ( !a_cam ) { return; }

  // Apply the model transformation.
  glBindBuffer( GL_UNIFORM_BUFFER, g->c_man->cam_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER,
                    g->c_man->cam_ubo,
                    g->c_man->cam_block_buffer );
  glBufferSubData( GL_UNIFORM_BUFFER,
                   0,
                   sizeof( float ) * 16,
                   transpose( m->transform ).m );

  // Apply the texture sampler.
  texture* m_tex = g->t_man->get( texture_fn );
  if ( m_tex ) {
    int tex_loc = glGetUniformLocation( g->s_man->cur_shader,
                                        "texture_sampler" );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_tex->tex );
    glUniform1i( tex_loc, m_tex->tex_sampler );
  }
  // Bind the Vertex Array Object.
  glBindVertexArray( m->vao );
  // Draw the mesh vertex data.
  glDrawArrays( GL_TRIANGLES, 0, m->num_vertices );
}

/** Set a given script as this object's 'on-use' script. */
void unity::make_use_script(script* s) { use_script = s; }

/**
 * Set this game object's velocity in the physics simulation
 * to a given X / Y / Z vector.
 */
void unity::set_phys_velocity( btVector3 vel ) {
  // Do nothing if the game object has no physics objects.
  if ( p_obj && p_obj->rigid_body ) {
    // If necessary, limit velocity to this object's 'max_velocity'.
    btVector3 new_vel = vel;
    float new_vel_mag = new_vel.length();
    if ( new_vel_mag > max_velocity ) {
      float factor = max_velocity / new_vel_mag;
      new_vel *= factor;
    }
    // Apply the new velocity.
    p_obj->rigid_body->setLinearVelocity( new_vel );
    p_obj->rigid_body->applyGravity();
    p_obj->rigid_body->applyCentralImpulse(
      btVector3( 0.0, 0.0, 0.0 )
    );
  }
}

/** Wrapper to call the 'dv' method with a 3-vector. */
void unity::dv( v3 delta ) {
  dv( delta.v[ 0 ], delta.v[ 1 ], delta.v[ 2 ] );
}

/**
 * Change the game object's velocity by a specified amount
 * along the X / Y / Z axes.
 */
void unity::dv( float dx, float dy, float dz ) {
  // Return early if the game object doesn't have physics objects.
  if ( !p_obj ) { return; }
  btRigidBody* r_b = p_obj->rigid_body;
  if ( !r_b ) { return; }

  // Activate the rigid body and get its current velocity.
  r_b->activate();
  btVector3 velocity = r_b->getLinearVelocity();

  // Adjust the current velocity by the given deltas.
  float new_x = velocity.getX() + dx;
  float new_y = velocity.getY() + dy;
  float new_z = velocity.getZ() + dz;
  // Assemble the new velocity.
  btVector3 new_velocity = btVector3( new_x, new_y, new_z );
  // Adjust to fit the object's 'max_velocity' if necessary.
  float new_vel_mag = new_velocity.length();
  if ( new_vel_mag > max_velocity ) {
    float factor = max_velocity / new_vel_mag;
    new_velocity *= factor;
  }

  // Apply the new velocity to the object.
  r_b->setLinearVelocity( new_velocity );
  r_b->applyGravity();
  r_b->applyCentralImpulse( btVector3( 0.0, 0.0, 0.0 ) );
  // Call 'update' to re-sync the game object's transformation
  // with the physics simulation, although this probably isn't
  // really necessary until after the next 'stepSimulation' call.
  update();
}

/**
 * Helper method to move a game object by a
 * specified X / Y / Z delta vector.
 */
void unity::move( v3 delta ) { translate(cur_center + delta); }

/**
 * Helper method to move a game object by a
 * specified X / Y / Z delta.
 */
void unity::move( float dx, float dy, float dz ) {
  move( v3( dx,dy,dz ) );
}

/**
 * Helper method to move a game object to a
 * specified X / Y / Z position vector.
 */
void unity::translate( v3 pos ) {
 translate( pos.v[ 0 ], pos.v[ 1 ], pos.v[ 2 ] ) ;
}

/** Move a game object to a specified X / Y / Z position. */
void unity::translate( float x, float y, float z ) {
  // Update the object's internal 'position' object.
  cur_center = v3( x, y, z );

  // Ensure that the game object has associated physics objects.
  if ( p_obj && p_obj->rigid_body && p_obj->motion_state ) {
    // Set the new position.
    btVector3 new_pos = btVector3( x, y, z );
    // Don't change the object's rotation.
    btQuaternion b_quat = btQuaternion(
      btVector3( rot.r[ 1 ], rot.r[ 2 ], rot.r[ 3 ] ),
      rot.r[ 0 ] );
    // Create the new physics object transformation matrix.
    btTransform phys_transform =
      p_obj->rigid_body->getCenterOfMassTransform();
    phys_transform.setOrigin( new_pos );
    phys_transform.setRotation( b_quat );

    // Activate the physics object, and set its position / rotation.
    p_obj->rigid_body->activate();
    p_obj->rigid_body->setCenterOfMassTransform( phys_transform );
    p_obj->motion_state->setWorldTransform( phys_transform );
    // Call 'update' to apply the new settings
    // from the physics simulation.
    update();
  }
}

/**
 * Update the game object's rotation or orientation in the
 * game world to match a given quaternion.
 */
void unity::set_rotation( quat new_rotation ) {
  // Update the game object's internal 'rotation' quaternion.
  rot = new_rotation;

  // Check that the game object has associated physics objects.
  if ( p_obj && p_obj->rigid_body && p_obj->motion_state ) {
    // Don't change the object's position.
    btVector3 b_pos = btVector3( cur_center.v[ 0 ],
                                 cur_center.v[ 1 ],
                                 cur_center.v[ 2 ] );
    // Set the new rotation.
    btQuaternion b_quat = btQuaternion(
      btVector3( rot.r[ 1 ], rot.r[ 2 ], rot.r[ 3 ] ),
      rot.r[ 0 ] );
    // Create the new physics object transformation matrix.
    btTransform phys_transform =
      p_obj->rigid_body->getCenterOfMassTransform();
    phys_transform.setOrigin( b_pos );
    phys_transform.setRotation( b_quat );

    // Activate the physics object, and set its position / rotation.
    p_obj->rigid_body->activate();
    p_obj->rigid_body->setCenterOfMassTransform( phys_transform );
    p_obj->motion_state->setWorldTransform( phys_transform );
    // Call 'update' to apply the new settings
    // from the physics simulation.
    update();
  }
}

/**
 * Rotate the game object by a quaternion delta. Quaternion rotation
 * is sort of hard to visualize, but it's good to have a method like
 * this to do things like slowly rotate something around an axis.
 */
void unity::rotate_by( quat dr ) {
  // Create the new rotation by calculating ( delta * old ).
  rot = dr * rot;

  // Ensure that the game object has associated physics objects.
  if ( p_obj && p_obj->rigid_body && p_obj->motion_state ) {
    // Create an updated physics transformation matrix.
    btVector3 b_pos = btVector3( cur_center.v[ 0 ],
                                 cur_center.v[ 1 ],
                                 cur_center.v[ 2 ] );
    btQuaternion b_quat = btQuaternion(
      btVector3( rot.r[ 1 ], rot.r[ 2 ], rot.r[ 3 ] ),
      rot.r[ 0 ] );
    btTransform phys_transform =
      p_obj->rigid_body->getCenterOfMassTransform();
    phys_transform.setOrigin( b_pos );
    phys_transform.setRotation( b_quat );

    // Activate the physics object, and set its position / rotation.
    p_obj->rigid_body->activate();
    p_obj->rigid_body->setCenterOfMassTransform( phys_transform );
    p_obj->motion_state->setWorldTransform( phys_transform );
    // Call 'update' to apply the new settings
    // from the physics simulation.
    update();
  }
}

/**
 * Scale the game object by specified scalar factors
 * along the X / Y / Z axes.
 */
void unity::scale( v3 new_scale ) {
  // Find the scaling ratio, and apply it to the mesh vertices.
  v3 ds = new_scale / cur_scale;
  m->scale_by( ds );
  // Scale the physics object too.
  cur_scale = new_scale;
  btVector3 new_phys_scale = btVector3( new_scale.v[ 0 ],
                                        new_scale.v[ 1 ],
                                        new_scale.v[ 2 ] );
  p_obj->c_shape->setLocalScaling( new_phys_scale );
}

/**
 * Wrapper method for scaling the game object
 * by specified scalars along the X / Y / Z axes.
 */
void unity::scale( float s_x, float s_y, float s_z ) {
  scale( v3( s_x, s_y, s_z ) );
}

/**
 * 'Unity manager' constructor. Keep in mind that 'unity_manager'
 * objects can nest, so there will probably be more than one of
 * these objects. But they don't really need to initialize
 * any internal state.
 */
unity_manager::unity_manager() {}

/**
 * 'Unity manager' destructor. Delete any game objects owned
 * by this manager, and then delete any child managers.
 */
unity_manager::~unity_manager() {
  // Delete game objects.
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( unities[ i ] ) {
      delete unities[ i ];
      unities[ i ] = 0;
    }
  }
  // Delete child managers and clear the array.
  for ( int i = 0; i < children.size(); ++i ) {
    if ( children [ i ] ) {
      delete children[ i ];
      children[ i ] = 0;
    }
  }
  children.clear();
}

/**
 * Add an existing game object to a 'unity_manager'.
 */
void unity_manager::add_unity( unity* u ) {
  // Add the object to the array of active objects.
  unities.push_back( u );
  // Add a mapping of its physics object to the lookup map.
  if ( u->p_obj && u->p_obj->rigid_body ) {
    phys_map[ u->p_obj->rigid_body ] = u;
  }
}

/**
 * Create a new game object and add it to this 'unity_manager'
 * object, given a string representing the new game object's type
 * and a starting position / rotation. Returns 0 if no game object
 * was created; most likely cause would be an unrecognized type.
 */
unity* unity_manager::add_unity( string type, v3 pos, quat rot ) {
  // Check the string type against various options, and
  // create a new object if there's a match.
  unity* new_unity = 0;
  if ( type == "u_flowerpot" ) {
    new_unity = new u_flowerpot( pos, rot );
  }
  else if ( type == "u_test_terrain" ) {
    new_unity = new u_test_terrain( pos, rot );
  }
  else if ( type == "u_light_ind" ) {
    new_unity = new u_light_ind( pos, rot );
  }
  else if ( type == "u_c_microscope" ) {
    new_unity = new u_c_microscope( pos, rot );
  }
  else if ( type == "u_c_circuit_1" ) {
    new_unity = new u_c_circuit_1( pos, rot );
  }
  else if ( type == "u_c_toolbox" ) {
    new_unity = new u_c_toolbox( pos, rot );
  }
  // If a new object was created, add it to the current manager.
  if ( new_unity ) {
    add_unity( new_unity );
  }
  // Return the new object, or 0 if one wasn't created.
  return new_unity;
}

/** Remove a game object from a 'unity_manager', and delete it. */
void unity_manager::evict_unity( unity* u ) {
  // If the game object is currently selected, clear the selection.
  if ( u == g->selected_unity ) { g->selected_unity = 0; }
  // Erase the physics object from the lookup map, if one exists.
  if ( u->p_obj && u->p_obj->rigid_body ) {
    phys_map.erase( u->p_obj->rigid_body );
  }
  // Find the game object in the array of active objects,
  // remove it, and delete it.
  for ( auto u_i = unities.begin(); u_i != unities.end(); ++u_i ) {
    unity* v_u = *u_i;
    if ( u == v_u ) {
      delete u;
      unities.erase( u_i );
      break;
    }
  }
}

/**
 * Retrieve a pointer to a game object from the lookup map,
 * given a pointer to its physics object. This lookup map
 * is useful because you will often want to find a game object
 * from the result of a collision or intersection in the
 * Bullet physics simulation, and 'btRigidBody' pointers are a
 * convenient way to convert between the physics and game worlds.
 */
unity* unity_manager::get( btRigidBody* rb ) {
  return phys_map[ rb ];
}

/**
 * Add a mapping between a physics object and a game object in
 * the lookup map. If the game object is not already in the
 * manager's list of active objects, this method also adds it
 * to that list.
 */
void unity_manager::set( btRigidBody* rb, unity* u ) {
  // Add the mapping.
  phys_map[ rb ] = u;

  // Add the game object to the list of active ones, if necessary.
  bool has_u = false;
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( u == unities[ i ] ) {
      has_u = true;
    }
  }
  if ( !has_u ) { unities.push_back( u ); }
}

/**
 * Empty the list of active game objects.
 * TODO: This should also empty the physics lookup map,
 * but it doesn't.
 */
void unity_manager::clear_unities() {
  // Delete all of the manager's game objects.
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( unities[ i ] ) {
      delete unities[ i ];
      unities[ i ] = 0;
    }
  }
  // Empty the game object array.
  unities.clear();
}

/**
 * Perform the game loop's 'update' step for the game objects
 * in this manager's array of active objects.
 */
void unity_manager::update() {
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( unities[ i ] ) {
      // Update each game object.
      unities[ i ]->update();
      // Also run any scripts attached to them, unless
      // the application is running in 'level editor' mode.
      if ( !g->editor ) {
        unities[ i ]->run_scripts();
      }
    }
  }
}

/**
 * Perform the game loop's 'draw' step for the game objects
 * in this manager's array of active objects.
 */
void unity_manager::draw() {
  // Just draw each object if it exists.
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( unities[ i ] ) { unities[ i ]->draw(); }
  }
}

/**
 * Run an arbitrary function on all game objects held by this
 * manager object. Optionally, also run the same function
 * on game objects held by all child manager objects.
 */
void unity_manager::for_each( function<void( unity* u )> action,
                              bool include_children ) {
  // Call the function with all game objects held by this manager.
  for ( int i = 0; i < unities.size(); ++i ) {
    if ( unities[ i ] ) { action( unities[ i ] ); }
  }
  // Recursively call the function in child managers, if requested.
  if ( include_children ) {
    for ( int i = 0; i < children.size(); ++i ) {
      if ( children[ i ] ) {
        children[ i ]->for_each( action, true );
      }
    }
  }
}

/** Constructor for a 'flowerpot' game object. */
u_flowerpot::u_flowerpot( v3 pos, quat rot ) {
  type = "u_flowerpot";
  texture_fn = "textures/png/flowerpot.png";
  mesh_fn = "meshes/flowerpot.dae";
  phys_type = BRLA_PHYS_CYL;

  gen_unity( pos, rot );
}

/** Constructor for a test terrain game object. */
u_test_terrain::u_test_terrain( v3 pos, quat rot ) {
  type = "u_test_terrain";
  texture_fn = "textures/png/test_terrain_2.png";
  mesh_fn = "meshes/test_terrain_2.dae";
  phys_type = BRLA_PHYS_STATIC_BVH_TRI;

  gen_unity( pos, rot );
}

/** Constructor for the temporary player character game object. */
u_player_mesh::u_player_mesh( v3 pos, quat rot ) {
  type = "u_player_mesh";
  texture_fn = "textures/png/player_mesh.png";
  mesh_fn = "meshes/player_mesh.dae";
  phys_type = BRLA_PHYS_CAP;

  gen_unity( pos, rot );
}

/** Constructor for a lighting indicator game object. */
u_light_ind::u_light_ind( v3 pos, quat rot ) {
  type = "u_light_ind";
  texture_fn = "textures/png/light_ind.png";
  mesh_fn = "meshes/light_ind.dae";
  phys_type = BRLA_PHYS_STATIC_SPH;

  gen_unity( pos, rot );
}

/** Constructor for a 'microscope' game object. */
u_c_microscope::u_c_microscope( v3 pos, quat rot ) {
  type = "u_c_microscope";
  texture_fn = "textures/png/c_microscope.png";
  mesh_fn = "meshes/c_microscope.dae";
  phys_type = BRLA_PHYS_BOX;

  gen_unity( pos, rot );
}

/** Constructor for a 'circuit board' game object. */
u_c_circuit_1::u_c_circuit_1( v3 pos, quat rot ) {
  type = "u_c_circuit_1";
  texture_fn = "textures/png/c_circuit_1.png";
  mesh_fn = "meshes/c_circuit_1.dae";
  phys_type = BRLA_PHYS_BOX;

  gen_unity( pos, rot );
}

/** Constructor for a 'toolbox' game object. */
u_c_toolbox::u_c_toolbox( v3 pos, quat rot ) {
	type = "u_c_toolbox";
	texture_fn = "textures/png/c_toolbox.png";
	mesh_fn = "meshes/c_toolbox.dae";
	phys_type = BRLA_PHYS_BOX;

	gen_unity( pos, rot );
}
