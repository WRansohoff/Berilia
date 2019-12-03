#include "game.h"

/**
 * Main 'game' object constructor. Initialize GLEW, GLFW,
 * the main window, and the physics simulation.
 */
game::game( int w, int h ) {
  // Initialize window width / height values.
  g_win_w = w;
  g_win_h = h;
  aspect_ratio = ( float )g_win_w / ( float )g_win_h;
  // Set up GLFW.
  log( "Initialize GLFW\n%s\n", glfwGetVersionString() );
  glfwSetErrorCallback( glfw_log_err );
  if ( !glfwInit() ) {
    log_error( "[ERROR] Could not start GLFW.\n" );
    should_quit = true;
    return;
  }
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, BRLA_MAJOR_VERSION );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, BRLA_MINOR_VERSION );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_SAMPLES, BRLA_AA_SAMPLES );
  glfwWindowHint( GLFW_DEPTH_BITS, 32 );
  // Initialize the main window.
  window = glfwCreateWindow( g_win_w, g_win_h,
                             "Berilia", NULL, NULL );
  if ( !window ) {
    log_error( "[ERROR] Could not initialize window.\n" );
    glfwTerminate();
    should_quit = true;
    return;
  }
  glfwMakeContextCurrent(window);
  // Setup GLFW callbacks.
  glfwSetWindowSizeCallback( window, glfw_win_resize );
  glfwSetCursorPosCallback( window, glfw_mouse_pos );
  glfwSetMouseButtonCallback( window, glfw_mouse_button );
  // Initialize GLEW.
  glewExperimental = GL_TRUE;
  GLenum glew_err = glewInit();
  if ( glew_err != GLEW_OK ) {
    log_error( "[ERROR] Could not wrangle glew: %s\n",
               glewGetErrorString( glew_err ) );
    glfwTerminate();
    should_quit = true;
    return;
  }
  // Log compatibility information.
  const GLubyte* renderer = glGetString( GL_RENDERER );
  const GLubyte* version = glGetString( GL_VERSION );
  log( "Renderer: %s\n", renderer );
  log( "OpenGL version supported: %s\n", version );
  // Enable depth testing, and define less depth as 'closer'.
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  // Initial alpha blending function for limited transparency.
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glClearColor( 0.5f, 0.5f, 0.6f, 1.0f );
  // Create the 'physics_manager' object.
  p_man = new physics_manager();
}

/** Main 'game' object destructor. Delete each system manager. */
game::~game() {
  if (l_man) { delete l_man; }
  if (t_man) { delete t_man; }
  if (s_man) { delete s_man; }
  if (c_man) { delete c_man; }
  if (u_man) { delete u_man; }
  if (g_man) { delete g_man; }
  if (p_man) { delete p_man; }
}

/**
 * Helper method to perform secondary initialization
 * steps for the main 'game' object.
 * Initialize individual system manager objects, load
 * shader programs, and setup core OpenGL data structures.
 */
void game::init() {
  // Create basic system managers.
  l_man = new lighting_manager();
  t_man = new texture_manager();

  // Load the basic font atlas.
  t_man->add_mapping_by_fn( f_mono );
  texture* mono_font = t_man->get( f_mono );
  flip_tex_V( mono_font->tex_buffer,
              mono_font->tex_x,
              mono_font->tex_y,
              mono_font->tex_n );
  mono_font->load_uniform_font_atlas();

  // Continue initializing system managers.
  s_man = new shader_manager();
  c_man = new camera_manager();
  u_man = new unity_manager();
  g_man = new gui_manager();

  // Load shaders.
  s_man->add_shader_prog( normal_shader_key,
                          normal_vert_shader_fn,
                          normal_frag_shader_fn );
  s_man->add_shader_prog( phys_debug_shader_key,
                          phys_debug_vert_shader_fn,
                          phys_debug_frag_shader_fn );
  s_man->add_shader_prog( gui_shader_key,
                          gui_vert_shader_fn,
                          gui_frag_shader_fn );
  s_man->add_shader_prog( world_gui_shader_key,
                          normal_vert_shader_fn,
                          world_gui_frag_shader_fn );
  s_man->add_shader_prog( color_shader_key,
                          normal_vert_shader_fn,
                          color_frag_shader_fn );
  s_man->add_shader_prog( pp_edge_detect_shader_key,
                          gui_vert_shader_fn,
                          pp_edge_detect_shader_fn );
  s_man->add_shader_prog( pp_img_shim_shader_key,
                          gui_vert_shader_fn,
                          pp_img_shim_frag_shader_fn );
  s_man->add_shader_prog( particles_shader_key,
                          particles_vert_shader_fn,
                          particles_frag_shader_fn );
  s_man->add_shader_prog( depth_shader_key,
                          normal_vert_shader_fn,
                          depth_frag_shader_fn );

  // Setup the global 'world' UBO.
  float default_buf[ BRLA_GAME_UBO_SIZE ];
  memset( default_buf, 0, sizeof( float ) * BRLA_GAME_UBO_SIZE );
  fill_float_buffer( world_ubo_buf,
                     default_buf,
                     0,
                     BRLA_GAME_UBO_SIZE );
  world_ubo_buf[ 0 ] = near;
  world_ubo_buf[ 1 ] = far;
  glGenBuffers( 1, &world_block_buffer );
  glBindBuffer( GL_UNIFORM_BUFFER, world_block_buffer );
  glBufferData( GL_UNIFORM_BUFFER,
                sizeof( float ) * BRLA_GAME_UBO_SIZE,
                NULL,
                GL_DYNAMIC_DRAW );
  // Initial UBO update.
  write_world_ubo();

  // Start using the 'normal' shader.
  s_man->swap_shader( normal_shader_key );

  // Setup the initial camera object.
  c_man->add_camera( cam_normal );
  c_man->switch_camera( cam_normal );
  c_man->init_cam_ubo();
  if ( !editor && c_man->active_camera ) {
    c_man->active_camera->cam_obj =
      new u_player_mesh( v3( 0.0f, 0.0f, 0.0f ) );
    c_man->active_camera->cam_obj->p_obj->
      rigid_body->setSleepingThresholds( 0.01, 1.0 );
    float d_b = ang_to_rad( 0.0 );
    c_man->active_camera->cam_obj->p_obj->set_global_angle_limit(
      v3( -d_b, -d_b, -d_b ), v3( d_b, d_b, d_b ) );
  }

  // Initialize the lighting Uniform Buffer Object.
  l_man->init_lighting_ubo();
}

