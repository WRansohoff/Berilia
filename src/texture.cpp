#include "texture.h"

/**
 * Default (empty) constructor for a
 * sprite atlas pixel range definition.
 */
atlas_px_range::atlas_px_range() {
  key = "default";
  x = y = w = h = 0;
}

/**
 * Constructor for a sprite atlas pixel range definition.
 */
atlas_px_range::atlas_px_range( const char* k,
                                int atlas_x, int atlas_y,
                                int atlas_w, int atlas_h ) {
  x = atlas_x;
  y = atlas_y;
  w = atlas_w;
  h = atlas_h;
  key = k;
}

/**
 * Constructor for a texture object.
 */
texture::texture( const char* filename, GLenum gl_tex_slot ) {
  tex_slot = gl_tex_slot;
  tex_sampler = 0.0f;
  load_texture( filename );
}

/**
 * Destructor for a texture object.
 */
texture::~texture() {
  if ( tex ) {
    glDeleteTextures( 1, &tex );
  }
  if ( tex_buffer ) {
    delete [] tex_buffer;
  }
}

/**
 * Load a texture using the stb_image library.
 */
void texture::load_texture(const char* filename) {
  // Load the texture from a file.
  unsigned char* m_tex_buffer = stbi_load( filename,
                                           &tex_x, &tex_y, &tex_n,
                                           tex_channels );
  if ( !m_tex_buffer ) {
    log_error( "[ERROR] STB Image Could not load file: %s\n",
               filename );
    return;
  }
  // Shim the malloc'd buffer; I use new.
  tex_buffer = new unsigned char[ tex_x * tex_y * tex_n ];
  memcpy( &tex_buffer[ 0 ], &m_tex_buffer[ 0 ],
          tex_x * tex_y * tex_n );
  free( m_tex_buffer );
  // This is a handy math trick I found in Anton's OpenGL Tutorials;
  // If x is a power of 2, there's only 1 1 in the whole number.
  // So x-1 will be 0...01...1.
  // Log if a texture isn't a power of 2 to a side;
  // the GPU will pad it out, waste of memory.
  if ( ( tex_x & ( tex_x - 1 ) ) != 0 ||
       ( tex_y & ( tex_y - 1 ) ) != 0 ) {
    if ( BRLA_TEX_DEBUG ) {
      log( "[WARN ] Loaded a Texture Of Unusual Size.\n  "
           "%s has x or y resolution that is not a power of 2.",
           filename );
    }
  }

  // Flip the texture vertically, and bind it as the active texture.
  flip_tex_V(tex_buffer, tex_x, tex_y, tex_n);
  bind();
}

/**
 * Load a uniform font atlas texture from a file.
 */
