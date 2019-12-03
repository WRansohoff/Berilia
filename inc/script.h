#ifndef BRLA_SCRIPT_H
#define BRLA_SCRIPT_H

#include "game.h"
#include "math3d.h"
#include "unity.h"
#include "util.h"

#include <random>
#include <string>
#include <vector>

using std::mt19937;
using std::string;
using std::uniform_real_distribution;
using std::vector;

class game;
class unity;

/**
 * 'Script' object class. These objects store arbitrary
 * state variables, and contain a function call which
 * accepts one argument: the elapsed time since the last frame.
 * They also keep track of a 'parent' game object.
 * Basically, they're small functions which run during
 * gameplay and can keep track of state over time.
 */
class script {
public:
  /** The game object associated with this script, if any. */
  unity* parent = 0;
  /**
   * The script type. This is an abstract class, so
   * different script types will have different 'call' functions.
   */
  string type = "";
  /** Array of boolean state variables to track. */
  vector<bool> state_flags;
  /** Array of integer state variables to track. */
  vector<int> state_ints;
  /** Array of float state variables to track. */
  vector<float> state_floats;
  /** Array of string state variables to track. */
  vector<string> state_strings;
  /** Array of pointer state variables to track. */
  vector<void*> state_pointers;

  script(unity* p);
  virtual ~script();

  virtual void call(float dt);
};

/**
 * Test 'on-use' script; show a GUI panel with some text.
 */
class s_test_use : public script {
public:
  /** The text string to display. */
  string my_text = "";

  /** Constructor - just set the 'type' string. */
  s_test_use( unity* p ) : script( p ) {
    type = "s_test_use";
  }

  void call( float dt );
  void set_text_contents_to( string text );
  void set_text_contents_from( const char* fn );
};

#endif
