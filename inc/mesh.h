#ifndef BRLA_MESH_H
#define BRLA_MESH_H

#include <GL/glew.h>

#include <string>

#include "game.h"
#include "math3d.h"
#include "util.h"

using std::string;

class game;

class mesh {
public:
  int num_vertices;
  GLfloat* points = 0;
  GLfloat* normals = 0;
  GLfloat* tex_coords = 0;
  GLuint points_vbo = 0;
  GLuint normals_vbo = 0;
  GLuint tex_coords_vbo = 0;
  GLuint vao = 0;
  aabb bounding_box;
  m4 transform;
  v3 local_pos;

  mesh(int num_verts, GLfloat* p, GLfloat* n, GLfloat* t,
       aabb baa, m4 t_m);
  ~mesh();

  void init_buffers();
  void scale_by(v3 s);
};

#endif