/** Process the game loop's "input" / "update" / "draw" steps. */
int game::process_game_loop() {
  // Log any OpenGL errors that may have occurred recently.
  log_gl_errors();

  // Update inter-frame timers.
  static double prev_sec = glfwGetTime();
  double cur_sec = glfwGetTime();
  elapsed_sec = cur_sec - prev_sec;
  prev_sec = cur_sec;

  // Process player input.
  glfwPollEvents();
  // Quit the game if the 'escape' key is pressed.
  // (Except in 'level editor' mode, where there
  //  may be unsaved progress)
  if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
    if ( !editor ) {
      glfwSetWindowShouldClose( window, 1 );
    }
  }
  // Don't do anything if the game is over.
  if ( lost ) { return 0; }

  // If there is a currently-selected GUI panel,
  // check for key presses and send them to the GUI.
  if ( g_man->cur_gui && g_man->cur_gui->selected ) {
    // Check text input for keys where capitalization
    // does not currently matter.
    check_text_input( GLFW_KEY_BACKSPACE, '|' );
    check_text_input( GLFW_KEY_SPACE, ' ' );
    check_text_input( GLFW_KEY_PERIOD, '.' );
    check_text_input( GLFW_KEY_0, '0' );
    check_text_input( GLFW_KEY_1, '1' );
    check_text_input( GLFW_KEY_2, '2' );
    check_text_input( GLFW_KEY_3, '3' );
    check_text_input( GLFW_KEY_4, '4' );
    check_text_input( GLFW_KEY_5, '5' );
    check_text_input( GLFW_KEY_6, '6' );
    check_text_input( GLFW_KEY_7, '7' );
    check_text_input( GLFW_KEY_8, '8' );
    check_text_input( GLFW_KEY_9, '9' );
    // Process capitalized inputs if either 'shift' key is pressed.
    if ( glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS ||
         glfwGetKey( window, GLFW_KEY_RIGHT_SHIFT ) == GLFW_PRESS ) {
      check_text_input( GLFW_KEY_MINUS, '_', '-' );
      check_text_input( GLFW_KEY_A, 'A', 'a' );
      check_text_input( GLFW_KEY_B, 'B', 'b' );
      check_text_input( GLFW_KEY_C, 'C', 'c' );
      check_text_input( GLFW_KEY_D, 'D', 'd' );
      check_text_input( GLFW_KEY_E, 'E', 'e' );
      check_text_input( GLFW_KEY_F, 'F', 'f' );
      check_text_input( GLFW_KEY_G, 'G', 'g' );
      check_text_input( GLFW_KEY_H, 'H', 'h' );
      check_text_input( GLFW_KEY_I, 'I', 'i' );
      check_text_input( GLFW_KEY_J, 'J', 'j' );
      check_text_input( GLFW_KEY_K, 'K', 'k' );
      check_text_input( GLFW_KEY_L, 'L', 'l' );
      check_text_input( GLFW_KEY_M, 'M', 'm' );
      check_text_input( GLFW_KEY_N, 'N', 'n' );
      check_text_input( GLFW_KEY_O, 'O', 'o' );
      check_text_input( GLFW_KEY_P, 'P', 'p' );
      check_text_input( GLFW_KEY_Q, 'Q', 'q' );
      check_text_input( GLFW_KEY_R, 'R', 'r' );
      check_text_input( GLFW_KEY_S, 'S', 's' );
      check_text_input( GLFW_KEY_T, 'T', 't' );
      check_text_input( GLFW_KEY_U, 'U', 'u' );
      check_text_input( GLFW_KEY_V, 'V', 'v' );
      check_text_input( GLFW_KEY_W, 'W', 'w' );
      check_text_input( GLFW_KEY_X, 'X', 'x' );
      check_text_input( GLFW_KEY_Y, 'Y', 'y' );
      check_text_input( GLFW_KEY_Z, 'Z', 'z' );
      check_text_input( GLFW_KEY_SLASH, '?', '/' );
    }
    // Process un-capitalized inputs if
    // neither 'shift' key is pressed.
    else {
      check_text_input( GLFW_KEY_MINUS, '-', '_' );
      check_text_input( GLFW_KEY_A, 'a', 'A' );
      check_text_input( GLFW_KEY_B, 'b', 'B' );
      check_text_input( GLFW_KEY_C, 'c', 'C' );
      check_text_input( GLFW_KEY_D, 'd', 'D' );
      check_text_input( GLFW_KEY_E, 'e', 'E' );
      check_text_input( GLFW_KEY_F, 'f', 'F' );
      check_text_input( GLFW_KEY_G, 'g', 'G' );
      check_text_input( GLFW_KEY_H, 'h', 'H' );
      check_text_input( GLFW_KEY_I, 'i', 'I' );
      check_text_input( GLFW_KEY_J, 'j', 'J' );
      check_text_input( GLFW_KEY_K, 'k', 'K' );
      check_text_input( GLFW_KEY_L, 'l', 'L' );
      check_text_input( GLFW_KEY_M, 'm', 'M' );
      check_text_input( GLFW_KEY_N, 'n', 'N' );
      check_text_input( GLFW_KEY_O, 'o', 'O' );
      check_text_input( GLFW_KEY_P, 'p', 'P' );
      check_text_input( GLFW_KEY_Q, 'q', 'Q' );
      check_text_input( GLFW_KEY_R, 'r', 'R' );
      check_text_input( GLFW_KEY_S, 's', 'S' );
      check_text_input( GLFW_KEY_T, 't', 'T' );
      check_text_input( GLFW_KEY_U, 'u', 'U' );
      check_text_input( GLFW_KEY_V, 'v', 'V' );
      check_text_input( GLFW_KEY_W, 'w', 'W' );
      check_text_input( GLFW_KEY_X, 'x', 'X' );
      check_text_input( GLFW_KEY_Y, 'y', 'Y' );
      check_text_input( GLFW_KEY_Z, 'z', 'Z' );
      check_text_input( GLFW_KEY_SLASH, '/', '?' );
    }

    // Process left arrow key presses and releases.
    if ( glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS &&
         !L_arrow_down ) {
      if ( g_man && g_man->cur_gui ) {
        g_man->cur_gui->cursor_L();
      }
      L_arrow_down = true;
    }
    else if ( L_arrow_down &&
              glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_RELEASE ) {
      L_arrow_down = false;
    }
    // Process right arrow key presses and releases.
    if ( glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS &&
         !R_arrow_down ) {
      if ( g_man && g_man->cur_gui ) {
        g_man->cur_gui->cursor_R();
      }
      R_arrow_down = true;
    }
    else if ( R_arrow_down &&
              glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_RELEASE ) {
      R_arrow_down = false;
    }
  }
  // If there is no currently-selected GUI panel, key
  // presses should be processed in the 3D game world.
  else {
    // Find the active camera, and calculate
    // how far it should move in this frame if it has no
    // physics object.
    camera* cam = c_man->active_camera;
    float factor = 0.5;
    float cam_step = CAM_SPEED * elapsed_sec * factor;

    // Process camera movement if the game isn't paused.
    if ( !paused ) {
      // In 'quaternion' rotation mode, two keys rotate in either
      // direction along each axis.
      if ( cam->cam_rot_type == B_CAM_ROT_QUAT ) {
        // Pitch / yaw input.
        if ( glfwGetKey( window, GLFW_KEY_H ) ) {
          cam->yaw( -CAM_ROT_SPEED * elapsed_sec );
        }
        if ( glfwGetKey( window, GLFW_KEY_L ) ) {
          cam->yaw( CAM_ROT_SPEED * elapsed_sec );
        }
        if ( glfwGetKey( window, GLFW_KEY_K ) ) {
          cam->pitch( -CAM_ROT_SPEED * elapsed_sec );
        }
        if ( glfwGetKey( window, GLFW_KEY_J ) ) {
          cam->pitch( CAM_ROT_SPEED * elapsed_sec );
        }
      }
      // In 'X / Y' camera mode, the mouse controls
      // the camera's pitch and yaw.
      // TODO: Comment this block of code.
      else if ( cam->cam_rot_type == B_CAM_ROT_XY ) {
        const int mid_w = 0;
        const int mid_h = 0;
        int mid_l_x = ( g_win_w - mid_w ) / 2;
        int mid_r_x = mid_l_x + mid_w;
        int mid_t_y = ( g_win_h - mid_h ) / 2;
        int mid_b_y = mid_t_y + mid_h;
        if ( mouse_x <= mid_l_x || mouse_x >= mid_r_x ) {
          cam_x_theta += ( mouse_x - g_win_w / 2 ) / 1000.0;
          if ( cam_x_theta < -360.0 ) { cam_x_theta += 360.0; }
          else if ( cam_x_theta > 360.0 ) { cam_x_theta -= 360.0; }

          cam_x_quat = quat( cam_x_theta, 0.0, 1.0, 0.0 );
          m4 x_rot = quaternion_to_rotation( cam_x_quat );
          cam_y_axis = x_rot.row( 0 );
          cam_y_quat = quat( cam_y_theta,
                             cam_y_axis.v[ 0 ],
                             cam_y_axis.v[ 1 ],
                             cam_y_axis.v[ 2 ] );
        }
        if ( mouse_y <= mid_t_y || mouse_y >= mid_b_y ) {
          cam_y_theta += ( mouse_y - g_win_h / 2 ) / 1000.0;
          if ( cam_y_theta < -360.0 ) { cam_y_theta += 360.0; }
          else if ( cam_y_theta > 360.0 ) { cam_y_theta -= 360.0; }
          cam_y_quat = quat( cam_y_theta,
                             cam_y_axis.v[ 0 ],
                             cam_y_axis.v[ 1 ],
                             cam_y_axis.v[ 2 ] );
        }
        quat total_quat = cam_x_quat * cam_y_quat;
        cam->rotate( total_quat );
      }
      // Control the camera's 'roll' rotation with keyboard input
      // in either camera mode.
      if ( glfwGetKey( window, GLFW_KEY_Q ) ) {
        cam->roll( CAM_ROT_SPEED * elapsed_sec );
      }
      if ( glfwGetKey( window, GLFW_KEY_E ) ) {
        cam->roll( -CAM_ROT_SPEED * elapsed_sec );
      }

      // Apply changes to the camera's velocity
      // in the physics simulation based on WASD input.
      if ( glfwGetKey( window, GLFW_KEY_A ) ) {
        cam->right( -cam_step );
      }
      if ( glfwGetKey( window, GLFW_KEY_D ) ) {
        cam->right( cam_step );
      }
      if ( glfwGetKey( window, GLFW_KEY_W ) ) {
        cam->fwd( cam_step );
      }
      if ( glfwGetKey( window, GLFW_KEY_S ) ) {
        cam->fwd( -cam_step );
      }
      // Use 'U' and 'O' for vertical movement.
      // TODO: Restrict this to 'quaternion' camera mode
      // and / or 'zero-G' mode?
      if ( glfwGetKey( window, GLFW_KEY_U ) ) {
        cam->up( cam_step );
      }
      if ( glfwGetKey( window, GLFW_KEY_O ) ) {
        cam->up( -cam_step );
      }
      // Handle jumping.
      // TODO: Restrict this to 'normal gravity' mode?
      if ( glfwGetKey( window, GLFW_KEY_SPACE ) ) {
        cam->jump( cam_step );
      }
      // Stabilization.
      // TODO: Restrict this to 'zero-G' mode?
      if ( glfwGetKey( window, GLFW_KEY_V ) ) {
        cam->stabilize( cam_step );
      }
      // Use the 'B' key to swap between camera types.
      // TODO: Remove this debug setting?
      if ( glfwGetKey( window, GLFW_KEY_B ) ) {
        if ( !pressed_keys[ 'b' ] ) {
          if ( cam->cam_rot_type == B_CAM_ROT_QUAT ) {
            cam->cam_rot_type = B_CAM_ROT_XY;
            // Hide the mouse cursor in 'X / Y' mode,
            // since the mouse is used for looking around.
            glfwSetInputMode( window,
                              GLFW_CURSOR,
                              GLFW_CURSOR_DISABLED );
          }
          else if ( cam->cam_rot_type == B_CAM_ROT_XY ) {
            cam->cam_rot_type = B_CAM_ROT_QUAT;
            // Show the cursor in 'quaternion' mode,
            // since the keyboard handles all rotation and movement.
            glfwSetInputMode( window,
                              GLFW_CURSOR,
                              GLFW_CURSOR_NORMAL );
          }
        }
        pressed_keys[ 'b' ] = true;
      }
      else { pressed_keys[ 'b' ] = false; }

      // 'F': 'use' key, for interacting with objects
      // in the game world.
      if ( glfwGetKey( window, GLFW_KEY_F ) == GLFW_PRESS &&
           !use_key_down ) {
        if ( looking_at_unity && looking_at_unity->use_script ) {
          looking_at_unity->use_script->call( 0.0f );
        }
        use_key_down = true;
      }
      else if ( glfwGetKey( window, GLFW_KEY_F ) == GLFW_RELEASE &&
                use_key_down ) {
        use_key_down = false;
      }
    }

    // 'P': Toggle whether to draw the physics debugging wireframes.
    if ( glfwGetKey( window, GLFW_KEY_P ) ) {
      draw_phys_debug = !draw_phys_debug;
    }
  }

  // Process the 'tab' key as a special case.
  // Pause the game if it is running normally,
  // or cycle through input elements if it is
  // in 'level editor' mode (TODO).
  if ( glfwGetKey( window, GLFW_KEY_TAB ) == GLFW_PRESS &&
       !tab_key_down ) {
    if ( !editor ) {
      // Pause / unpause.
      paused = !paused;

      if ( paused ) {
        // Add a 'paused' overlay.
        if ( g_man && g_man->cur_gui && g_man->cur_gui->root ) {
          gui_panel* r = g_man->cur_gui->root;
          r->clear_all();
          r->load_pause_screen();
        }
      }
      else {
        // Remove any overlays.
        if ( g_man && g_man->cur_gui && g_man->cur_gui->root ) {
          gui_panel* r = g_man->cur_gui->root;
          r->clear_all();
        }
      }
    }
    else {
      // If we're in editor mode, make 'tab' a convenience key to
      // cycle through selectable text fields (TODO).
      /*
      if ( g_man && g_man->cur_gui ) {
        g_man->cur_gui->cycle_selected();
      }
      */
    }

    tab_key_down = true;
  }
  else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && tab_key_down) {
    tab_key_down = false;
  }

  // Step the physics simulation, if the game is not paused.
  if ( !paused ) {  p_man->update(); }

  // Perform a raycast to update the game's record of the object
  // that the player is currently looking at, if any.
  // TODO: Is this necessary? Can it be moved or removed?
  if (!paused) {
    v3 fwd_mouse_ray =
      get_mouse_ray( g->g_win_w / 2.0f, g->g_win_h / 2.0f );
    unity* past_at = looking_at_unity;
    pick_looking_at( fwd_mouse_ray );
  }

  // General 'update' step: Update the camera position and
  // the 'game_manager' object, run global scripts, and
  // update the sets of active game objects / lights.
  if ( !paused ) {
    if ( c_man->active_camera ) {
      c_man->active_camera->update_cam_pos();
    }
    g_man->update();
    // Run global scripts if 'level editor' mode is not active.
    if ( !editor ) {
      for ( int i=0; i<g_scripts.size(); i++ ) {
        g_scripts[ i ]->call( elapsed_sec );
      }
    }
    // Update unities.
    u_man->update();
    if ( c_man->active_camera->cam_obj ) {
      c_man->active_camera->cam_obj->update();
    }
    // Update lights.
    l_man->update();
  }

  // Shadow casting: draw the shadow depth buffers.
  l_man->draw_shadow_casters();

  // Clear the OpenGL canvas.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glViewport( 0, 0, g_win_w, g_win_h );

  // Draw the world using the normal shader program.
  s_man->swap_shader( normal_shader_key );
  u_man->draw();
  l_man->draw();

  // Reset the camera translation matrix, for physics debugging.
  glBindBuffer( GL_UNIFORM_BUFFER, c_man->cam_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER,
                    c_man->cam_ubo,
                    c_man->cam_block_buffer );
  glBufferSubData( GL_UNIFORM_BUFFER,
                   0,
                   sizeof(float) * 16,
                   transpose( id4()).m );
  // Perform physics debug drawing if necessary.
  if ( draw_phys_debug ) {
    p_man->draw();
  }

  // Draw the GUI overlay.
  glDepthFunc( GL_ALWAYS );
  g_man->draw();
  glDepthFunc( GL_LESS );

  // Done drawing; ensure that the normal shader program is set.
  s_man->swap_shader( normal_shader_key );
  // Draw the completed OpenGL canvas to the display.
  glfwSwapBuffers( window );

  // Update the FPS counter.
  double cur_seconds, elapsed_seconds;
  cur_seconds = glfwGetTime();
  elapsed_seconds = cur_seconds - prev_seconds;
  // Limit to ~4 updates per second.
  if ( elapsed_seconds > 0.25 ) {
    prev_seconds = cur_seconds;
    double fps = ( double )fps_frame_count / elapsed_seconds;
    snprintf( win_title_buf,
              BRLA_TITLE_BUF_SIZE,
              "Berilia - FPS: %.2f",
              fps );
    glfwSetWindowTitle( window, win_title_buf );
    fps_frame_count = 0;
  }
  fps_frame_count++;

  // Done; return 0 to indicate success.
  return 0;
}

