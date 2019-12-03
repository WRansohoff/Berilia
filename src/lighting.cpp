#include "lighting.h"

/**
 * Shadow 'depth pass' framebuffer constructor. Currently just
 * calls an initialization method, which might be superfluous.
 */
fb_depth_pass::fb_depth_pass( int map_res_x,
                              int map_res_y,
                              float falloff ) {
  gen_self( map_res_x, map_res_y, falloff, 0 );
}

/**
 * Shadow 'depth pass' framebuffer constructor, with an extra
 * argument for adding a game object (probably a light's indicator)
 * to the 'do_not_draw' list.
 */
fb_depth_pass::fb_depth_pass( int map_res_x, int map_res_y,
                              float falloff, unity* first_ignore ) {
  gen_self( map_res_x, map_res_y, falloff, first_ignore );
}

/**
 * Shadow 'depth pass' framebuffer destructor.
 * Delete the camera and OpenGL buffers.
 * TODO: Is this really all that needs to be deleted?
 */
fb_depth_pass::~fb_depth_pass() {
  if ( shadow_cam ) { delete shadow_cam; }
  if ( shadow_block_buffer ) {
    glDeleteBuffers( 1, &shadow_block_buffer );
  }
}

/** Method to initialize a 'depth map' framebuffer object. */
void fb_depth_pass::gen_self( int map_res_x, int map_res_y,
                              float falloff, unity* first_ignore ) {
  res_x = map_res_x;
  res_y = map_res_y;

  // Delete the camera if one already exists.
  if ( shadow_cam ) { delete shadow_cam; }
  // Initialize the camera.
  //shadow_cam = new camera( g->near, g->far, cam_fov, 1.0f );
  shadow_cam = new camera( 0.1f, 20.0f, cam_fov, 1.0f );

  // Initialize the shadow / framebuffer UBO.
  if ( shadow_block_buffer ) {
    glDeleteBuffers( 1, &shadow_block_buffer );
  }
  glGenBuffers( 1, &shadow_block_buffer );
  glBindBuffer( GL_UNIFORM_BUFFER, shadow_block_buffer );
  glBufferData( GL_UNIFORM_BUFFER,
                sizeof( float ) * CAM_UBO_SIZE,
                NULL,
                GL_DYNAMIC_DRAW );

  // Generate the framebuffer and depth texture.
  glGenFramebuffers( 1, &fbuf );
  glBindFramebuffer( GL_FRAMEBUFFER, fbuf );

  glGenTextures( 1, &fbuf_depth_tex );
  glActiveTexture( GL_TEXTURE0 +
                   g->t_man->num_textures +
                   depth_tex_ind );
  glBindTexture( GL_TEXTURE_2D, fbuf_depth_tex );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_COMPARE_FUNC,
                   GL_LEQUAL );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_COMPARE_MODE,
                   GL_NONE );
  // This causes 0x0500 GL_INVALID_ENUM. Not sure why.
  glTexParameteri( GL_TEXTURE_2D,
                   GL_DEPTH_TEXTURE_MODE,
                   GL_INTENSITY );
  glTexImage2D( GL_TEXTURE_2D,
                0,
                GL_DEPTH24_STENCIL8,
                map_res_x,
                map_res_y,
                0,
                GL_DEPTH_STENCIL,
                GL_UNSIGNED_INT_24_8,
                NULL );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER,
                   GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_WRAP_S,
                   GL_CLAMP_TO_BORDER );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_WRAP_T,
                   GL_CLAMP_TO_BORDER );

  glFramebufferTexture2D( GL_FRAMEBUFFER,
                          GL_DEPTH_STENCIL_ATTACHMENT,
                          GL_TEXTURE_2D,
                          fbuf_depth_tex,
                          0 );
  GLenum draw_bufs[] = { GL_NONE };
  glDrawBuffers( 1, draw_bufs );
  glReadBuffer( GL_NONE );
  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  if ( GL_FRAMEBUFFER_COMPLETE != status ) {
    log_error( "[ERROR (fb_depth_pass)] Incomplete framebuffer. "
               "Status code: %i\n",
               status );
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  // If a 'first_ignore' game object is provided, add it to the
  // 'do_not_draw' array. This is really just a convenience for
  // lights, which have debugging meshes blocking their views.
  if ( first_ignore ) {
    do_not_draw.push_back( first_ignore );
  }
}

