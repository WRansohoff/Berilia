#include "camera.h"

/** Default (empty) camera constructor. */
camera::camera() {
  // Initialize camera values.
  cam_trans = translation_matrix( cam_pos );
  cam_rot = quaternion_to_rotation( cam_quat );
  c_view_matrix = view_matrix( cam_trans, cam_rot );
  persp_matrix = perspective( g->near, g->far,
                              g->fov, g->aspect_ratio );
}

/** Camera constructor. */
camera::camera( float near, float far,
                float fov, float aspect_ratio ) {
  cam_trans = translation_matrix( cam_pos );
  cam_rot = quaternion_to_rotation( cam_quat );
  c_view_matrix = view_matrix( cam_trans, cam_rot );
  persp_matrix = perspective( near, far, fov, aspect_ratio );
}

/** Camera destructor. */
camera::~camera() {
  // Currently, the camera does not 'own' the game object it
  // is associated with, so don't delete that object.
  //if ( cam_obj ) { delete cam_obj; }
}

/**
 * Update the camera's position for a new frame.
 * If the camera is associated with an object in the Bullet
 * physics simulation, then set its position to that object's
 * location. Otherwise, update it according to the current
 * 'cam_move' vector.
 */
void camera::update_cam_pos() {
  if ( cam_obj && cam_obj->p_obj && cam_obj->p_obj->motion_state ) {
    // Get the current physics object transformation.
    btScalar phys_transform[ 16 ];
    cam_obj->p_obj->motion_state->
      get_world_transform( phys_transform );
    // Set the camera position to the object's X/Y/Z coordinates.
    cam_pos.v[ 0 ] = -phys_transform[ 12 ];
    cam_pos.v[ 1 ] = -phys_transform[ 13 ];
    cam_pos.v[ 2 ] = -phys_transform[ 14 ];
  }
  else {
    // Update the camera's position according to its
    // 'cam_move' vector.
    cam_pos.v[ 0 ] += cam_move.v[ 0 ];
    cam_pos.v[ 1 ] += cam_move.v[ 1 ];
    cam_pos.v[ 2 ] += cam_move.v[ 2 ];
  }
  // Reset the 'cam_move' vector for the next frame.
  cam_move = v3( 0.0f, 0.0f, 0.0f );

  // Update the current camera translation and view matrices.
  cam_trans = translation_matrix( cam_pos.v[0],
                                  cam_pos.v[1],
                                  cam_pos.v[2] );
  c_view_matrix = view_matrix( cam_trans, cam_rot );

  // Mark that the camera movement is complete, until something
  // else marks 'cam_moved' as true.
  cam_moved = false;
}

/**
 * Helper method to 'stabilize' the camera's physics object in
 * 'zero-g' mode. This basically slows the player down without
 * changing direction. TODO: move or remove this mode-specific 
 * method, and also make it able to target any game object
 * with an attached physics object.
 */
void camera::stabilize( float step ) {
  if ( !cam_obj || !cam_obj->p_obj || !cam_obj->p_obj->rigid_body ) {
    return;
  }
  btRigidBody* r_body = cam_obj->p_obj->rigid_body;
  btVector3 velocity = r_body->getLinearVelocity();
  float x_sign = 1.0f;
  float y_sign = 1.0f;
  float z_sign = 1.0f;
  float new_x = velocity.getX();
  float new_y = velocity.getY();
  float new_z = velocity.getZ();
  if ( new_x < 0.0f ) { x_sign = -1.0f; }
  if ( new_y < 0.0f ) { y_sign = -1.0f; }
  if ( new_z < 0.0f ) { z_sign = -1.0f; }

  new_x -= new_x * CAM_STABILIZE_FACTOR * step;
  if ( new_x * x_sign < CAM_STABILIZE_MIN ) { new_x = 0.0f; }
  new_y -= new_y * CAM_STABILIZE_FACTOR * step;
  if ( new_y * y_sign < CAM_STABILIZE_MIN ) { new_y = 0.0f; }
  new_z -= new_z * CAM_STABILIZE_FACTOR * step;
  if ( new_z * z_sign < CAM_STABILIZE_MIN ) { new_z = 0.0f; }

  btVector3 new_velocity = btVector3( new_x, new_y, new_z );
  r_body->setLinearVelocity( new_velocity );
  r_body->applyGravity();
  r_body->applyCentralImpulse( btVector3( 0.0, 0.0, 0.0 ) );
}

