#include "shaders.h"

/**
 * Shader manager constructor. Currently empty,
 * the class doesn't need to initialize anything internally.
 */
shader_manager::shader_manager() {}

/**
 * Shader manager destructor. Deletes any shader programs in
 * the hashmap of available ones.
 */
shader_manager::~shader_manager() {
  // Set the shader program to zero. Shader execution should
  // not be attempted in this state, but we _are_ about to
  // delete all of the currently-compiled shader programs.
  glUseProgram( 0 );
  // Delete all of the currently-compiled shader programs. :)
  for ( auto sh_iter = shader_map.begin();
        sh_iter != shader_map.end();
        ++sh_iter ) {
    GLuint shader_prog = sh_iter->second;
    if ( shader_prog ) { glDeleteProgram( shader_prog ); }
  }
}

/**
 * Helper method to load a shader by filename, and compile it.
 * This method is protected, because it should only be called
 * by the shader manager as part of the process of compiling
 * a full shader program.
 */
void shader_manager::load_shader( const char* s_fn, GLuint s ) {
  // TODO: Use the 'util.h' 'read_from_file' method?
  // Open the shader file for reading with an input stream.
  std::ifstream file;
  file.open( s_fn, std::ios::in );
  if ( !file ) { printf( "Couldn't open file\n" ); return; }
  // Read the contents of the file.
  unsigned long len = get_file_length( file );
  char* contents = new char[ len + 1 ];
  const char* c_contents;
  file.read( contents, len );
  // Better safe than sorry; null-terminate the C-string.
  contents[ len ] = '\0';
  c_contents = contents;

  // Try to compile the shader.
  glShaderSource( s, 1, &c_contents, NULL );
  glCompileShader( s );

  // Log shader errors if compilation failed.
  // TODO: Make a separate method for this or use 'log_shader_errors'?
  int params = -1;
  glGetShaderiv( s, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    log_error( "ERROR: GLSL shader '%s'(%i) did not compile\n",
               s_fn, s );
    int actual_length = 0;
    char glsl_log[ GL_SHADER_LOG_LEN ];
    glGetShaderInfoLog( s, GL_SHADER_LOG_LEN,
                        &actual_length, glsl_log );
    glsl_log[ GL_SHADER_LOG_LEN - 1 ] = '\0';
    log( "Info:\n---\n%s\n---\n", glsl_log );
    if ( actual_length >= GL_SHADER_LOG_LEN ) {
      log( "[WARN ] (GLSL dump exceeded size of buffer: %d >= %d.)\n",
           actual_length, GL_SHADER_LOG_LEN );
    }
  }
}

/**
 * Compile a shader program consisting of a vertex shader and a
 * fragment shader, then store it in the shader manager's hash map
 * of available shader programs with the given string key.
 */
void shader_manager::add_shader_prog( string key,
                                      string vert_fn,
                                      string frag_fn ) {
  // Shaders are small enough and few enough that I'm not
  // going to worry about re-using individual v/f shaders.
  // Compile the vertex and fragment shaders.
  GLuint vert_shader = glCreateShader( GL_VERTEX_SHADER );
  load_shader( vert_fn.c_str(), vert_shader );
  GLuint frag_shader = glCreateShader( GL_FRAGMENT_SHADER );
  load_shader( frag_fn.c_str(), frag_shader );

  // Create a new shader program, attach the vertex / fragment
  // shaders, and link them. TODO: Error checking with cleanup
  // and early returns?
  GLuint shader_prog = glCreateProgram();
  glAttachShader( shader_prog, vert_shader );
  glAttachShader( shader_prog, frag_shader );
  glLinkProgram( shader_prog );

  // Log shader errors, if any.
  g->log_shader_errors( shader_prog );

  // Unlink and delete the vertex/fragment shaders.
  // (This does not delete the shader program itself)
  glDetachShader( shader_prog, vert_shader );
  glDetachShader( shader_prog, frag_shader );
  glDeleteShader( vert_shader );
  glDeleteShader( frag_shader );

  // Map the shader program in the shader manager.
  evict_mapping( key );
  shader_map[ key ] = shader_prog;
}

/**
 * Evict a shader program from the hash map of available shaders,
 * and delete the OpenGL shader program reference.
 */
void shader_manager::evict_mapping( string key ) {
  // Find the shader program associated with the given key, if any.
  auto sh_iter = shader_map.find( key );
  // If the shader program exists, delete it.
  if ( sh_iter != shader_map.end() ) {
    GLuint shader_prog = sh_iter->second;

    // If the shader is currently active,
    // deactivate it before deletion.
    if ( cur_shader == sh_iter->second ) {
      cur_shader = 0;
      glUseProgram( 0 );
    }

    // Delete the shader program and remove it from the hash map.
    glDeleteProgram( shader_prog );
    shader_map.erase( key );
  }
}

/**
 * Retrieve a shader program from the hash map of available shaders.
 * Returns the OpenGL shader program ID if one is found,
 * or 0 if no entry with the given string key exists.
 */
GLuint shader_manager::get( string key ) {
  // Look for an entry with the given key, and return either it or 0.
  auto sh_iter = shader_map.find( key );
  if ( sh_iter == shader_map.end() ) { return 0; }
  return sh_iter->second;
}

/**
 * Helper method to switch active shaders, given the OpenGL ID
 * of the new shader.
 */
void shader_manager::swap_shader( GLuint shader ) {
  // Update the shader manager's record of the current shader program.
  cur_shader = shader;
  // Tell OpenGL to use the new shader program.
  glUseProgram( shader );

  // Update Uniform Buffer Objects in the new shader program.
  g->write_world_ubo();
  // Update shader values associated with the camera and lighting,
  // but only if an active camera exists.
  if (g->c_man->active_camera) {
    g->c_man->update_cam_ubo();
    g->l_man->update();
  }
}

/**
 * Helper method to switch active shaders, given the string key
 * of the new shader.
 */
void shader_manager::swap_shader( string key ) {
  swap_shader( get( key ) );
}