void texture::load_uniform_font_atlas() {
  // Set the 'sprite atlas' texture capability.
  capabilities |= BRLA_TEX_CAP_ATLAS;

  // Set the atlas. This is just a helper method for my font sheets,
  // so I'll use a set format for now.
  int l_w = 8;
  int l_h = 16;
  tex_atlas[ "a" ] = atlas_px_range( "a", 0, 0, l_w, l_h );
  tex_atlas[ "b" ] = atlas_px_range( "b", 8, 0, l_w, l_h );
  tex_atlas[ "c" ] = atlas_px_range( "c", 16, 0, l_w, l_h );
  tex_atlas[ "d" ] = atlas_px_range( "d", 24, 0, l_w, l_h );
  tex_atlas[ "e" ] = atlas_px_range( "e", 32, 0, l_w, l_h );
  tex_atlas[ "f" ] = atlas_px_range( "f", 40, 0, l_w, l_h );
  tex_atlas[ "g" ] = atlas_px_range( "g", 48, 0, l_w, l_h );
  tex_atlas[ "h" ] = atlas_px_range( "h", 56, 0, l_w, l_h );
  tex_atlas[ "i" ] = atlas_px_range( "i", 0, 16, l_w, l_h );
  tex_atlas[ "j" ] = atlas_px_range( "j", 8, 16, l_w, l_h );
  tex_atlas[ "k" ] = atlas_px_range( "k", 16, 16, l_w, l_h );
  tex_atlas[ "l" ] = atlas_px_range( "l", 24, 16, l_w, l_h );
  tex_atlas[ "m" ] = atlas_px_range( "m", 32, 16, l_w, l_h );
  tex_atlas[ "n" ] = atlas_px_range( "n", 40, 16, l_w, l_h );
  tex_atlas[ "o" ] = atlas_px_range( "o", 48, 16, l_w, l_h );
  tex_atlas[ "p" ] = atlas_px_range( "p", 56, 16, l_w, l_h );
  tex_atlas[ "q" ] = atlas_px_range( "q", 0, 32, l_w, l_h );
  tex_atlas[ "r" ] = atlas_px_range( "r", 8, 32, l_w, l_h );
  tex_atlas[ "s" ] = atlas_px_range( "s", 16, 32, l_w, l_h );
  tex_atlas[ "t" ] = atlas_px_range( "t", 24, 32, l_w, l_h );
  tex_atlas[ "u" ] = atlas_px_range( "u", 32, 32, l_w, l_h );
  tex_atlas[ "v" ] = atlas_px_range( "v", 40, 32, l_w, l_h );
  tex_atlas[ "w" ] = atlas_px_range( "w", 48, 32, l_w, l_h );
  tex_atlas[ "x" ] = atlas_px_range( "x", 56, 32, l_w, l_h );
  tex_atlas[ "y" ] = atlas_px_range( "y", 0, 48, l_w, l_h );
  tex_atlas[ "z" ] = atlas_px_range( "z", 8, 48, l_w, l_h );
  tex_atlas[ "A" ] = atlas_px_range( "A", 16, 48, l_w, l_h );
  tex_atlas[ "B" ] = atlas_px_range( "B", 24, 48, l_w, l_h );
  tex_atlas[ "C" ] = atlas_px_range( "C", 32, 48, l_w, l_h );
  tex_atlas[ "D" ] = atlas_px_range( "D", 40, 48, l_w, l_h );
  tex_atlas[ "E" ] = atlas_px_range( "E", 48, 48, l_w, l_h );
  tex_atlas[ "F" ] = atlas_px_range( "F", 56, 48, l_w, l_h );
  tex_atlas[ "G" ] = atlas_px_range( "G", 0, 64, l_w, l_h );
  tex_atlas[ "H" ] = atlas_px_range( "H", 8, 64, l_w, l_h );
  tex_atlas[ "I" ] = atlas_px_range( "I", 16, 64, l_w, l_h );
  tex_atlas[ "J" ] = atlas_px_range( "J", 24, 64, l_w, l_h );
  tex_atlas[ "K" ] = atlas_px_range( "K", 32, 64, l_w, l_h );
  tex_atlas[ "L" ] = atlas_px_range( "L", 40, 64, l_w, l_h );
  tex_atlas[ "M" ] = atlas_px_range( "M", 48, 64, l_w, l_h );
  tex_atlas[ "N" ] = atlas_px_range( "N", 56, 64, l_w, l_h );
  tex_atlas[ "O" ] = atlas_px_range( "O", 0, 80, l_w, l_h );
  tex_atlas[ "P" ] = atlas_px_range( "P", 8, 80, l_w, l_h );
  tex_atlas[ "Q" ] = atlas_px_range( "Q", 16, 80, l_w, l_h );
  tex_atlas[ "R" ] = atlas_px_range( "R", 24, 80, l_w, l_h );
  tex_atlas[ "S" ] = atlas_px_range( "S", 32, 80, l_w, l_h );
  tex_atlas[ "T" ] = atlas_px_range( "T", 40, 80, l_w, l_h );
  tex_atlas[ "U" ] = atlas_px_range( "U", 48, 80, l_w, l_h );
  tex_atlas[ "V" ] = atlas_px_range( "V", 56, 80, l_w, l_h );
  tex_atlas[ "W" ] = atlas_px_range( "W", 0, 96, l_w, l_h );
  tex_atlas[ "X" ] = atlas_px_range( "X", 8, 96, l_w, l_h );
  tex_atlas[ "Y" ] = atlas_px_range( "Y", 16, 96, l_w, l_h );
  tex_atlas[ "Z" ] = atlas_px_range( "Z", 24, 96, l_w, l_h );
  tex_atlas[ "0" ] = atlas_px_range( "0", 32, 96, l_w, l_h );
  tex_atlas[ "1" ] = atlas_px_range( "1", 40, 96, l_w, l_h );
  tex_atlas[ "2" ] = atlas_px_range( "2", 48, 96, l_w, l_h );
  tex_atlas[ "3" ] = atlas_px_range( "3", 56, 96, l_w, l_h );
  tex_atlas[ "4" ] = atlas_px_range( "4", 0, 112, l_w, l_h );
  tex_atlas[ "5" ] = atlas_px_range( "5", 8, 112, l_w, l_h );
  tex_atlas[ "6" ] = atlas_px_range( "6", 16, 112, l_w, l_h );
  tex_atlas[ "7" ] = atlas_px_range( "7", 24, 112, l_w, l_h );
  tex_atlas[ "8" ] = atlas_px_range( "8", 32, 112, l_w, l_h );
  tex_atlas[ "9" ] = atlas_px_range( "9", 40, 112, l_w, l_h );
  tex_atlas[ "_" ] = atlas_px_range( "_", 48, 112, l_w, l_h );
  tex_atlas[ "." ] = atlas_px_range( ".", 56, 112, l_w, l_h );
  tex_atlas[ " " ] = atlas_px_range( " ", 0, 128, l_w, l_h );
  tex_atlas[ ":" ] = atlas_px_range( ":", 8, 128, l_w, l_h );
  tex_atlas[ "-" ] = atlas_px_range( "-", 16, 128, l_w, l_h );
  tex_atlas[ "/" ] = atlas_px_range( "/", 24, 128, l_w, l_h );
  tex_atlas[ "?" ] = atlas_px_range( "?", 32, 128, l_w, l_h );
  tex_atlas[ ">" ] = atlas_px_range( ">", 40, 128, l_w, l_h );
  tex_atlas[ "<" ] = atlas_px_range( "<", 48, 128, l_w, l_h );
  tex_atlas[ "'" ] = atlas_px_range( "'", 56, 128, l_w, l_h );
  tex_atlas[ "\"" ] = atlas_px_range( "\"", 0, 144, l_w, l_h );
  tex_atlas[ "+" ] = atlas_px_range( "+", 8, 144, l_w, l_h );
  // (Empty slot currently used by an extra '-')
  tex_atlas[ "*" ] = atlas_px_range( "*", 24, 144, l_w, l_h );
  tex_atlas[ "%" ] = atlas_px_range( "%", 32, 144, l_w, l_h );
  tex_atlas[ "[" ] = atlas_px_range( "[", 40, 144, l_w, l_h );
  tex_atlas[ "]" ] = atlas_px_range( "]", 48, 144, l_w, l_h );
  tex_atlas[ "!" ] = atlas_px_range( "!", 56, 144, l_w, l_h );
}

