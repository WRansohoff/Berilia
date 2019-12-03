#ifndef BRLA_SHADERS_H
#define BRLA_SHADERS_H

#include <GL/glew.h>

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.h"
#include "util.h"

// Max length in chars of OpenGL Shader logs to record.
#define GL_SHADER_LOG_LEN 2048

using std::string;
using std::unordered_map;
using std::vector;

class game;

class shader_manager {
protected:
  void load_shader( const char* s_fn, GLuint s );

public:
  unordered_map<string, GLuint> shader_map;
  GLuint cur_shader = 0;

  shader_manager();
  ~shader_manager();

  void add_shader_prog( string key, string vert_fn, string frag_fn );
  void evict_mapping( string key );
  GLuint get( string key );

  void swap_shader( GLuint shader );
  void swap_shader( string key );
};

#endif