/** Load a game world from a file. TODO: Comment this method. */
void game::reload_file( string fn ) {
  u_man->clear_unities();
  l_man->clear_phong_lights();

  string loaded_file = read_from_file( fn.c_str() );
  json j = json::parse( loaded_file );
  for ( int i = 0; i < (int)j["unities"].size(); ++i ) {
    json u_j = j[ "unities" ][ i ];
    if ( u_j[ "type" ] != "u_light_ind" ) {
      v3 pos = v3( u_j[ "pos_x" ], u_j[ "pos_y" ], u_j[ "pos_z" ] );
      quat rot = quat( rad_to_ang( u_j[ "rot_t" ] ), u_j[ "rot_x" ],
                                   u_j[ "rot_y" ], u_j[ "rot_z" ] );
      unity* loaded_unity = u_man->add_unity( u_j[ "type" ] );
      if ( loaded_unity ) {
        loaded_unity->name = u_j[ "name" ];
        v3 u_sc = v3( u_j[ "scale_x" ],
                      u_j[ "scale_y" ],
                      u_j[ "scale_z" ] );
        loaded_unity->scale( u_sc );
        loaded_unity->translate( pos.v[ 0 ], pos.v[ 1 ], pos.v[ 2 ] );
        loaded_unity->set_rotation( rot );

        for ( int j = 0; j < ( int )u_j[ "scripts" ].size(); ++j ) {
          selected_unity = loaded_unity;
          add_script_to_selected( u_j[ "scripts" ][ j ][ "type" ] );
          script* added = selected_unity->
            scripts[ selected_unity->scripts.size() - 1 ];
          if ( added && added->type == "s_test_use" ) {
            if ( u_j[ "scripts" ][ j ].find( "txt" ) !=
                 u_j[ "scripts" ][ j ].end() ) {
              ( ( s_test_use* )added )->my_text =
                u_j[ "scripts" ][ j ][ "txt" ];
            }
          }
          if ( u_j[ "scripts" ][ j ].find( "on_use" ) !=
               u_j[ "scripts" ][ j ].end() ) {
            selected_unity->use_script = added;
          }
        }
      }
    }
  }

  for ( int i = 0; i < ( int )j[ "phong_lights" ].size(); ++i ) {
    json j_pl = j[ "phong_lights" ][ i ];
    v4 pl_pos = v4( j_pl[ "p_x" ], j_pl[ "p_y" ],
                    j_pl[ "p_z" ], j_pl[ "p_a" ] );
    v4 pl_amb = v4( j_pl[ "a_r" ], j_pl[ "a_g" ],
                    j_pl[ "a_b" ], j_pl[ "a_a" ] );
    v4 pl_dif = v4( j_pl[ "d_r" ], j_pl[ "d_g" ],
                    j_pl[ "d_b" ], j_pl[ "d_a" ] );
    v4 pl_spe = v4( j_pl[ "s_r" ], j_pl[ "s_g" ],
                    j_pl[ "s_b" ], j_pl[ "s_a" ] );
    phong_light* p_l = new phong_light( pl_pos, pl_amb,
                                        pl_dif, pl_spe );
    p_l->specular_exp = j_pl[ "s_e" ];
    p_l->falloff = j_pl[ "f" ];
    p_l->type = j_pl[ "l_type" ];
    p_l->dir = v3( ( float )j_pl[ "sp_dir_x" ],
            ( float )j_pl[ "sp_dir_y" ],
            ( float )j_pl[ "sp_dir_z" ] );
    p_l->spot_rads = ( float )j_pl[ "sp_dir_t" ];
    p_l->indicator = u_man->add_unity( "u_light_ind" );
    p_l->indicator->translate( pl_pos.v[ 0 ],
                               pl_pos.v[ 1 ],
                               pl_pos.v[ 2 ] );
    l_man->add_phong_light( p_l );
  }
}