/** Draw to the 'depth-pass' framebuffer. */
void fb_depth_pass::draw() {
  // Bind the object's framebuffer, and set its viewport size.
  glBindFramebuffer( GL_FRAMEBUFFER, fbuf );
  glViewport( 0, 0, res_x, res_y );
  // Clear the framebuffer's depth mask.
  //glClearColor( 0.0, 0.0, 0.0, 1.0 ); // TODO: Remove or uncomment?
  glClear( GL_DEPTH_BUFFER_BIT );

  // Store previous shader / camera states to restore
  // after drawing to this framebuffer.
  GLuint last_shader = g->s_man->cur_shader;
  camera* last_cam = g->c_man->active_camera;
  // Prepare shader / camera settings for drawing a shadow
  // map from the light's perspective.
  g->s_man->swap_shader( g->depth_shader_key );
  g->c_man->active_camera = shadow_cam;
  g->c_man->active_camera->update_cam_pos();

  // Update the shadow UBO.
  fill_float_buffer( shadow_ubo_buf, transpose( id4() ).m, 0, 16 );
  fill_float_buffer(
    shadow_ubo_buf,
    transpose( g->c_man->active_camera->c_view_matrix ).m,
    16,
    16 );
  fill_float_buffer(
    shadow_ubo_buf,
    transpose( g->c_man->active_camera->persp_matrix ).m,
    32,
    16 );

  // Overwrite the normal camera UBO with the shadow map data,
  // so the framebuffer is drawn from the light's perspective.
  glBindBuffer( GL_UNIFORM_BUFFER, g->c_man->cam_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER,
                    g->c_man->cam_ubo,
                    g->c_man->cam_block_buffer );
  GLuint cam_ubo_index =
    glGetUniformBlockIndex( g->s_man->cur_shader, "cam_ubo" );
  if ( ( ( int )cam_ubo_index ) >= 0 ) {
    glUniformBlockBinding( g->s_man->cur_shader,
                           cam_ubo_index,
                           shadow_ubo );
    glBufferSubData( GL_UNIFORM_BUFFER,
                     0,
                     sizeof( float ) * CAM_UBO_SIZE,
                     shadow_ubo_buf );
  }

  // Only draw game objects that should cast shadows.
  // For now, that is everything except the contents
  // of the 'do_not_draw' array.
  // This function will be called once for each active 'unity'.
  function<void( unity* u )> f = [ &dnd = do_not_draw ]( unity* u ) {
    if ( u ) {
      bool should_draw = true;
      // Check whether the game object is in the 'do_not_draw' array.
      for ( int j = 0; j < dnd.size(); ++j ) {
        if ( u == dnd[ j ] ) {
          should_draw = false;
        }
      }
      // If it isn't in the 'do_not_draw' array, draw the object.
      if ( should_draw ) { u->draw(); }
    }
  };
  // The 'unity_manager' object's 'for_each' method accepts
  // a function, and calls that function once for each active
  // 'unity' object in the game world.
  g->u_man->for_each( f, true );

  // Reset OpenGL stuff for normal drawing.
  // Bind the regular framebuffer at index 0.
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  // Restore the active camera and associated shader data.
  g->c_man->active_camera = last_cam;
  g->c_man->active_camera->update_cam_pos();
  // Restore the previous shader program.
  g->s_man->swap_shader( last_shader );
}

/**
 * Phong 'point light' constructor: creates a light
 * which illuminates every direction.
 */
phong_light::phong_light( v4 position, v4 amb, v4 dif, v4 spec ) {
  pos = position;
  a = amb;
  d = dif;
  s = spec;
  dir = v3( 0.0f, 0.0f, 1.0f );
}

