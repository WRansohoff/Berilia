#include "util.h"

/**
 * Get the length of a file, in bytes.
 * Returns 0 if the file does not exist.
 */
unsigned long get_file_length( ifstream& file ) {
  // Return 0 if the file object is invalid.
  if ( !file.good() ) { return 0; }

  // Get the length of the file.
  // TODO: Apparently this is not a great way to accurately
  // read file lengths on all systems; maybe I should use
  // a different method?
  unsigned long len = file.tellg();
  file.seekg( 0, std::ios::end );
  len = file.tellg();
  file.seekg( std::ios::beg );

  // Return the calculated length.
  return len;
}

/**
 * Copy an array of floats from one array into another, at
 * an offset from the destination pointer.
 * TODO: Make this a macro or inline function?
 */
void fill_float_buffer( float* dest, float* dat,
                        int start_index, int len ) {
  memcpy( &dest[ start_index ], dat, len * sizeof( float ) );
}

/**
 * Load a mesh from a file into a 'mesh' data structure.
 * The returned pointer, if not null, must be freed by the caller.
 * In practice, game objects ('unities') will usually
 * take ownerhsip of these pointers.
 */
mesh* load_mesh( const char* filename ) {
  // Open the file using the AssImp library.
  const aiScene* scene = aiImportFile( filename,
                                       aiProcess_Triangulate );
  // Log an error and return null if the mesh could not be loaded.
  if ( !scene ) {
    log_error( "Error loading mesh file: %s\n", filename );
    return 0;
  }

  // Allocate arrays to store the mesh's vertex positions, normals,
  // and texture coordinates. These arrays are de-allocated by the
  // mesh object in its destructor.
  aiNode* root_node = scene->mRootNode;
  int num_verts = count_vertices( root_node, scene, 0 );
  GLfloat* points = new GLfloat     [ num_verts * 3 ];
  GLfloat* normals = new GLfloat    [ num_verts * 3 ];
  GLfloat* tex_coords = new GLfloat [ num_verts * 2 ];
  // Traverse and load the mesh's tree structure recursively.
  import_node( root_node, scene, points, normals, tex_coords, 0 );

  // One more pass over the vertices to generate the AABB.
  // (Axis-Aligned Bounding Box, used for physics and first-pass
  //  approximations to reduce the amount of geometry processing.)
  // TODO: I should probably just make this part of the import above,
  // but I don't feel like passing around half a dozen more args or
  // making a new class/struct right now.
  float min_x = 0.0f;
  float max_x = 0.0f;
  float min_y = 0.0f;
  float max_y = 0.0f;
  float min_z = 0.0f;
  float max_z = 0.0f;
  for ( int i = 0; i < num_verts * 3; i += 3 ) {
    float px = points[ i ];
    float py = points[ i + 1 ];
    float pz = points[ i + 2 ];
    if ( px < min_x ) { min_x = px; }
    if ( px > max_x ) { max_x = px; }
    if ( py < min_y ) { min_y = py; }
    if ( py > max_y ) { max_y = py; }
    if ( pz < min_z ) { min_z = pz; }
    if ( pz > max_z ) { max_z = pz; }
  }
  float mag_x = max_x - min_x;
  float mag_y = max_y - min_y;
  float mag_z = max_z - min_z;

  // Create the actual AABB and Mesh objects.
  aabb bounding_box = aabb( mag_x, mag_y, mag_z );
  mesh* m = new mesh( num_verts, points, normals,
                      tex_coords, bounding_box, id4() );

  // Done; close the AssImp file representation and return the Mesh.
  aiReleaseImport( scene );
  return m;
}

/**
 * Load a mesh file using the AssImp library, then export only the
 * information used by this engine to a JSON-formatted file.
 */
