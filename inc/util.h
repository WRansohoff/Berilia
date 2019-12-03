#ifndef BRLA_UTIL
#define BRLA_UTIL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstring>
#include <fstream>
#include <stdarg.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unordered_map>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "game.h"
#include "mesh.h"

using std::ifstream;
using std::memcpy;
using std::stof;
using std::string;
using std::to_string;

class game;
class mesh;

// File path to use for logging information during gameplay.
#define BRLA_LOG_FILE "log/berilia.log"
// 'w' to overwrite the log after each run, 'a' to append to the file.
#define BRLA_LOG_TYPE "a"

// General utility functions.
unsigned long get_file_length( ifstream& file );
void fill_float_buffer( float* dest, float* dat,
                        int start_index, int len );
mesh* load_mesh( const char* filename );
void export_mesh_json( const char* mesh_fn, const char* json_fn );
float str_to_f( string s );

// GL helpers.
string framebuffer_error_to_string( GLenum err );

// Recursive mesh import helpers.
// TODO: Move to the 'mesh' header/source files?
int count_vertices( const aiNode* cur_node,
                    const aiScene* scene, int c );
void import_node( const aiNode* cur_node, const aiScene* scene,
                  GLfloat* p, GLfloat* n, GLfloat* t, int index );

// Logging functions.
void log_gl_errors();
void log_fb_errors();
int restart_log();
int log( const char* msg, ... );
int log_error( const char* msg, ... );
void log_gl_info();
void write_to_file( const char* message, const char* filename );
string read_from_file( const char* filename );

#endif