/**
 * Bind the current texture to slot #0, and set some
 * basic texture parameters.
 */
void texture::bind() {
  int mipmap_LOD = 0;
  glGenTextures( 1, &tex );
  // TODO: Use 'tex_slot' instead of 'GL_TEXTURE0'?
  //glActiveTexture( tex_slot );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, tex );
  glTexImage2D( GL_TEXTURE_2D, mipmap_LOD, GL_SRGB_ALPHA,
                tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                tex_buffer );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  // Nearest-neighbor; pixelated look.
  /*
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  */
  // Bilinear mipmap filtering; smoothed, but sometimes 'muddy', look.
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  /*
  glTexParameteri( GL_TEXTURE_2D,
                   GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glGenerateMipmap( GL_TEXTURE_2D );
  */
  // Enable max supported level of anisotropic filtering.
  GLfloat max_anisotropic = 0.0f;
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropic );
  glTexParameterf( GL_TEXTURE_2D,
                   GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropic );
}

/**
 * Texture manager constructor.
 */
texture_manager::texture_manager() {
  // Set the maximum number of active textures.
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_textures);
  // Reserve 1 texture for the GUI.
  num_textures -= 1;
  // And reserve 1 texture for the NPR depth buffer.
  num_textures -= 1;
  // And 1 for the NPR texture buffer.
  num_textures -= 1;
  // And 1 for a shadow depth buffer.
  num_textures -= 1;
}

/**
 * Texture manager destructor: delete any stored textures.
 */
texture_manager::~texture_manager() {
  for ( auto tex_iter = tex_fn_map.begin();
        tex_iter != tex_fn_map.end();
        ++tex_iter ) {
    if ( tex_iter->second ) {
      delete tex_iter->second;
      tex_iter->second = 0;
    }
  }
}

/**
 * Add a texture mapping to the texture manager.
 * Textures are stored in a hash map by string key.
 */
void texture_manager::add_mapping( string key, texture* tex ) {
  evict_mapping( key );
  tex_fn_map[ key ] = tex;
}

/**
 * Add a texture mapping to the texture manager by filename.
 * This method loads the texture and adds it to the manager in
 * one step, using the filename as the string key.
 */
texture* texture_manager::add_mapping_by_fn( string fn ) {
  texture* tex = new texture( fn.c_str(), GL_TEXTURE0 );
  evict_mapping( fn );
  tex_fn_map[ fn ] = tex;
  return tex;
}

/**
 * Evict a texture from the texture manager, and delete it.
 */
void texture_manager::evict_mapping( string key ) {
  auto tex_iter = tex_fn_map.find( key );
  if ( tex_iter != tex_fn_map.end() ) {
    if ( tex_iter->second ) {
      delete tex_iter->second;
      tex_iter->second = 0;
      tex_fn_map.erase( key );
    }
  }
}

/**
 * Retrieve a texture from the texture manager by string key.
 */
texture* texture_manager::get( string key ) {
  auto tex_iter = tex_fn_map.find( key );
  if ( tex_iter != tex_fn_map.end() ) {
    return tex_iter->second;
  }
  return 0;
}