void export_mesh_json( const char* mesh_fn, const char* json_fn ) {
  // Open the file using the AssImp library.
  // TODO: Move this shared mesh-import logic into a separate method.
  const aiScene* scene = aiImportFile( mesh_fn,
                                       aiProcess_Triangulate );
  // Log an error and return null if the mesh could not be loaded.
  if ( !scene ) {
    log_error( "Couldn't load mesh file: %s\n", mesh_fn );
    return;
  }

  // Allocate arrays to store the mesh's vertex positions, normals,
  // and texture coordinates. These arrays are de-allocated by the
  // mesh object in its destructor.
  aiNode* root_node = scene->mRootNode;
  int num_verts = count_vertices( root_node, scene, 0 );
  GLfloat* points = new GLfloat     [ num_verts * 3 ];
  GLfloat* normals = new GLfloat    [ num_verts * 3 ];
  GLfloat* tex_coords = new GLfloat [ num_verts * 2 ];
  // Traverse and load the mesh's tree structure.
  import_node( root_node, scene, points, normals, tex_coords, 0 );

  // Export the core mesh data to JSON.
  // Open the target JSON file.
  FILE* file = fopen( json_fn, "w" );
  // Log an error and return if the target file couldn't be opened.
  if ( !file ) {
    log_error( "Couldn't open file for writing: %s\n", json_fn );
    return;
  }

  // Write mesh data to the file in JSON format.
  // TODO: Use an actual serializer for this? Maybe Rust/Serde? :)
  fprintf( file, "{\"num_verts\":%i,\n\"points\":[", num_verts );
  for ( int i = 0; i < num_verts * 3; ++i ) {
    if ( i != ( num_verts * 3 ) - 1 ) {
      fprintf( file, "%.5f,", points[ i ] );
    }
    else {
      fprintf( file, "%.5f],\n\"normals\":[", points[ i ] );
    }
  }
  for ( int i = 0; i < num_verts * 3; ++i ) {
    if ( i != ( num_verts * 3 ) - 1 ) {
      fprintf( file, "%.5f,", normals[ i ] );
    }
    else {
      fprintf( file, "%.5f],\n\"tex_coords\":[", normals[ i ] );
    }
  }
  for ( int i = 0; i < num_verts * 2; ++i ) {
    if ( i != ( num_verts * 2 ) - 1 ) {
      fprintf( file, "%.5f,", tex_coords[ i ] );
    }
    else {
      fprintf( file, "%.5f]\n}", tex_coords[ i ] );
    }
  }

  // Done; close the JSON file.
  fclose( file );
}

/**
 * Helper method to convert a string into a floating-point number
 * with basic error-handling. Values returned can include 'nanf'.
 */
float str_to_f( string s ) {
  // Use 'stof', and return 'not-a-number' if a common
  // error is encountered.
  float val;
  try {
    val = stof( s );
  }
  catch( std::invalid_argument& e ) {
    printf( "Not a float: %s\n", e.what() );
    val = nanf( "" );
  }
  catch( std::out_of_range& e ) {
    printf( "Out of range: %s\n", e.what() );
    val = nanf( "" );
  }
  return val;
}

/**
 * Convert an OpenGL error enumeration value into a
 * human-readable string.
 */
string framebuffer_error_to_string( GLenum err ) {
  string ret = "Unrecognized OpenGL error, sorry.";
  if ( err == GL_FRAMEBUFFER_COMPLETE ) {
    ret = "[GL_FRAMEBUFFER_COMPLETE] "
          "Framebuffer is fine; that's not an error.";
  }
  else if ( err == GL_FRAMEBUFFER_UNDEFINED ) {
    ret = "[GL_FRAMEBUFFER_UNDEFINED] "
          "Framebuffer doesn't exist.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT] "
          "One of the framebuffer's attachment points is incomplete.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT] "
          "No images attached to the framebuffer.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER] "
          "I'm not totally sure what this means. The draw buffer "
          "color attachment point isn't valid? Google it, sorry.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER] "
          "I'm not totally sure what this means. The read buffer "
          "color attachment point isn't valid? Google it, sorry.";
  }
  else if ( err == GL_FRAMEBUFFER_UNSUPPORTED ) {
    ret = "[GL_FRAMEBUFFER_UNSUPPORTED] "
          "The set of image formats attached to this framebuffer "
          "is invalid for some reason.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE] "
          "I think this has to do with renderbuffers "
          "(which I don't use), so hopefully this doesn't come up.";
  }
  else if ( err == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS ) {
    ret = "[GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS] "
          "A framebuffer attachment is layered, but its attachee "
          "is not layered. Or, all populated color attachments "
          "are not from 'textures of the same target'.";
  }
  return ret;
}

