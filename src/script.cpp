#include "script.h"

/** Generic script constructor; just set the 'parent' game object. */
script::script( unity* p ) {
  parent = p;
}

/** Shared script destructor. Currently does nothing. */
script::~script() {}

/**
 * Default 'call' function. Does nothing; individual
 * script implementations need to override this method.
 */
void script::call( float dt ) {}

/**
 * Test 'on-use' script's 'call' method. Toggle between opening
 * a GUI panel with the configured text and pausing the game,
 * and closing the GUI panel and un-pausing the game.
 */
void s_test_use::call( float dt ) {
  if ( g->paused ) {
    g->g_man->cur_gui->root->clear_all();
    g->paused = false;
  }
  else {
    g->g_man->cur_gui->root->load_book_screen( my_text.c_str() );
    g->paused = true;
  }
}

/**
 * Set the text for the script to display, from a string variable.
 */
void s_test_use::set_text_contents_to( string text ) {
  my_text = text;
}

/**
 * Set the text for the script to display, from a text file.
 */
void s_test_use::set_text_contents_from( const char* fn ) {
  my_text = read_from_file( fn );
}