/**
 * Adjust the camera's yaw, for use in 'quaternion rotation' mode.
 */
void camera::yaw( float yaw ) {
  // Create the new 'yaw' rotation quaternion.
  quat yaw_quat = normalize( set_quat( yaw, cam_rot.row(1) ) );
  // Adjust the current rotation by multiplying by the 'yaw' quat.
  cam_quat = cam_quat * yaw_quat;
  // Update the camera rotation matrix.
  cam_rot = quaternion_to_rotation( cam_quat );
  // Mark that the camera has moved.
  cam_moved = true;
}

/**
 * Adjust the camera's pitch, for use in 'quaternion rotation' mode.
 */
void camera::pitch(float pitch) {
  // Create the new 'pitch' rotation quaternion.
  quat pitch_quat = normalize( set_quat( pitch, cam_rot.row( 0 ) ) );
  // Adjust the current rotation by multiplying by the 'pitch' quat.
  cam_quat = cam_quat * pitch_quat;
  // Update the camera rotation matrix.
  cam_rot = quaternion_to_rotation( cam_quat );
  // Mark that the camera has moved.
  cam_moved = true;
}

/**
 * Adjust the camera's roll, for use in 'quaternion rotation' mode.
 */
void camera::roll(float roll) {
  // Create the new 'rol' rotation quaternion.
  quat roll_quat =
    normalize( set_quat( roll, ( cam_rot.row( 2 ) * -1.0f ) ) );
  // Adjust the current rotation by multiplying by the 'roll' quat.
  cam_quat = cam_quat * roll_quat;
  // Update the camera rotation matrix.
  cam_rot = quaternion_to_rotation( cam_quat );
  // Mark that the camera has moved.
  cam_moved = true;
}

/**
 * Rotate the camera to 'look along' the axis-angle
 * representation of a given quaternion.
 */
void camera::rotate(quat new_rot) {
  // Update the camera's stored rotation quaternion...
  cam_quat = new_rot;
  // ..and adjust the camera's rotation matrix to match it.
  cam_rot = quaternion_to_rotation( cam_quat );
  // Mark that the camera has moved.
  cam_moved = true;
}

/**
 * Rotate the camera such that it is looking at a specific
 * X/Y/Z location in the game world.
 */
void camera::look_at( v3 focus ) {
  // Setup a quaternion along the axis of ( point2 - point1 ).
  quat c_quat = set_quat( 0.0f, normalize( focus - cam_pos ) );
  // Rotate the camera to look along the new axis.
  rotate( c_quat );
}

/**
 * Move the camera 'right', according to its current viewpoint.
 * This adjusts the camera's velocity, rather than moving
 * by a specified number of units.
 */
void camera::right( float right ) {
  // Get a 3-vector representing the 'right' direction.
  v4 c_right = cam_rot.row( 0 );
  v3 c_dv_dir( c_right );
  // Apply a velocity change along that axis.
  delta_v( c_dv_dir, right );
}

/**
 * Move the camera 'forwards', according to its current viewpoint.
 * This adjusts the camera's velocity, rather than moving
 * by a specified number of units.
 */
void camera::fwd( float fwd ) {
  // Get a 3-vector representing the 'forwards' direction.
  v4 c_fwd = cam_rot.row( 2 ) * -1.0f;
  v3 c_dv_dir( c_fwd );
  // Apply a velocity change along that axis.
  delta_v( c_dv_dir, fwd );
}

/**
 * Move the camera 'up', according to its current viewpoint.
 * This adjusts the camera's velocity, rather than moving
 * by a specified number of units.
 */
void camera::up( float up ) {
  // Get a 3-vector representing the 'up' direction.
  v4 c_up = cam_rot.row( 1 );
  v3 c_dv_dir( c_up );
  // Apply a velocity change along that axis.
  delta_v( c_dv_dir, up );
}

/**
 * Helper method to apply an arbitrary change in velocity
 * to the camera object.
 */