/**
 * AssImp mesh import helper method: recursively count the
 * number of vertices in a given AssImp mesh object.
 */
int count_vertices( const aiNode* cur_node,
                    const aiScene* scene, int c ) {
  // Start with the current vertex count.
  int new_count = c;

  // Add the number of vertices in this mesh object.
  const int num_meshes = cur_node->mNumMeshes;
  for ( int i = 0; i < num_meshes; ++i ) {
    new_count +=
      scene->mMeshes[ cur_node->mMeshes[ i ] ]->mNumVertices;
  }

  // Add the number of vertices in any child mesh objects.
  const int num_children = cur_node->mNumChildren;
  for ( int i = 0; i < num_children; ++i ) {
    new_count =
      count_vertices( cur_node->mChildren[ i ], scene, new_count );
  }

  // Return the new vertex count.
  return new_count;
}

/**
 * AssImp mesh import helper method: recursively import vertex
 * position, normal, and texture information for a given 'node'
 * and its children in AssImp's mesh tree import structure.
 */
void import_node( const aiNode* cur_node, const aiScene* scene,
                  GLfloat* p, GLfloat* n, GLfloat* t, int index ) {
  // Track the current index in position/normal/texture arrays.
  int new_index = index;
  // Import the mesh data from each AssImp 'mesh' in the current node.
  const int num_meshes = cur_node->mNumMeshes;
  for ( int i = 0; i < num_meshes; ++i ) {
     // Import the actual mesh data.
    aiMesh* m = scene->mMeshes[ cur_node->mMeshes[ i ] ];
    int mesh_vertices = m->mNumVertices;
    // Mesh positions and normals are relative to
    // the node's current transform, so store that in a 4x4 matrix.
    aiMatrix4x4 tr = cur_node->mTransformation;
    m4 mesh_transform = m4( tr.a1, tr.a2, tr.a3, tr.a4,
                            tr.b1, tr.b2, tr.b3, tr.b4,
                            tr.c1, tr.c2, tr.c3, tr.c4,
                            tr.d1, tr.d2, tr.d3, tr.d4 );

    // Store each vertex's position / normal / texture coordinates
    // in the corresponding GLFloat arrays.
    for ( int j = 0; j < mesh_vertices; ++j ) {
      int b_i = new_index + ( j * 3 );
      int t_i = new_index + ( j * 2 );

      // Position values.
      if ( m->HasPositions() ) {
        const aiVector3D* vec_pos = &( m->mVertices[ j ] );
        v4 v_pos_raw = v4( vec_pos->x, vec_pos->y, vec_pos->z, 0.0 );
        v3 v_pos = v3( ( mesh_transform * v_pos_raw ) );
        p[ b_i ]   = v_pos.v[ 0 ];
        p[ b_i+1 ] = v_pos.v[ 1 ];
        p[ b_i+2 ] = v_pos.v[ 2 ];
      }
      else {
        log( "[WARN ] Loaded mesh %s without vertices.\n",
             m->mName.C_Str() );
        p[b_i]   = 0.0f;
        p[b_i+1] = 0.0f;
        p[b_i+2] = 0.0f;
      }

      // Surface normal values. A surface normal is basically
      // a ray which is an orthogonal tangent to the surface.
      // Think of it as an arrow pointing 'away from' the mesh at
      // the given position. These are used to calculate things like
      // lighting reflections and intensity.
      if ( m->HasNormals() ) {
        const aiVector3D* vec_norm = &( m->mNormals[ j ] );
        v4 v_norm_raw = v4( vec_norm->x, vec_norm->y,
                            vec_norm->z, 1.0  );
        v3 v_norm = v3( ( mesh_transform * v_norm_raw ) );
        v_norm = normalize( v_norm );
        n[ b_i ]   = v_norm.v[ 0 ];
        n[ b_i+1 ] = v_norm.v[ 1 ];
        n[ b_i+2 ] = v_norm.v[ 2 ];
      }
      else {
        log( "[WARN ] Loaded mesh %s without surface normals.\n",
             m->mName.C_Str() );
        n[ b_i ]   = 0.0f;
        n[ b_i+1 ] = 0.0f;
        n[ b_i+2 ] = 0.0f;
      }

      // Assume 1 set of texture coordinates for mesh.
      if ( m->HasTextureCoords( 0 ) ) {
        const aiVector3D* vec_tex = &( m->mTextureCoords[ 0 ][ j ] );
        t[ t_i ]   = vec_tex->x;
        t[ t_i+1 ] = vec_tex->y;
      }
      else {
        log( "[WARN ] Loaded mesh %s without texture coordinates.\n",
             m->mName.C_Str() );
        t[ t_i ]   = 0.0f;
        t[ t_i+1 ] = 0.0f;
      }
    }

    // Increment the vertex index.
    new_index += mesh_vertices;
  }

  // Done with this node, now process any child nodes in the mesh.
  const int num_children = cur_node->mNumChildren;
  for ( int i = 0; i < num_children; ++i ) {
    import_node( cur_node->mChildren[ i ],
                 scene, p, n, t, new_index );
  }
}