/**
 * Phong 'spotlight' constructor: creates a light
 * which illuminates only a certain angular area
 * in one direction.
 */
phong_light::phong_light( v4 position, v4 amb, v4 dif, v4 spec,
                          v3 spot_dir, float spotlight_rads ) {
  pos = position;
  a = amb;
  d = dif;
  s = spec;
  type = BRLA_LIGHT_PHONG_SPOT;
  dir = spot_dir;
  spot_rads = spotlight_rads;
}

/**
 * Phong light destructor: remove the 'indicator' game object
 * if any, and delete the shadow map framebuffer if one exists.
 */
phong_light::~phong_light() {
  g->u_man->evict_unity( indicator );
  if ( shadow_depth_fb ) { delete shadow_depth_fb; }
}

/**
 * Make an existing light into a shadow caster.
 */
void phong_light::make_shadow_caster() {
  // Enable shadowcasting; populate the fb / camera if necessary.
  cast_shadows = true;
  if ( !shadow_depth_fb ) {
    shadow_depth_fb = new fb_depth_pass( 1024,
                                         1024,
                                         falloff,
                                         indicator );
    // We want updates to the light to persist to the UBO, so
    // associate the shadow-map camera with the 'light indicator'
    // game object if both exist.
    if ( indicator && shadow_depth_fb->shadow_cam ) {
      shadow_depth_fb->shadow_cam->cam_obj = indicator;
    }
  }
}

/** Lighting manager constructor: initialize the OpenGL buffers. */
lighting_manager::lighting_manager() {
  phong_ubo_size = BRLA_PHONG_LIGHT_SIZE * BRLA_MAX_PHONG_LIGHTS;
  // Extra space for options. Currently just used for telling
  // the shader how many lights were actually passed in.
  phong_ubo_size += 4;
  phong_ubo_buf = new GLfloat[ phong_ubo_size ];
  memset( phong_ubo_buf, 0, sizeof( GLfloat ) * phong_ubo_size );
}

/** Lighting manager destructor: delete the OpenGL buffers. */
lighting_manager::~lighting_manager() {
  if ( phong_ubo_buf ) { delete phong_ubo_buf; }
  if ( phong_block_buffer ) {
    glDeleteBuffers( 1, &phong_block_buffer );
  }

  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ] ) {
      delete phong_lights[ i ];
    }
  }
}

/**
 * Add an existing phong light to the lighting manager's
 * array of active lights.
 */
void lighting_manager::add_phong_light( phong_light* light ) {
  phong_lights.push_back( light );
}

/**
 * Create a new phong light and add it to the lighting
 * manager's array of active lights.
 */
phong_light* lighting_manager::add_phong_light( v4 pos,
                                                v4 amb,
                                                v4 dif,
                                                v4 spec ) {
  phong_light* l = new phong_light( pos, amb, dif, spec );
  add_phong_light( l );
  return l;
}

/** Evict a phong light from the manager's array of active lights. */
void lighting_manager::evict_phong_light( phong_light* light ) {
  // Iterate over the array to look for the given light.
  for ( auto l_iter = phong_lights.begin();
        l_iter != phong_lights.end();
        ++l_iter ) {
    phong_light* l = *l_iter;
    // If a match is found, delete it and return.
    if ( l == light ) {
      delete l;
      phong_lights.erase( l_iter );
      return;
    }
  }
}

/**
 * Find the phong light associated with a given game object, if any.
 * Returns 0 if no light was found.
 */
phong_light* lighting_manager::get_by_indicator( unity* u ) {
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ] && phong_lights[ i ]->indicator == u ) {
      return phong_lights[ i ];
    }
  }
  return 0;
}

/**
 * Helper method to remove all lights from the
 * array of active lights, and delete them.
 */
void lighting_manager::clear_phong_lights() {
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ] ) {
      delete phong_lights[ i ];
      phong_lights[ i ] = 0;
    }
  }
  phong_lights.clear();
}

/**
 * Initialize the phong light Uniform Buffer Object.
 */
