#ifndef BRLA_GUI_H
#define BRLA_GUI_H

#include <GL/glew.h>

#include <algorithm>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "json.hpp"

#include "game.h"
#include "math3d.h"
#include "script.h"
#include "stb_image.h"
#include "texture.h"
#include "tinydir.h"
#include "unity.h"

// TODO: I should use an existing GUI library, like maybe nuklear.

/**
 * GUI X anchor definitions for the
 * left / right side of the screen.
 */
enum gui_x_anchors {
  BRLA_GUI_X_LEFT = 0,
  BRLA_GUI_X_RIGHT = 1
};
/**
 * GUI Y anchor definitions for the
 * top / bottom side of the screen.
 */
enum gui_y_anchors {
  BRLA_GUI_Y_TOP = 0,
  BRLA_GUI_Y_BOTTOM = 1
};

/**
 * GUI capabilities for individual elements.
 * These define extra capabilities like buttons, text entry, etc.
 */
enum gui_capabilities {
  BRLA_GUI_CAP_NONE = 0,
  BRLA_GUI_CAP_BUTTON = 1,
  BRLA_GUI_CAP_TEXTBOX = 2,
  BRLA_GUI_CAP_SCROLLBAR = 4,
  BRLA_GUI_CAP_SCROLLABLE = 8
};
/**
 * GUI buffer types; track whether GUI elements are within
 * the bounds of the window.
 */
enum gui_buffer_types {
  BRLA_BUF_NORMAL = 0,
  BRLA_BUF_PARTIAL = 1,
  BRLA_BUF_OOB = 2
};

using json = nlohmann::json;

using std::function;
using std::max;
using std::memcpy;
using std::min;
using std::set;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

class game;
class gui_t;
class script;
class unity;

/**
 * GUI panel class. GUI panels are arranged in a tree structure,
 * where child elements exist inside the boundaries of parent
 * elements.
 * Panels with lower z-index values are drawn first, so they will
 * appear 'below' elements with larger z-index values. But z-index
 * comparisons are only made between panels which share a parent.
 */
class gui_panel {
public:
  /** Parent GUI object. */
  gui_t* p_gui = 0;
  /** Parent GUI panel object. */
  gui_panel* parent = 0;
  /** Array of child GUI panel objects. */
  vector<gui_panel*> children;
  /** The name of the GUI panel object. */
  string name;
  /**
   * X-anchor type for the GUI panel object.
   * A 'gui_x_anchor' value, this determines whether the panel
   * is offset from the left or right side of the parent panel.
   */
  int x_anchor;
  /**
   * Y-anchor type for the GUI panel object.
   * A 'gui_y_anchor' value, this determines whether the panel
   * is offset from the top or bottom of the parent panel.
   */
  int y_anchor;
  /** X-offset of the GUI panel. */
  int x = 0;
  /** Y-offset of the GUI panel. */
  int y = 0;
  /** Width of the GUI panel. */
  int w = 0;
  /** Height of the GUI panel. */
  int h = 0;
  /** X-offset of the GUI viewport. */
  int vx = 0;
  /** Y-offset of the GUI viewport. */
  int vy = 0;
  /** Width of the GUI viewport. */
  int vw = 0;
  /** Height of the GUI viewport. */
  int vh = 0;
  /** Z-index of the GUI panel amongst its siblings. */
  int z_index = 0;
  /**
   * If the 'text input' capability is set, this value tracks
   * the cursor's index in the text string.
   */
  int tex_cursor_ind = 0;
  /**
   * Type of this GUI panel's buffer; basically, an indication of
   * whether the panel fits within the bounds of its parent.
   */
  int buffer_type = BRLA_BUF_NORMAL;
  /** RGBA buffer for the GUI panel texture. */
  unsigned char* panel_buffer = 0;
  /**
   * RGBA buffer for a sub-area of the GUI panel,
   * if it only partially fits within the bounds of its parents.
   */
  unsigned char* sub_buffer = 0;
  /**
   * GUI capability flags. These indicate whether a GUI panel can
   * implement interactive elements like a button, text entry, etc.
   */
  int flags = 0;
  /**
   * If the 'button' capability is set, this function will execute
   * when the GUI panel is clicked on.
   */
  function<void( gui_panel* me )> clicked;
  /** String representing text contents of the GUI panel. */
  string text_contents = "";

