#ifndef BRLA_TEXTURE_H
#define BRLA_TEXTURE_H

#include <GL/glew.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "game.h"
#include "math2d.h"
#include "stb_image.h"
#include "util.h"

#define BRLA_TEX_DEBUG false
#define BRLA_TEX_CAP_ATLAS 1

using std::string;
using std::unordered_map;
using std::vector;

class game;

struct atlas_px_range {
  int x, y, w, h;
  const char* key;

  atlas_px_range();
  atlas_px_range( const char* k, int atlas_x, int atlas_y,
                  int atlas_w, int atlas_h );
};

// Hashing.
namespace std {
  template<>
  struct hash<atlas_px_range> {
    typedef atlas_px_range argument_type;
    typedef size_t result_type;

    result_type operator()( const argument_type& r ) const {
      return ( hash<int>()( r.x ) + 2 * hash<int>()( r.y ) +
               4 * hash<int>()( r.w ) + 8 * hash<int>()( r.h ) +
               16 * hash<const char*>()( r.key ) );
    }
  };
}

class texture {
public:
  GLenum tex_slot;
  GLuint tex = 0;
  GLfloat tex_sampler;
  int tex_x, tex_y, tex_n;
  int tex_channels = 4; // RGBA
  unsigned char* tex_buffer = 0;

  // Extra capabilities, e.g. if it's a sprite atlas.
  // 0 is a default texture.
  int capabilities = 0;
  unordered_map<string, atlas_px_range> tex_atlas;

  texture( const char* filename, GLenum gl_tex_slot );
  ~texture();

  void load_texture( const char* filename );
  void load_uniform_font_atlas();
  void bind();
};

class texture_manager {
public:
  unordered_map<string, texture*> tex_fn_map;
  int num_textures = 0;

  texture_manager();
  ~texture_manager();

  void add_mapping( string key, texture* tex );
  texture* add_mapping_by_fn( string fn );
  void evict_mapping( string key );
  texture* get( string key );
};

#endif