/**
 * Check whether a key is pressed. Set or un-set its entry in the
 * 'pressed_keys' map, and return true or false.
 */
bool game::check_key_press( int glfw_key, const char c ) {
  check_key_press( glfw_key, c, c );
}

/**
 * Check whether a key is pressed. Set or un-set its entry in the
 * 'pressed_keys' map, and return true or false.
 * The second character will also be cleared from the 'pressed_keys'
 * map if the first one is not pressed, for handling capitalizations.
 */
bool game::check_key_press( int glfw_key,
                            const char c,
                            const char c2 ) {
  const int input_type = GLFW_PRESS;
  const int release_type = GLFW_RELEASE;
  if ( glfwGetKey( window, glfw_key ) == input_type &&
       !pressed_keys[ c ] &&
       !pressed_keys[ c2 ] ) {
    pressed_keys[ c ] = true;
    return true;
  }
  else if ( glfwGetKey( window, glfw_key ) == release_type ) {
    pressed_keys[ c ] = false;
    pressed_keys[ c2 ] = false;
  }
  return false;
}

/**
 * Process text input for a given character by sending a
 * key press to the currently-active GUI if the key is pressed.
 */
void game::check_text_input( int glfw_key, const char c ) {
  check_text_input( glfw_key, c, c );
}