void lighting_manager::init_lighting_ubo() {
  glGenBuffers( 1, &phong_block_buffer );
  glBindBuffer( GL_UNIFORM_BUFFER, phong_block_buffer );
  glBufferData( GL_UNIFORM_BUFFER,
                phong_ubo_size * sizeof( GLfloat ),
                NULL,
                GL_DYNAMIC_DRAW );
  phong_ubo_index = glGetUniformBlockIndex( g->s_man->cur_shader,
                                            "phong_ubo" );
  glUniformBlockBinding( g->s_man->cur_shader,
                         phong_ubo_index,
                         phong_ubo );
  // Once the buffers are created, write initial values to them.
  write_lighting_ubo();
}

/**
 * Lighting manager 'update' step. Update the 'indicator' game
 * objects for active lights, buffer the lights closest to
 * the player in the lighting Uniform Buffer Object, update
 * shadow-mapping objects for lights with shadow-casting enabled,
 * and write the lighting UBO values.
 * TODO: Better comments throughout this method.
 */
void lighting_manager::update() {
  // Update the 'indicator' game objects.
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ]->indicator ) {
      phong_lights[ i ]->indicator->update();
    }
  }

  // Find the N closest lights to the player.
  // TODO: Use a better data structure for sorting by distance. I
  // can do this in log(n) instead of n for the closest lights.
  if ( g->c_man->active_camera == NULL ) { return; }
  v3 c_pos = g->c_man->active_camera->cam_pos;
  // Create an array of N lights to buffer, and zero out
  // its entries since there may end up being fewer lights.
  phong_light* closest[ BRLA_MAX_PHONG_LIGHTS ];
  memset( closest,
          0,
          sizeof( phong_light* ) * BRLA_MAX_PHONG_LIGHTS );
  bool full_buffer = false, first_pass = false;
  int max_ind = 0;
  float max_dist2 = 0.0f;
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    v3 light_pos = v3( phong_lights[ i ]->pos );
    float light1_dist = distance2( c_pos, light_pos );
    // TODO: Better sorting; this isn't ideal.
    for ( int j = 0; j < BRLA_MAX_PHONG_LIGHTS; ++j ) {
      if ( !closest[ j ] ) {
        closest[ j ] = phong_lights[ i ];
        if ( j == BRLA_MAX_PHONG_LIGHTS - 1 ) {
          full_buffer = true;
          first_pass = true;
        }
        break;
      }
      else if ( full_buffer ) {
        float light2_dist = distance2( c_pos,
                                       v3( closest[ j ]->pos ) );
        if ( light2_dist > max_dist2 ) {
          max_dist2 = light2_dist;
          max_ind = j;
        }
      }
    }
    if ( full_buffer ) {
      if ( first_pass ) { first_pass = false; }
      else if ( light1_dist < max_dist2 ) {
        closest[ max_ind ] = phong_lights[ i ];
      }
    }
  }

  // Buffer the lights.
  int num_lights = 0;
  int offset = 4;
  for ( int i = 0; i < BRLA_MAX_PHONG_LIGHTS; ++i ) {
    if ( closest[ i ] ) {
      float l_type = ( float )( closest[ i ]->type );
      fill_float_buffer( phong_ubo_buf, closest[ i ]->pos.v,
                         offset, 4 );
      fill_float_buffer( phong_ubo_buf, closest[ i ]->a.v,
                         offset + 4, 4 );
      fill_float_buffer( phong_ubo_buf, closest[ i ]->d.v,
                         offset + 8, 4 );
      fill_float_buffer( phong_ubo_buf, closest[ i ]->s.v,
                         offset + 12, 4 );
      fill_float_buffer( phong_ubo_buf, &closest[ i ]->specular_exp,
                         offset + 16, 1 );
      fill_float_buffer( phong_ubo_buf, &closest[ i ]->falloff,
                         offset + 17, 1 );
      fill_float_buffer( phong_ubo_buf, &l_type,
                         offset + 18, 1 );
      if ( closest[ i ]->cast_shadows ) {
        phong_ubo_buf[ offset + 19 ] = 1.0f;
      }
      else {
        phong_ubo_buf[ offset + 19 ] = 0.0f;
      }
      fill_float_buffer( phong_ubo_buf, closest[ i ]->dir.v,
                         offset + 20, 3 );
      fill_float_buffer( phong_ubo_buf, &closest[ i ]->spot_rads,
                         offset + 23, 1 );
      offset += BRLA_PHONG_LIGHT_SIZE;
      num_lights += 1;

      // Buffer the shadow UBO/uniform, if applicable.
      fb_depth_pass* sfb = closest[ i ]->shadow_depth_fb;
      if ( closest[ i ]->cast_shadows && sfb ) {
        glBindBuffer( GL_UNIFORM_BUFFER, sfb->shadow_block_buffer );
        glBindBufferBase( GL_UNIFORM_BUFFER,
                          sfb->shadow_ubo,
                          sfb->shadow_block_buffer );
        sfb->shadow_ubo_index =
          glGetUniformBlockIndex( g->s_man->cur_shader,
                                  "shadow_cam_ubo" );
        if ( ( ( int )sfb->shadow_ubo_index ) >= 0 ) {
          glUniformBlockBinding( g->s_man->cur_shader,
                                 sfb->shadow_ubo_index,
                                 sfb->shadow_ubo );
          glBufferSubData( GL_UNIFORM_BUFFER,
                           0,
                           sizeof( float ) * CAM_UBO_SIZE,
                           sfb->shadow_ubo_buf );
        }

        int shadow_sampler_loc =
          glGetUniformLocation( g->s_man->cur_shader,
                                "shadow_depth_map_sampler" );
        if ( shadow_sampler_loc >= 0 ) {
          int shadow_tex_sampler =
            ( g->t_man->num_textures + sfb->depth_tex_ind );
          glActiveTexture( GL_TEXTURE0 + shadow_tex_sampler );
          glBindTexture( GL_TEXTURE_2D, sfb->fbuf_depth_tex );
          glUniform1i( shadow_sampler_loc,
                       ( float )shadow_tex_sampler );
        }
      }
    }
  }

  // Buffer the phong light options.
  float f_num_lights = ( float )num_lights;
  fill_float_buffer( phong_ubo_buf, &f_num_lights, 0, 1 );
  fill_float_buffer( phong_ubo_buf, &f_num_lights, 1, 1 );
  fill_float_buffer( phong_ubo_buf, &f_num_lights, 2, 1 );
  fill_float_buffer( phong_ubo_buf, &f_num_lights, 3, 1 );
  // Write to the phong light Uniform Buffer Object.
  write_lighting_ubo();
}