  gui_panel( string n, int x_anch, int y_anch,
             int x_off, int y_off, const char* tex_fn,
             gui_panel* panel_parent, gui_t* gui_parent );
  gui_panel( string n, int x_anch, int y_anch,
             int x_off, int y_off, int p_w, int p_h,
             gui_panel* panel_parent, gui_t* gui_parent );
  ~gui_panel();

  void write_to_buffer( GLuint tex, int tex_ind );
  void resize( int dx, int dy );
  bool on_click( int m_x, int m_y );
  bool on_key( const char c );
  gui_panel* find_panel( string n );

  void empty_gui_buffer();
  void set_px( int p_x, int p_y,
               unsigned char r, unsigned char g,
               unsigned char b, unsigned char a );
  void fill( int p_x, int p_y, int p_w, int p_h,
             unsigned char r, unsigned char g,
             unsigned char b, unsigned char a );
  void outline( int border,
                unsigned char r, unsigned char g,
                unsigned char b, unsigned char a );
  void draw_outline( int x, int y, int w, int h, int o, v4 color );
  void text( int t_x, int t_y, string t, string font );
  void text( int t_x, int t_y, string t, string font, v4 color );
  void text( int t_x, int t_y,
             string t, string font,
             v4 color, int scale );

  void make_button( function<void( gui_panel* me )> click_func );
  void make_text_entry();

  void draw_crosshair();
  void draw_targeted_label( unity* target );

  void clear_all();
  void load_pause_screen();
  void load_book_screen( const char* text );
};

/**
 * Base GUI class. This represents a GUI which draws to the actual
 * display. It renders the tree structure represented by a
 * 'gui_panel' object to an OpenGL texture, which can be drawn
 * over the 3D game world render.
 */
class gui_t {
public:
  /** Pointer to the root of the 'gui_panel' tree structure. */
  gui_panel* root = 0;
  /** Pointer to the currently-selected GUI panel, if any. */
  gui_panel* selected = 0;
  /**
   * Pointer to the previously-selected GUI panel (if any), for
   * helping with things like secondary pop-up inputs.
   */
  gui_panel* prev_sel = 0;

  /** OpenGL Vertex Array Object for the GUI. */
  GLuint gui_vao = 0;
  /** OpenGL Vertex Buffer Object representing the GUI plane. */
  GLuint gui_pos_vbo = 0;
  /** OpenGL texture ID for the GUI object.  */
  GLuint gui_tex = 0;
  /** OpenGL texture index for the GUI object. */
  int gui_tex_index;
  /** Current width of the GUI, in pixels. */
  int cur_w;
  /** Current height of the GUI, in pixels. */
  int cur_h;
  /** Current script page, for use in the 'level editor' mode. */
  int cur_script_page_index = 0;
  /** Pointer to the RGBA GUI texture buffer. */
  unsigned char* gui_buffer = 0;
  /**
   * This value tracks whether the GUI has changed
   * since the last frame.
   */
  bool updated = true;

  gui_t( int x, int y );
  virtual ~gui_t();

  void resize( int r_x, int r_y );
  bool mouse_click( int m_x, int m_y );
  void key_press( const char c );
  void empty_gui_buffer();
  void update_gui_buffer();
  void cursor_L();
  void cursor_R();

  void update_selected();
  void update_script_subgui();
  void check_script_subgui_values( script* cur_script );
  void draw_to_texture();
  void draw();

  void load_editor_gui();

  void load_gui( string fn );
  void append_gui( const char* p_fn, gui_panel* p_parent );
  void load_gui_panel( json p, gui_panel* parent );
  void load_gui_panel( json p, gui_panel* parent,
                       int x_offset, int y_offset,
                       bool use_offset );
  void load_from_texture( const char* tex_fn );
  void write_to_panel( string panel_name, string panel_text );

  gui_panel* get_gui_panel( string name );
};

/**
 * 'GUI manager' class. This class keeps track of GUI
 * objects, processing input related to the GUI, and
 * drawing the active GUI panel tree.
 */
class gui_manager {
public:
  /** Pointer to the active GUI object. */
  gui_t* cur_gui = 0;
  /** Array of available GUI objects. */
  unordered_map<string, gui_t*> guis;

  gui_manager();
  ~gui_manager();

  void window_resize_update();
  bool mouse_click_update( int m_x, int m_y );
  void key_press( const char c );
  void update_selected();

  void add_gui( string key, gui_t* gui );
  void evict_gui( string key );
  gui_t* get_gui( string key );
  void swap_gui( string key );
  void clear_guis();

  void update();
  void draw();
};

#endif