/**
 * Process text input for a given character by sending a
 * key press to the currently-active GUI if the key is pressed.
 * The second character will also be cleared from the 'pressed_keys'
 * map if the first one is not pressed, for handling capitalizations.
 */
void game::check_text_input( int glfw_key,
                             const char c,
                             const char c2 ) {
  const int input_type = GLFW_PRESS;
  const int release_type = GLFW_RELEASE;
  if ( glfwGetKey( window, glfw_key ) == input_type &&
       !pressed_keys[ c ] &&
       !pressed_keys[ c2 ] ) {
    g_man->key_press( c );
    pressed_keys[ c ] = true;
  }
  else if ( glfwGetKey( window, glfw_key ) == release_type ) {
    pressed_keys[ c ] = false;
    pressed_keys[ c2 ] = false;
  }
}

/**
 * Helper method to log any errors encountered when
 * linking a shader program.
 */
void game::log_shader_errors( GLuint shader ) {
  int params = -1;
  glGetProgramiv( shader, GL_LINK_STATUS, &params );
  if ( GL_TRUE != params ) {
    log_error( "(%i) Error in shader linking\n", shader );
    int actual_length = 0;
    char log_buf[ GL_SHADER_LOG_LEN ];
    glGetProgramInfoLog( shader,
                         GL_SHADER_LOG_LEN,
                         &actual_length,
                         log_buf );
    log_buf[ GL_SHADER_LOG_LEN - 1 ] = '\0';
    log( "Shader log:\n-=-=-=-=\n%s\n-=-=-=-=-=\n", log_buf );
    if ( actual_length >= GL_SHADER_LOG_LEN ) {
      log( "[WARN ] shader log exceeds buffer size. %d vs %d\n",
           actual_length,
           GL_SHADER_LOG_LEN - 1 );
    }
  }
}