/**
 * Log OpenGL errors, if any are pending.
 * This wil normally be called at least once per game loop cycle.
 */
void log_gl_errors() {
  // Log errors until none are left pending.
  GLenum err = GL_NO_ERROR;
  while ( ( err = glGetError() ) != GL_NO_ERROR ) {
    // Either log to the log file or, to avoid clutter, to stdout.
    // TODO: Make a configuration setting for this?
    //log_error( "[ERROR] OpenGL reports error code %i\n", err );
    printf( "[ERROR] OpenGL reports error code %i\n", err );
  }
}

/**
 * Check for and log framebuffer errors.
 */
void log_fb_errors() {
  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
  if ( GL_FRAMEBUFFER_COMPLETE != status ) {
    // Either log to the log file or, to avoid clutter, to stdout.
    // TODO: Make a configuration setting for this?
    log_error( "[ERROR] Incomplete framebuffer. Status code: %i\n",
               status );
    log_error( "  (%s)\n",
               framebuffer_error_to_string(status).c_str() );
    /*
    printf( "[ERROR] Incomplete framebuffer. Status code: %i\n",
            status );
    printf( "  (%s)\n",
            framebuffer_error_to_string(status).c_str() );
    */
  }
}

/**
 * Restart the log file for a new application run.
 */
int restart_log() {
  // Open the log file, and log an error if it couldn't be opened.
  FILE* file = fopen( BRLA_LOG_FILE, BRLA_LOG_TYPE );
  if ( !file ) {
    fprintf( stderr,
             "[ERROR] couldn't open log file for writing: %s\n",
             BRLA_LOG_FILE );
    return 1;
  }

  // Log a line break and the current time.
  time_t now = time( 0 );
  char* datetime = ctime( &now );
  fprintf( file,
           "=-=-=-=-=-=-=-=-=-=-\nInit log. Local time %s\n",
           datetime );

  // Close the file again.
  fclose( file );
  return 0;
}

/**
 * Log a message to the log file.
 */
int log( const char* msg, ... ) {
  // Open the log file for appending, and print an error on failure.
  FILE* file = fopen( BRLA_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr,
             "Error: couldn't open log file for writing: %s\n",
             BRLA_LOG_FILE );
    return 1;
  }

  // Normal logging only prints to the log file.
  va_list argptr;
  va_start( argptr, msg );
  vfprintf( file, msg, argptr );
  va_end( argptr );

  // Close the log file.
  fclose( file );
  return 0;
}

/**
 * Log an error to the log file and stderr.
 */