void camera::delta_v( v3 dir, float dv ) {
  if ( cam_obj && cam_obj->p_obj ) {
    // If the camera has an associated physics object, activate it
    // and adjust the object's velocity.
    cam_obj->p_obj->rigid_body->activate();
    btVector3 velocity =
      cam_obj->p_obj->rigid_body->getLinearVelocity();
    // Ensure that the new velocities do not exceed the
    // maximum camera speed along any one axis.
    // TODO: Limit total speed instead of per-axis speed?
    float n_x = velocity.getX() + dir.v[ 0 ] * dv;
    if ( n_x > CAM_SPEED ) { n_x = CAM_SPEED; }
    if ( n_x < -CAM_SPEED ) { n_x = -CAM_SPEED; }
    float n_y = velocity.getY() + dir.v[ 1 ] * dv;
    if ( n_y > CAM_SPEED ) { n_y = CAM_SPEED; }
    if ( n_y < -CAM_SPEED ) { n_y = -CAM_SPEED; }
    float n_z = velocity.getZ() + dir.v[ 2 ] * dv;
    if ( n_z > CAM_SPEED ) { n_z = CAM_SPEED; }
    if ( n_z < -CAM_SPEED ) { n_z = -CAM_SPEED; }
    // Apply the new velocity in the Bullet physics simulation.
    btVector3 new_velocity = btVector3( n_x, n_y, n_z );
    cam_obj->p_obj->rigid_body->setLinearVelocity( new_velocity );
    cam_obj->p_obj->rigid_body->applyGravity();
    cam_obj->p_obj->rigid_body->applyCentralImpulse(
      btVector3( 0.0, 0.0, 0.0 ) );
  }
  else {
    // If the camera does not have an associated physics object,
    // adjust its 'cam_move' vector to reflect the desired velocity.
    cam_move.v[ 0 ] += dir.v[ 0 ] * dv * CAM_REF_SPEED;
    cam_move.v[ 1 ] += dir.v[ 1 ] * dv * CAM_REF_SPEED;
    cam_move.v[ 2 ] += dir.v[ 2 ] * dv * CAM_REF_SPEED;
  }
  // Mark that the camera has moved.
  cam_moved = true;
}

/**
 * Apply a 'jump' upwards velocity to the physics object associated
 * with a camera in 'X/Y' rotation mode.
 * TODO: I don't quite remember how the 'CAM_MAX_JUMP' restriction
 * is used; this method should probably be revisited.
 */
void camera::jump( float j ) {
  if ( cam_obj && cam_obj->p_obj ) {
    cam_obj->p_obj->rigid_body->activate();
    btVector3 vel = cam_obj->p_obj->rigid_body->getLinearVelocity();
    if ( vel.getY() <= CAM_MAX_JUMP ) {
      cam_obj->p_obj->rigid_body->applyCentralImpulse(
        btVector3( 0.0, ( j * 4.0f ), 0.0 ) );
    }
  }
}

/**
 * Camera manager constructor. Currently empty,
 * the class doesn't need to initialize anything internally.
 */
camera_manager::camera_manager() {}

/**
 * Camera manager destructor. Deletes any cameras present in its
 * hash map of available cameras.
 */
camera_manager::~camera_manager() {
  // For each entry in the 'cameras' map...
  for ( auto cam_iter = cameras.begin();
        cam_iter != cameras.end();
        ++cam_iter ) {
    // ...delete the camera if it exists.
    if ( cam_iter->second ) {
      delete cam_iter->second;
      cam_iter->second = 0;
    }
  }
}

/**
 * Helper method to update the 'camera' Uniform Buffer Object
 * in the current shader program to match the active camera.
 */
void camera_manager::update_cam_ubo() {
  // This method only makes sense if there is an active camera
  // and an active shader; return early if there isn't.
  if ( !active_camera || !g->s_man || !g->s_man->cur_shader ) {
    return;
  }

  // Setup the UBO. Layout:
  //   0  - 16: T - model translation matrix.
  //   16 - 32: V - view matrix.
  //   32 - 48: P - perspective matrix
  fill_float_buffer( cam_ubo_buf, transpose( id4() ).m, 0, 16 );
  fill_float_buffer( cam_ubo_buf,
                     transpose( active_camera->c_view_matrix ).m,
                     16, 16 );
  fill_float_buffer( cam_ubo_buf,
                     transpose( active_camera->persp_matrix ).m,
                     32, 16 );

  // Bind the camera UBO and get a reference to it.
  GLuint cur_shader = g->s_man->cur_shader;
  glBindBuffer( GL_UNIFORM_BUFFER, cam_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER, cam_ubo, cam_block_buffer );
  cam_ubo_index = glGetUniformBlockIndex( cur_shader, "cam_ubo" );
  // Usually 0 does not seem to be a valid index for OpenGL values.
  // But I guess with UBO indices it is. Go figure.
  // Anyways, if the UBO index exists, buffer the new camera data.
  if ( ( ( int )cam_ubo_index ) >= 0 ) {
    glUniformBlockBinding( cur_shader, cam_ubo_index, cam_ubo );
    glBufferSubData( GL_UNIFORM_BUFFER,
                     0,
                     sizeof( float ) * CAM_UBO_SIZE,
                     cam_ubo_buf );
  }
}