/**
 * Write up-to-date values to the 'game world' Uniform Buffer Object.
 */
void game::write_world_ubo() {
  glBindBuffer( GL_UNIFORM_BUFFER, world_block_buffer );
  glBindBufferBase( GL_UNIFORM_BUFFER,
                    world_ubo,
                    world_block_buffer );
  world_ubo_index =
    glGetUniformBlockIndex( s_man->cur_shader, "world_ubo" );
  if ( ( ( int )world_ubo_index ) >= 0 ) {
    glUniformBlockBinding( s_man->cur_shader,
                           world_ubo_index,
                           world_ubo );
    glBufferSubData( GL_UNIFORM_BUFFER,
                     0,
                     sizeof( float ) * BRLA_GAME_UBO_SIZE,
                     world_ubo_buf );
  }
}

/**
 * Get a ray pointing from the camera along an axis
 * defined by an X / Y mouse click location on the window.
 * TODO: Comment this method.
 */
v3 game::get_mouse_ray( int pix_x, int pix_y ) {
  float m_x = ( 2.0 * pix_x ) / g_win_w - 1.0;
  float m_y = 1.0 - ( 2.0 * pix_y ) / g_win_h;
  float m_z = 1.0;
  v3 m_ray = v3( m_x, m_y, m_z );
  v4 m_ray_clip = v4( m_ray.v[ 0 ], m_ray.v[ 1 ], -1.0, 1.0 );
  v4 m_ray_eye =
          inverse( c_man->active_camera->persp_matrix ) * m_ray_clip;
  m_ray_eye = v4( m_ray_eye.v[ 0 ],
                        m_ray_eye.v[ 1 ],
                        -1.0,
                        0.0 );
  v3 m_ray_world = v3( inverse(
          c_man->active_camera->c_view_matrix ) * m_ray_eye );
  m_ray_world = normalize( m_ray_world );
  return m_ray_world;
}