int log_error( const char* msg, ... ) {
  // Open the log file for appending, and print an error on failure.
  FILE* file = fopen( BRLA_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr,
             "Error: couldn't open log file for writing: %s\n",
             BRLA_LOG_FILE );
    return 1;
  }

  // Errors print twice; once to the log, and once to stderr.
  va_list argptr;
  va_start( argptr, msg );
  vfprintf( file, msg, argptr );
  va_end( argptr );
  va_start( argptr, msg );
  vfprintf( stderr, msg, argptr );
  va_end( argptr );

  // Close the log file.
  fclose( file );
  return 0;
}

/**
 * Log information about the curent machine's OpenGL capabilities.
 */
void log_gl_info() {
  // Make a list of integer and floating-point parameters to check.
  int num_int_params = 14;
  int num_float_params = 1;
  int num_norm_params = num_int_params + num_float_params;
  GLenum params[] = {
    GL_MAX_UNIFORM_BUFFER_BINDINGS,
    GL_MAX_UNIFORM_BLOCK_SIZE,
    GL_MAX_VERTEX_UNIFORM_BLOCKS,
    GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE,
    GL_MAX_DRAW_BUFFERS,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    GL_MAX_TEXTURE_IMAGE_UNITS,
    GL_MAX_TEXTURE_SIZE,
    GL_MAX_VARYING_FLOATS,
    GL_MAX_VERTEX_ATTRIBS,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS,
    GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
    GL_MAX_VIEWPORT_DIMS,
    GL_STEREO
  };
  const char* names[] = {
    "GL_MAX_UNIFORM_BUFFER_BINDINGS",
    "GL_MAX_UNIFORM_BLOCK_SIZE",
    "GL_MAX_VERTEX_UNIFORM_BLOCKS",
    "GL_MAX_FRAGMENT_UNIFORM_BLOCKS",
    "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
    "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
    "GL_MAX_DRAW_BUFFERS",
    "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
    "GL_MAX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_TEXTURE_SIZE",
    "GL_MAX_VARYING_FLOATS",
    "GL_MAX_VERTEX_ATTRIBS",
    "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
    "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT",
    "GL_MAX_VIEWPORT_DIMS",
    "GL_STEREO"
  };

  // Check and log each parameter of interest.
  log( "GL Context Info:\n" );
  log( "=-=-=-=-=-=-=-=-=-=-\n" );
  for ( int i = 0; i < num_int_params; i++ ) {
    int v = 0;
    glGetIntegerv( params[ i ], &v );
    log( "%s: %i\n", names[ i ], v );
  }
  for ( int i = 0; i < num_float_params; i++ ) {
    float v = 0;
    glGetFloatv( params[ num_int_params + i ], &v );
    log( "%s: %.2f\n", names[ num_int_params + i ], v );
  }
  int v[ 2 ];
  v[ 0 ] = v[ 1 ] = 0;
  glGetIntegerv( params[ num_norm_params ], v );
  log( "%s %i %i\n", names[ num_norm_params ], v[ 0 ], v[ 1 ] );
  // (Also check one boolean parameter, GL_STEREO)
  unsigned char s = 0;
  glGetBooleanv( params[ num_norm_params+1 ], &s );
  log( "%s: %u\n", names[ num_norm_params+1 ], ( unsigned int )s );
  log( "=-=-=-=-=-=-=-=-=-=-\n" );
}

/**
 * Write a message to a file.
 */
void write_to_file( const char* message, const char* filename ) {
  // Open the file and print an error if it fails.
  FILE* file = fopen( filename, "w" );
  if ( !file ) {
    fprintf( stderr,
             "Error: couldn't open log file for writing: %s\n",
             filename );
    return;
  }
  // Print the message.
  fputs( message, file );
  // Close the file.
  fclose( file );
}

/**
 * Read the full contents of a file into a string.
 * You probably shouldn't use this with a very large file.
 */
string read_from_file( const char* filename ) {
  ifstream file_stream( filename );
  string file_contents(
    ( std::istreambuf_iterator<char>( file_stream ) ),
    ( std::istreambuf_iterator<char>() )
  );
  return file_contents;
}
