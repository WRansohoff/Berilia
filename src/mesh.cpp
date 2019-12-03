#include "mesh.h"

/**
 * Constructor: populate the main mesh object attributes,
 * and initialize its OpenGL buffers.
 */
mesh::mesh(int num_verts, GLfloat* p, GLfloat* n, GLfloat* t,
           aabb baa, m4 t_m) {
  num_vertices = num_verts;
  points = p;
  normals = n;
  tex_coords = t;
  bounding_box = baa;
  transform = t_m;

  init_buffers();
}

/**
 * Destructor: delete OpenGL buffers and the underlying
 * position / normal / texture coordinate arrays.
 */
mesh::~mesh() {
  if ( points_vbo ) { glDeleteBuffers( 1, &points_vbo ); }
  if ( points ) { delete [] points; }
  if ( normals_vbo ) { glDeleteBuffers( 1, &normals_vbo ); }
  if ( normals ) { delete [] normals; }
  if ( tex_coords_vbo ) { glDeleteBuffers( 1, &tex_coords_vbo ); }
  if ( tex_coords ) { delete [] tex_coords; }
  if ( vao ) { glDeleteVertexArrays( 1, &vao ); }
}

/**
 * Helper method to initialize OpenGL buffers for a mesh.
 */
void mesh::init_buffers() {
  // Create a VAO to store vertex attribute data.
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  // Create and buffer the position values in a VBO.
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER,
                ( 3 * num_vertices * sizeof( GLfloat ) ),
                points, GL_STATIC_DRAW );
  glVertexAttribPointer(  0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  // Create and buffer the normal values in a VBO.
  glGenBuffers( 1, &normals_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, normals_vbo );
  glBufferData( GL_ARRAY_BUFFER,
                ( 3 * num_vertices * sizeof( GLfloat ) ),
                normals, GL_STATIC_DRAW );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 1 );
  // Create and buffer the texture coordinate values in a VBO.
  glGenBuffers( 1, &tex_coords_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, tex_coords_vbo );
  glBufferData( GL_ARRAY_BUFFER,
                ( 2 * num_vertices * sizeof( GLfloat ) ),
                tex_coords, GL_STATIC_DRAW );
  glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 2 );
}

/**
 * Scale a mesh by X/Y/Z scalars stored in a 3-vector.
 */
void mesh::scale_by( v3 s ) {
  // TODO: SIMD candidate.
  for ( int i = 0; i < num_vertices * 3; i += 3 ) {
    points[ i ]   *= s.v[ 0 ];
    points[ i+1 ] *= s.v[ 1 ];
    points[ i+2 ] *= s.v[ 2 ];
  }

  // Update the buffer objects.
  // TODO: This should be baked into the mesh transformation matrix.
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferSubData( GL_ARRAY_BUFFER, 0,
                   3 * num_vertices * sizeof( GLfloat ), points );
}
