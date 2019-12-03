#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <assert.h>
#include <math.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::mt19937;
using std::random_device;

// Basic UI values. Default to 1280x720.
int win_w = 1280;
int win_h = 720;

// Main game object pointer.
game* g = 0;
// PRNG example usage: (int)(re()%100) = [0, 100).
// re() returns [0, big].
random_device rd;
mt19937 re( rd() );

/**
 * Main method.
 *
 * Performs basic command-line argument processing, sets up the
 * main game object, and runs the outermost game loop.
 */
int main( int argc, char** args ) {
  // Setup the log file.
  assert( restart_log() == 0 );

  // Initialize the game object.
  g = new game( win_w, win_h );

  // Apply settings based on command-line arguments.
  if ( argc >= 2 ) {
    for ( int i = 1; i < argc; ++i ) {
      if ( !strcmp( args[ i ], "-e" ) ) {
        g->editor = true;
      }
      if ( !strcmp( args[ i ], "-c" ) ) {
        export_mesh_json( args[ i + 1 ], args[ i + 2 ] );
        return 0;
      }
    }
  }

  // Initialize the 'game' object.
  printf( "Init\r\n" );
  g->init();

  // Load a file if applicable.
  if ( argc >= 3 ) {
    for ( int i = 1; i < ( argc - 1 ); ++i ) {
      if ( !strcmp( args[ i ], "-l" ) ) {
        g->reload_file( args[ i + 1 ] );
      }
    }
  }

  // Process the game loop until it's time to quit.
  while ( !glfwWindowShouldClose( g->window ) && !g->should_quit ) {
    g->process_game_loop();
  }

  // Done; clean up and exit.
  delete g;
  glfwTerminate();
  return 0;
}