/**
 * Draw the game object indicators associated with the active
 * phong lights. TODO: If these are also tracked in the
 * 'unity_manager' object, do they get drawn twice?
 */
void lighting_manager::draw() {
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ]->indicator ) {
      phong_lights[ i ]->indicator->draw();
    }
  }
}

/**
 * Draw the shadow depth framebuffers for lights that have
 * shadow-casting enabled.
 */
void lighting_manager::draw_shadow_casters() {
  for ( int i = 0; i < phong_lights.size(); ++i ) {
    if ( phong_lights[ i ] && phong_lights[ i ]->shadow_depth_fb ) {
      phong_lights[ i ]->shadow_depth_fb->draw();
    }
  }
}

/** Write values to the phong light Uniform Buffer Object. */
void lighting_manager::write_lighting_ubo() {
  // Bind and buffer the lighting UBO.
  glBindBuffer( GL_UNIFORM_BUFFER, phong_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER,
                    phong_ubo,
                    phong_block_buffer );
  phong_ubo_index = glGetUniformBlockIndex( g->s_man->cur_shader,
                                            "phong_ubo" );
  if ( ( ( int )phong_ubo_index ) >= 0 ) {
    glUniformBlockBinding( g->s_man->cur_shader,
                           phong_ubo_index,
                           phong_ubo );
    glBufferSubData( GL_UNIFORM_BUFFER,
                     0,
                     phong_ubo_size * sizeof(GLfloat),
                     phong_ubo_buf );
  }
}