/**
 * Create a script and add it to the selected game object, if able.
 * TODO: Move this to the 'script.h' / 'script.cpp' files.
 */
void game::add_script_to_selected( string type ) {
  // Return early if the script type is empty.
  if ( type == "" ) { return; }

  // Add the script to the global 'game' object if no
  // 'unity' game object is selected.
  vector<script*>* sel_scrs = NULL;
  if ( selected_unity == NULL ) { sel_scrs = &g_scripts; }
  else { sel_scrs = &( selected_unity->scripts ); }

  // Figure out which type of script to create.
  if ( type == "s_test_use" ) {
    sel_scrs->push_back( new s_test_use( selected_unity ) );
  }

  // Update the GUI to reflect the new script.
  // (If the game is in 'level editor' mode)
  g_man->update_selected();
}

/**
 * Helper method to select a game object using a raycast
 * from the active camera's location.
 */
void game::pick_looking_at( v3 dir ) {
  v3 c_pos = c_man->active_camera->cam_pos * -1;
  pick_looking_at( c_pos, dir );
}

/**
 * Select a game object using a raycast from a given position,
 * in a given direction. TODO: Comment this method.
 */
void game::pick_looking_at( v3 pos, v3 dir ) {
  btVector3 c_pos = btVector3( pos.v[ 0 ], pos.v[ 1 ], pos.v[ 2 ] );
  btVector3 c_dir = btVector3( dir.v[ 0 ],
                               dir.v[ 1 ],
                               dir.v[ 2 ] ) * far;
  btCollisionWorld::ClosestRayResultCallback ray_callback( c_pos,
                                                           c_dir );
  p_man->phys_world->rayTest( c_pos, c_dir, ray_callback );
  if ( ray_callback.hasHit() ) {
    btRigidBody* r_body = ( btRigidBody* )
      btRigidBody::upcast( ray_callback.m_collisionObject );
    if ( !r_body ) {
      looking_at_unity = 0;
      return;
    }
    last_picked_pos = ray_callback.m_hitPointWorld;
    // Limit the maximum distance to pick an object from.
    // TODO: Configurable setting for this?
    if ( ( last_picked_pos - c_pos ).length() > 4 ) {
      return;
    }
    looking_at_unity = u_man->get( r_body );
  }
  else {
    looking_at_unity = 0;
    return;
  }
  last_picked_pos = c_pos + c_dir;
}

/**
 * Helper method to pick a game object or lighting object using
 * a raycast from the active camera, given a direction.
 * TODO: Comment this method. Also, this has some differences,
 * but it is very similar to 'pick_looking_at'.
 */
void game::pick_selection( v3 dir ) {
  v3 pos = c_man->active_camera->cam_pos * -1;
  btVector3 c_pos = btVector3( pos.v[ 0 ], pos.v[ 1 ], pos.v[ 2 ] );
  btVector3 c_dir = btVector3( dir.v[ 0 ],
                               dir.v[ 1 ],
                               dir.v[ 2 ] ) * far;
  btCollisionWorld::ClosestRayResultCallback ray_callback( c_pos,
                                                           c_dir );
  p_man->phys_world->rayTest( c_pos, c_dir, ray_callback );
  if ( ray_callback.hasHit() ) {
    btRigidBody* r_body = ( btRigidBody* )
      btRigidBody::upcast( ray_callback.m_collisionObject );
    if ( !r_body ) { return; }
    last_picked_pos = ray_callback.m_hitPointWorld;
    unity* sel = u_man->get( r_body );
    phong_light* l = l_man->get_by_indicator( sel );
    if ( !l ) { selected_unity = sel; }
    else { selected_light = l; }
    g_man->update_selected();
  }
  last_picked_pos = c_pos + c_dir;
}

/** GLFW callback: log an error. */
void glfw_log_err( int err, const char* description ) {
  log_error( "GLFW Error (%i): %s\n", err, description );
}

/** GLFW callback: resize the main window. */
void glfw_win_resize( GLFWwindow* window, int w, int h ) {
  g->g_win_w = w;
  g->g_win_h = h;
  g->aspect_ratio = ( float )w / ( float )h;
  
  // Update the viewport.
  glViewport( 0, 0, g->g_win_w, g->g_win_h );

  // We should update the projection matrix next game loop step.
  g->update_proj_matrix = true;
  g->g_man->window_resize_update();
}