/**
 * Initialize the camera Uniform Buffer Object in the
 * current shader program.
 */
void camera_manager::init_cam_ubo() {
  // Return early if there is no active shader program.
  if ( !g->s_man || !g->s_man->cur_shader ) {
    log_error( "[ERROR] Could not initialize Camera UBO: "
               "no active shader\n" );
    return;
  }
  GLuint cur_shader = g->s_man->cur_shader;

  // If the current camera already has an OpenGL block buffer,
  // delete it before creating a new one.
  if ( cam_block_buffer ) {
    glDeleteBuffers( 1, &cam_block_buffer );
  }

  // Create the OpenGL buffer, and bind it as the active buffer.
  glGenBuffers( 1, &cam_block_buffer );
  glBindBuffer( GL_UNIFORM_BUFFER, cam_block_buffer );
  // Set the size of the buffer to the required number of elements.
  glBufferData( GL_UNIFORM_BUFFER,
                sizeof( float ) * CAM_UBO_SIZE,
                NULL,
                GL_DYNAMIC_DRAW );
  // Bind the buffer to the 'cam_ubo' index in the current
  // shader program. TODO: Verify that 'cam_ubo_index' is valid?
  cam_ubo_index = glGetUniformBlockIndex( cur_shader, "cam_ubo" );
  glUniformBlockBinding( cur_shader, cam_ubo_index, cam_ubo );

  // Call 'update_cam_ubo()' to apply the current camera settings.
  update_cam_ubo();
}

/**
 * Add an existing camera to the camera manager, by string key.
 */
camera* camera_manager::add_camera( string name, camera* c ) {
  // Remove an existing camera with the given key, if any.
  evict_camera( name );
  // Add the new camera.
  cameras[ name ] = c;
  return c;
}

/**
 * Create a new camera in the camera manager with a given string key.
 */
camera* camera_manager::add_camera( string name ) {
  // Remove an existing camera with the given key, if any.
  evict_camera( name );
  // Create a new camera and add it to the manager.
  camera* c = new camera();
  cameras[ name ] = c;
  return c;
}

/**
 * Remove a camera from the hash map of available cameras, if
 * any exist with the given string key.
 */
void camera_manager::evict_camera( string name ) {
  // Try to find a camera by key.
  auto c_iter = cameras.find( name );
  // If one doesn't exist, return.
  if ( c_iter == cameras.end() ) { return; }
  // If one does exist, delete it and erase its entry.
  if ( c_iter->second ) {
    delete c_iter->second;
    c_iter->second = 0;
    cameras.erase( c_iter );
  }
}

/**
 * Try to find a camera by string key in the hash map
 * of available cameras. Returns 0 if no camera was found.
 */
camera* camera_manager::get_camera( string name ) {
  // Try to find a camera by key.
  auto c_iter = cameras.find( name );
  // Return 0 if one couldn't be found.
  if ( c_iter == cameras.end() ) { return 0; }
  // If one could be found, return it.
  return c_iter->second;
}

/**
 * Helper method to switch active cameras. Returns a pointer to
 * the new camera object, or 0 if no camera could be found
 * with the given string key.
 */
camera* camera_manager::switch_camera( string name ) {
  // Get the new camera by name.
  camera* s_cam = get_camera( name );
  // Log an error and return 0 if no camera could be found.
  if ( !s_cam ) {
    log_error( "[ERROR] Cannot switch to non-existant camera: %s\n",
               name.c_str() );
    return 0;
  }
  // Update the active camera.
  active_camera = s_cam;
  // Ensure that the new camera's position data is up-to-date.
  active_camera->update_cam_pos();
  // Return a pointer to the newly-active camera.
  return active_camera;
}

/**
 * Helper method to retrieve the current 'camera'
 * Uniform Buffer Object index.
 */
int camera_manager::get_ubo_index() {
  return cam_ubo;
}

/**
 * Camera Manager 'update' method: perform actions and updates
 * that need to take place once per frame. Part of the 'update'
 * step in the core 'parse input / update / draw' loop.
 */
void camera_manager::update() {
  // Update the active camera's position in the game world.
  if (active_camera) {
    active_camera->update_cam_pos();
  }
}