/** GLFW callback: mouse position update. */
void glfw_mouse_pos( GLFWwindow* window, double m_x, double m_y ) {
  g->mouse_dx = g->mouse_x - m_x;
  g->mouse_dy = g->mouse_y - m_y;
  g->mouse_x = m_x;
  g->mouse_y = m_y;
  if ( g->c_man->active_camera &&
       g->c_man->active_camera->cam_rot_type == B_CAM_ROT_XY ) {
    // We're in first-person camera mode, so rotate the view accordingly.
    float x_theta = ( g->mouse_dx / g->g_win_w ) * -50.0f;
    float y_theta = ( g->mouse_dy / g->g_win_h ) * -50.0f;
    camera* cam = g->c_man->active_camera;
    quat cur_quat = cam->cam_quat;

    v3 up_axis = normalize( v3( cam->cam_rot.row( 1 ) ) );
    v3 right_axis = normalize( v3( cam->cam_rot.row( 0 ) ) );
    quat dx_quat = quat( x_theta,
                         up_axis.v[ 0 ],
                         up_axis.v[ 1 ],
                         up_axis.v[ 2 ] );
    quat dy_quat = quat( y_theta,
                         right_axis.v[ 0 ],
                         right_axis.v[ 1 ],
                         right_axis.v[ 2 ] );
    quat total_quat = dx_quat * dy_quat;
    cur_quat = cur_quat * total_quat;
    cam->rotate( cur_quat );
  }
}

/** GLFW callback: mouse button update. */
void glfw_mouse_button( GLFWwindow* window,
                        int button,
                        int action,
                        int mods ) {
  int m_x = ( int )g->mouse_x;
  int m_y = ( int )g->mouse_y;
  v3 mouse_ray = g->get_mouse_ray( m_x, m_y );
  if ( g->c_man->active_camera &&
       g->c_man->active_camera->cam_rot_type == B_CAM_ROT_XY ) {
    mouse_ray = g->get_mouse_ray( 0, 0 );
  }

  if ( action == GLFW_PRESS ) {
    if ( button == GLFW_MOUSE_BUTTON_LEFT && !g->left_mouse_down ) {
      g->mouse_ray = mouse_ray;
      if ( g->editor || g->paused ) {
        if ( g->g_man ) {
          if ( !g->g_man->mouse_click_update( m_x, m_y ) ) {
            // Perform ray cast.
            if ( g->g_man->cur_gui ) {
              g->g_man->cur_gui->selected = 0;
            }
            g->pick_selection( mouse_ray );
            //g->left_mouse_down = true;
          }
        }
        g->left_mouse_down = true;
      }
      /*
       * This 'shoot' functionality is no longer used, but
       * I'm keeping it commented-out to have a record of how
       * to use Bullet's "CCD" feature to detect collisions when
       * objects move quickly enough to clip through each other
       * in a single step of the physics simulation.
      else {
        if ( g->fire_delay >= 0.1f ) {
          // Fire some plasma bolt unities.
          v4 c_fwd =
            g->c_man->active_camera->cam_rot.row( 2 ) * -1.0f;
          unity* shot = g->u_man->add_unity( "u_test_plasma_bolt" );
          if ( shot ) {
            // CCD motion clamping; threshold is the minimum
            // distance to activate CCD,
            // and the sphere radius is the base sphere size.
            btRigidBody* s_body = shot->p_obj->rigid_body;
            s_body->setCcdMotionThreshold( 0.1f );
            s_body->setCcdSweptSphereRadius( 0.001f );
            // Set the callback for catching these collisions,
            // since normal manifolds
            // don't seem to be generated with CCD.
            v3 s_pos = ( g->c_man->active_camera->cam_pos * -1.0f ) +
                       ( c_fwd * 0.5f );
            shot->translate( s_pos.v[ 0 ],
                             s_pos.v[ 1 ],
                             s_pos.v[ 2 ] );
            quat cq = g->c_man->active_camera->cam_quat;
            quat cq_n = quat( rad_to_ang( cq.r[ 0 ] ),
                              cq.r[ 1 ] * -1.0f,
                              cq.r[ 2 ] * -1.0f,
                              cq.r[ 3 ] * -1.0f );
            shot->set_rotation( cq_n );
            shot->scale( v3( 1.0f, 1.0f, 1.0f ) );
            float s_mag = 30.0f;
            btVector3 shot_vel = btVector3( c_fwd.v[ 0 ] * s_mag,
                                            c_fwd.v[ 1 ] * s_mag,
                                            c_fwd.v[ 2 ] * s_mag );
            shot->p_obj->rigid_body->setLinearVelocity( shot_vel );
            v3 cc = shot->cur_center;
          };
          g->fire_delay = 0.0f;
        }
        g->left_mouse_down = true;
      }
      */
    }
    else if ( button == GLFW_MOUSE_BUTTON_RIGHT &&
              !g->right_mouse_down ) {
      g->r_mouse_ray = mouse_ray;
      g->right_mouse_down = true;
    }
    else if ( button == GLFW_MOUSE_BUTTON_MIDDLE &&
              !g->middle_mouse_down ) {
      g->m_mouse_ray = mouse_ray;
      g->middle_mouse_down = true;
    }
  }
  else if ( action == GLFW_RELEASE ) {
    if ( button == GLFW_MOUSE_BUTTON_LEFT ) {
      g->left_mouse_down = false;
    }
    else if ( button == GLFW_MOUSE_BUTTON_RIGHT ) {
      g->right_mouse_down = false;
    }
    else if ( button == GLFW_MOUSE_BUTTON_MIDDLE ) {
      g->middle_mouse_down = false;
    }
  }
}
