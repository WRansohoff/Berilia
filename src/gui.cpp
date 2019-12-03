#include "gui.h"

/**
 * GUI panel constructor.
 * TODO: enumerate arguments and what they all do.
 */
gui_panel::gui_panel( string n,
                      int x_anch, int y_anch,
                      int x_off, int y_off,
                      const char* tex_fn,
                      gui_panel* panel_parent,
                      gui_t* gui_parent) {
  name = n;
  x_anchor = x_anch;
  y_anchor = y_anch;
  parent = panel_parent;
  p_gui = gui_parent;
  if ( parent ) {
    panel_parent->children.push_back( this );
  }

  int tex_n;
  // Use RGBA, 4 bytes per pixel.
  unsigned char* tex_panel_buffer = stbi_load( tex_fn,
                                               &w, &h,
                                               &tex_n, 4 );
  if ( !tex_panel_buffer ) {
    log_error( "[ERROR] (GUI) STB Image could not load file: %s\n",
               tex_fn );
    return;
  }

  // Calculate the right/bottom-aligned coords if necessary.
  if ( x_anchor == BRLA_GUI_X_LEFT ) {
    x = x_off;
  }
  else if ( x_anchor == BRLA_GUI_X_RIGHT ) {
    x = p_gui->cur_w - ( x_off + w );
  }
  if ( y_anchor == BRLA_GUI_Y_TOP ) {
    y = y_off;
  }
  else if ( y_anchor == BRLA_GUI_Y_BOTTOM ) {
    y = p_gui->cur_h - ( y_off + h );
  }
  vx = x;
  vy = y;
  vw = w;
  vh = h;

  // Shim STBI's malloc'd buffer into a 'new' one.
  // You're really not supposed to mix 'new/free' or 'malloc/delete'.
  panel_buffer = new unsigned char[ w * h * tex_n ];
  memcpy( &panel_buffer[ 0 ], &tex_panel_buffer[ 0 ], w * h * tex_n );
  free( tex_panel_buffer );
  // Flip the texture vertically, so it doesn't render upside-down.
  flip_tex_V( panel_buffer, w, h, 4 );
}

/**
 * GUI panel constructor.
 * TODO: enumerate arguments and what they all do.
 */
gui_panel::gui_panel( string n,
                      int x_anch, int y_anch,
                      int x_off, int y_off,
                      int p_w, int p_h,
                      gui_panel* panel_parent,
                      gui_t* gui_parent ) {
  name = n;
  x_anchor = x_anch;
  y_anchor = y_anch;
  w = p_w;
  h = p_h;
  parent = panel_parent;
  p_gui = gui_parent;
  if ( panel_parent ) {
    panel_parent->children.push_back( this );
  }

  if ( x_anchor == BRLA_GUI_X_LEFT ) {
    x = x_off;
  }
  else if ( x_anchor == BRLA_GUI_X_RIGHT ) {
    x = p_gui->cur_w - ( x_off + w );
  }
  if ( y_anchor == BRLA_GUI_Y_TOP ) {
    y = y_off;
  }
  else if ( y_anchor == BRLA_GUI_Y_BOTTOM ) {
    y = p_gui->cur_h - ( y_off + h );
  }

  // Calculate the viewport dimensions. Children can't draw outside their parent panel's bounds.
  vx = x;
  vy = y;
  vw = w;
  vh = h;
  if ( parent ) {
    if ( vx < parent->vx ) {
      // This panel is outside of its parent to the left.
      printf( "%s is off to the left: %i < %i\n",
              name.c_str(), vx, parent->vx );
      vx = parent->vx;
      if ( x + w > parent->vx + parent->vw ) {
        // Panel is out of both x-dimension bounds.
        vw = parent->vw;
      }
      else {
        vw = ( x + w ) - vx;
      }
    }
    else if ( x + w > parent->vx + parent->vw ) {
      // This panel is outside of its parent to the right.
      printf( "%s is off to the right\n", name.c_str() );
      vw = w - ( ( x + w ) - ( parent->vx + parent->vw ) );
    }
    if ( vy < parent->vy ) {
      // This panel is outside of its parent to the top.
      vy = parent->vy;
      if ( y + h > parent->vy + parent->vh ) {
        // Panel is out of both y-dimension bounds.
        vh = parent->vh;
      }
      else {
        vh = ( y + h ) - vy;
      }
    }
    else if ( y + h > parent->vy + parent->vh ) {
      // This panel is outside of its parent to the bottom.
      vh = h - ( ( y + h ) - ( parent->vy + parent->vh ) );
    }

    // Catch anything outside of the window's boundaries.
    if ( vx < 0 ) {
      vw += vx;
      vx = 0;
      if ( vw < 0 ) { 
        vw = 0;
        buffer_type = BRLA_BUF_OOB;
      }
    }
    if ( vx >= g->g_win_w ) {
      buffer_type = BRLA_BUF_OOB;
    }
    if ( vx + vw >= g->g_win_w ) {
      vw -= ( ( vx + vw ) - g->g_win_w );
      if ( vw < 0 ) { 
        vw = 0;
        buffer_type = BRLA_BUF_OOB;
      }
    }
    if ( vy < 0 ) {
      vh += vy;
      vy = 0;
      if ( vh < 0 ) { 
        vh = 0;
        buffer_type = BRLA_BUF_OOB;
      }
    }
    if ( vy >= g->g_win_h ) {
      buffer_type = BRLA_BUF_OOB;
    }
    if ( vy + vh >= g->g_win_h ) {
      vh -= ( ( vy + vh ) - g->g_win_h );
      if ( vh < 0 ) { 
        vh = 0;
        buffer_type = BRLA_BUF_OOB;
      }
    }

    if ( vx > parent->vx + parent->vw || vx + vw < parent->vx ||
         vy > parent->vy + parent->vh || vy + vh < parent->vy ) {
      vx = vy = vw = vh = 0;
      if ( sub_buffer ) { delete sub_buffer; }
      buffer_type = BRLA_BUF_OOB;
    }

    if ( buffer_type == BRLA_BUF_NORMAL &&
         ( vx != x || vy != y || vw != w || vh != h ) ) {
      if ( name == "right_divider_v01") {
        printf( "%i + %i > %i + %i\n", y, h, parent->vy, parent->vh );
      }
      buffer_type = BRLA_BUF_PARTIAL;
    }
  }

  // Allocate the buffers.
  // First, if applicable, Create and 0 a sub-buffer.
  if ( buffer_type == BRLA_BUF_PARTIAL ) {
    // Allocate the sub buffer.
    int sb_size = vw * vh * 4;
    if ( sb_size > 0 ) {
      sub_buffer = new unsigned char[ sb_size ];
      memset( sub_buffer, 0, sb_size );
    }
    else {
      // I mean, I hope this doesn't happen, but just in case...
      buffer_type = BRLA_BUF_OOB;
    }
  }

  // Allocate and zero out the panel buffer.
  // This means that newly-constructed panels will be transparent.
  // (Then fill with the parent buffer if applicable.)
  int panel_size = w * h * 4;
  panel_buffer = new unsigned char[ panel_size ];
  memset( panel_buffer, 0, panel_size );

  if ( parent && w != 0 && h != 0 ) {
    int dx = x - parent->x;
    int dy = y - parent->y;
    int sub_dx = vx - parent->x;
    int sub_dy = vy - parent->y;
    for ( int i = 0; i < w * 4; ++i ) {
      for ( int j = 0; j < h; ++j ) {
        int ind = i + j * ( w * 4 );
        int pi_x = dx * 4 + i;
        int pi_y = dy + j;
        int pi_ind = pi_x + pi_y * ( parent->w * 4 );
        if ( pi_x >= 0 && pi_x < parent->w*4 &&
             pi_y >= 0 && pi_y < parent->h ) {
          panel_buffer[ ind ] = parent->panel_buffer[ pi_ind ];
        }
        else {
          panel_buffer[ ind ] = 0;
        }

        // Set the sub-buffer as well, if applicable.
        if ( buffer_type == BRLA_BUF_PARTIAL ) {
          int sub_x = i;
          int sub_y = j;
          if ( x < parent->vx ) {
            sub_x = sub_x - ( vx - x ) * 4;
          }
          else {
            sub_x = sub_x - ( x - vx ) * 4;
          }
          if ( y < parent->vy ) {
            sub_y = sub_y - ( vy - y );
          }
          else {
            sub_y = sub_y - ( y - vy );
          }
          if ( sub_x >= 0 && sub_x < vw*4 &&
               sub_y >= 0 && sub_y < vh ) {
            sub_buffer[ sub_x + sub_y * vw * 4 ] =
              parent->panel_buffer[ pi_ind ];
          }
        }
      }
    }
  }
}

/**
 * GUI panel destructor. Remove this panel from its parent's
 * record of children (if it has a parent), delete its children,
 * and delete its RGBA buffers.
 */
gui_panel::~gui_panel() {
  if ( parent ) {
    for ( int i = 0; i < parent->children.size(); ++i ) {
      if ( parent->children[ i ] == this ) {
        parent->children.erase( parent->children.begin() + i );
      }
    }
  }

  for ( int i = 0; i < children.size(); ++i ) {
    if ( children[ i ] ) {
      delete children[ i ];
      children[ i ] = 0;
    }
  }
  if ( panel_buffer ) {
    delete panel_buffer;
    panel_buffer = 0;
  }
  if ( sub_buffer ) {
    delete sub_buffer;
    sub_buffer = 0;
  }
}

/**
 * Draw the GUI panel's RGBA buffers to an OpenGL texture,
 * and then draw its child elements on top.
 */
void gui_panel::write_to_buffer( GLuint tex, int tex_ind ) {
  if ( buffer_type == BRLA_BUF_NORMAL ) {
    glTexSubImage2D( GL_TEXTURE_2D, 0,
                     x, flip_y( y + h, p_gui->cur_h ),
                     w, h,
                     GL_RGBA, GL_UNSIGNED_BYTE,
                     panel_buffer );
  }
  else if ( buffer_type == BRLA_BUF_PARTIAL ) {
    glTexSubImage2D( GL_TEXTURE_2D, 0,
                     vx, flip_y( vy + vh, p_gui->cur_h ),
                     vw, vh,
                     GL_RGBA, GL_UNSIGNED_BYTE,
                     sub_buffer );
  }
  else if ( buffer_type == BRLA_BUF_OOB ) {
    // Out of bounds; don't draw anything.
  }

  // Draw children in order of their z-height indices.
  int max_z_ind = 0;
  for ( int i = 0; i < children.size(); ++i ) {
    if ( children[ i ]->z_index > max_z_ind ) {
      max_z_ind = children[ i ]->z_index;
    }
  }
  for ( int i = 0; i<=max_z_ind; ++i ) {
    for ( int j = 0; j < children.size(); ++j ) {
      if ( children[ j ]->z_index == i ) {
        children[ j ]->write_to_buffer( tex, tex_ind );
      }
    }
  }
}

/**
 * Callback to resizing a GUI panel by a given number of
 * pixels in the X / Y direction. Note that this adds 'dx'
 * and 'dy' to the current size, rather than resizing the
 * panel to be 'dx' and 'dy' pixels in each direction.
 * This is so the changes can be propagated to child panels.
 *
 * TODO: This method needs to resize buffers and width/height
 * values, but it doesn't at the moment.
 */
void gui_panel::resize( int dx, int dy ) {
  // Move left if right-anchored, and up if bottom-anchored.
  // Otherwise, do nothing.
  if (x_anchor == BRLA_GUI_X_RIGHT) {
    x += dx;
    vx += dx;
  }
  if (y_anchor == BRLA_GUI_Y_BOTTOM) {
    y += dy;
    vy += dy;
  }

  // TODO: Recalculate boundaries & de- / re-allocate sub buffers.
  // The fact that I don't do this is currently causing
  // 'GL_INVALID_VALUE' OpenGL errors in the 'write_to_buffer'
  // method, so this should be a priority even if it doesn't
  // cause the application to crash just yet.

  // Resize children.
  for (int i=0; i<children.size(); ++i) {
    children[i]->resize(dx, dy);
  }
}

/**
 * 'On-click' callback helper. This method finds the topmost
 * clickable element under the mouse cursor (if any) when
 * a mouse click event occurs, and performs the appropriate
 * GUI action. If no clickable GUI element is under the
 * mouse cursor, this method returns false so that the game
 * knows that it should process the mouse click in the 3D world.
 */
bool gui_panel::on_click( int m_x, int m_y ) {
  if ( !p_gui ) { return false; }
  if ( flags & BRLA_GUI_CAP_BUTTON ) {
    if ( p_gui->selected &&
         ( p_gui->selected->flags & BRLA_GUI_CAP_TEXTBOX ) ) {
      p_gui->selected->empty_gui_buffer();
      p_gui->selected->text( 0,
                             0,
                             p_gui->selected->text_contents,
                             g->f_mono );
    }
    // Execute the on click action of this panel.
    p_gui->selected = 0;
    g->pressed_keys.clear();
    clicked( this );
    return true;
  }
  else if ( flags & BRLA_GUI_CAP_TEXTBOX ) {
    if ( p_gui->selected &&
         ( p_gui->selected->flags & BRLA_GUI_CAP_TEXTBOX ) ) {
      p_gui->selected->empty_gui_buffer();
      p_gui->selected->text( 0,
                             0,
                             p_gui->selected->text_contents,
                             g->f_mono );
    }
    // Select this panel for text entry.
    p_gui->selected = this;
    // Set the cursor location.
    int rel_x = m_x - x;
    int c_ind = rel_x / 8;
    if ( c_ind > text_contents.length() ) {
      c_ind = text_contents.length();
    }
    fill( tex_cursor_ind * 8, 0, 1, 16, 0, 0, 0, 0 );
    tex_cursor_ind = c_ind;
    fill( tex_cursor_ind * 8, 0, 1, 16, 0, 0, 0, 255 );
    return true;
  }
  else {
    // Defer to the topmost child, if any.
    // Ignore any out-of-bounds children.
    bool hit = true;
    if ( !parent ) { hit = false; }
    gui_panel* highest_at_click = 0;
    for ( int i = 0; i < children.size(); ++i ) {
      if ( m_x >= children[i]->x &&
           m_x <= children[i]->x + children[i]->w &&
           m_y >= children[i]->y &&
           m_y <= children[i]->y + children[i]->h &&
           children[i]->buffer_type != BRLA_BUF_OOB ) {
        if ( !highest_at_click ||
             children[ i ]->z_index > highest_at_click->z_index ) {
          highest_at_click = children[ i ];
        }
      }
    }
    if ( highest_at_click ) {
      if ( highest_at_click->on_click( m_x, m_y ) ) {
        hit = true;
      }
    }
    return hit;
  }
}

/**
 * Callback method to process a key press which was sent to a
 * GUI element. Returns true if the keypress was used, false if not.
 */
bool gui_panel::on_key( const char c ) {
  // If this GUI panel has the 'text box' capability set,
  // adjust the text entry based on the keypress.
  if ( flags & BRLA_GUI_CAP_TEXTBOX ) {
    if ( c == '|' ) {
      // Backspace special character.
      // TODO: change how 'backspace' is handled and add 'delete'.
      // Delete the character before the cursor, if any.
      if ( text_contents.length() > 0 &&
           tex_cursor_ind > 0 ) {
        text_contents.erase( tex_cursor_ind - 1, 1 );
        tex_cursor_ind -= 1;
      }
    }
    else {
      // Insert the character at the cursor location.
      text_contents.insert( tex_cursor_ind, &c );
      tex_cursor_ind += 1;
    }
    empty_gui_buffer();
    fill( tex_cursor_ind * 8, 0, 1, 16, 0, 0, 0, 255 );
    text( 0, 0, text_contents, g->f_mono );
    return true;
  }
  // Return false if this GUI panel doesn't
  // accept keyboard inputs.
  else { return false; }
}

/**
 * Helper method to find a GUI panel by name in a
 * given tree structure. Returns 0 if no panel was found.
 */
gui_panel* gui_panel::find_panel( string n ) {
  // Return the current panel if the name matches.
  if ( name == n ) { return this; }
  // Otherwise, recursively check child panels.
  else {
    for ( int i = 0; i < children.size(); ++i ) {
      gui_panel* found = children[ i ]->find_panel( n );
      if ( found ) { return found; }
    }
  }
  // Return 0 if no matching panel was found.
  return 0;
}

/**
 * Empty out the GUI panel's RGBA buffer, by copying the relevant
 * parts of the parent element's buffer into it if possible.
 */
void gui_panel::empty_gui_buffer() {
  int panel_size = w * h * 4;
  if ( !parent ) {
    memset( panel_buffer, 0, panel_size );
  }
  else if ( x < parent->x ||
            x > parent->x + parent->w ||
            y < parent->y ||
            y > parent->y + parent->h ) {
    memset( panel_buffer, 0, panel_size );
    if ( buffer_type == BRLA_BUF_PARTIAL ) {
      int dx = x-parent->x;
      int dy = y-parent->y;
      for ( int i = 0; i < w * 4; ++i ) {
        for ( int j = 0; j < h; ++j ) {
          int pi_ind = ( dx * 4 + i ) +
                       ( dy + j ) * ( parent->w * 4 );
          if ( buffer_type == BRLA_BUF_PARTIAL ) {
            int sub_x = i;
            int sub_y = j;
            if ( x < parent->vx ) {
              sub_x = sub_x - ( vx - x ) * 4;
            }
            else {
              sub_x = sub_x - ( x - vx ) * 4;
            }
            if ( y < parent->vy ) {
              sub_y = sub_y - ( vy - y );
            }
            else {
              sub_y = sub_y - ( y - vy );
            }
            if ( sub_x >= 0 && sub_x < vw * 4 &&
                 sub_y >= 0 && sub_y < vh ) {
              sub_buffer[ sub_x + sub_y * vw * 4 ] =
                parent->panel_buffer[ pi_ind ];
            }
          }
        }
      }
    }
  }
  else {
    int dx = x-parent->x;
    int dy = y-parent->y;
    for ( int i = 0; i < w * 4; ++i ) {
      for ( int j = 0; j < h; ++j ) {
        int ind = i + j * ( w * 4 );
        int pi_ind = ( dx * 4 + i ) + ( dy + j ) * ( parent->w * 4 );
        panel_buffer[ ind ] = parent->panel_buffer[ pi_ind ];
      }
    }
  }
}

/**
 * Set an individual pixel in this GUI panel's buffer to a
 * given RGBA color. This method uses 'local' X / Y coordinates
 * which are relative to the panel boundaries, not the
 * display window boundaries.
 */
void gui_panel::set_px( int p_x, int p_y,
                        unsigned char r, unsigned char g,
                        unsigned char b, unsigned char a ) {
  int f_y = flip_y( p_y, h );
  panel_buffer[ f_y * w * 4 + p_x ] = r;
  panel_buffer[ f_y * w * 4 + p_x + 1 ] = g;
  panel_buffer[ f_y * w * 4 + p_x + 2 ] = b;
  panel_buffer[ f_y * w * 4 + p_x + 3 ] = a;

  if ( buffer_type == BRLA_BUF_PARTIAL ) {
    int sub_x = p_x - vx;
    int sub_y = f_y - vy;
    if ( sub_x >= 0 && sub_y >= 0 ) {
      sub_buffer[ sub_y * vw * 4 + sub_x ] = r;
      sub_buffer[ sub_y * vw * 4 + sub_x + 1 ] = g;
      sub_buffer[ sub_y * vw * 4 + sub_x + 2 ] = b;
      sub_buffer[ sub_y * vw * 4 + sub_x + 3 ] = a;
    }
  }
}

/**
 * Fill a rectangular area with a given RGBA color in the
 * current GUI panel. This uses local coordinates relative
 * to the GUI panel's buffer, not global window coordinates.
 * TODO: Comment this method.
 */
void gui_panel::fill( int p_x, int p_y, int p_w, int p_h,
                      unsigned char r, unsigned char g,
                      unsigned char b, unsigned char a ) {
  int f_y = flip_y( p_y, h );
  if ( p_x >= w || f_y >= h ) {
    printf("Bad w or h: %i < %i || %i < %i\n", p_x, w, f_y, h);
    return;
  }
  // Crop as necessary.
  int a_w = p_w;
  int a_h = p_h;
  if ( p_x + p_w > w ) { a_w = w - p_x; }
  if ( f_y - p_h < 0 ) { a_h = f_y; }

  int procpx = 0;
  for ( int i = f_y; i > ( f_y - a_h ); --i ) {
    for ( int j = p_x * 4; j < ( p_x + a_w ) * 4; j += 4 ) {
      panel_buffer[ i * w * 4 + j ] = r;
      panel_buffer[ i * w * 4 + j + 1 ] = g;
      panel_buffer[ i * w * 4 + j + 2 ] = b;
      panel_buffer[ i * w * 4 + j + 3 ] = a;

      if ( buffer_type == BRLA_BUF_PARTIAL ) {
        // TODO: This mostly works for now, but I'm not
        // too sure about the logic...
        int sub_x = ( j / 4 ) + p_x;
        int sub_y = i;
        if ( sub_x >= 0 && sub_x < ( vw ) * 4 &&
             sub_y >= 0 && sub_y < vh ) {
          int val = ( sub_y * vw + sub_x ) * 4;
          if ( val < 0 || val + 3 >= vw * vh * 4 ) {
            printf( "Maybe bad value? [%i, %i] = %i "
                    "([%i, %i], %i, %i)\n",
                    sub_x, sub_y, val,
                    vx, vy, vw, vh );
          }
          procpx += 1;
          sub_buffer[ ( sub_y * vw + sub_x ) * 4 ] = r;
          sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 1 ] = g;
          sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 2 ] = b;
          sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 3 ] = a;
        }
      }
    }
  }
}

/**
 * Outline the current panel with a specified RGBA color and
 * thickness in pixels.
 * TODO: Comment this method.
 */
void gui_panel::outline( int border,
                         unsigned char r,
                         unsigned char g,
                         unsigned char b,
                         unsigned char a ) {
  int b_h = border;
  int b_w = border;
  if ( b_h > h / 2 ) {
    b_h = h / 2;
  }
  if ( b_w > w / 2 ) {
    b_w = w / 2;
  }

  for ( int i = 0; i < b_h; ++i ) {
    for ( int j = 0; j < w * 4; j += 4 ) {
      panel_buffer[ i * w * 4 + j ] = r;
      panel_buffer[ i * w * 4 + j + 1 ] = g;
      panel_buffer[ i * w * 4 + j + 2 ] = b;
      panel_buffer[ i * w * 4 + j + 3 ] = a;

      if ( buffer_type == BRLA_BUF_PARTIAL ) {
        int sub_x = j - vx;
        int sub_y = i - vy;
        if ( sub_x >= 0 && sub_y >= 0 ) {
          sub_buffer[ sub_y * vw * 4 + sub_x ] = r;
          sub_buffer[ sub_y * vw * 4 + sub_x + 1 ] = g;
          sub_buffer[ sub_y * vw * 4 + sub_x + 2 ] = b;
          sub_buffer[ sub_y * vw * 4 + sub_x + 3 ] = a;
        }
      }
    }
  }
  for ( int i = h - b_h; i < h; ++i ) {
    for ( int j = 0; j < w * 4; j += 4 ) {
      panel_buffer[ i * w * 4 + j ] = r;
      panel_buffer[ i * w * 4 + j + 1 ] = g;
      panel_buffer[ i * w * 4 + j + 2 ] = b;
      panel_buffer[ i * w * 4 + j + 3 ] = a;

      if ( buffer_type == BRLA_BUF_PARTIAL ) {
        int sub_x = j - vx;
        int sub_y = i - vy;
        if ( sub_x >= 0 && sub_y >= 0 ) {
          sub_buffer[ sub_y * vw * 4 + sub_x ] = r;
          sub_buffer[ sub_y * vw * 4 + sub_x + 1 ] = g;
          sub_buffer[ sub_y * vw * 4 + sub_x + 2 ] = b;
          sub_buffer[ sub_y * vw * 4 + sub_x + 3 ] = a;
        }
      }
    }
  }
  for ( int i = 0; i < h; ++i ) {
    for ( int j = 0; j < b_w * 4; j += 4 ) {
      panel_buffer[ i * w * 4 + j ] = r;
      panel_buffer[ i * w * 4 + j + 1 ] = g;
      panel_buffer[ i * w * 4 + j + 2 ] = b;
      panel_buffer[ i * w * 4 + j + 3 ] = a;

      if ( buffer_type == BRLA_BUF_PARTIAL ) {
        int sub_x = j - vx;
        int sub_y = i - vy;
        if ( sub_x >= 0 && sub_y >= 0 ) {
          sub_buffer[ sub_y * vw * 4 + sub_x ] = r;
          sub_buffer[ sub_y * vw * 4 + sub_x + 1 ] = g;
          sub_buffer[ sub_y * vw * 4 + sub_x + 2 ] = b;
          sub_buffer[ sub_y * vw * 4 + sub_x + 3 ] = a;
        }
      }
    }
    for ( int j = ( w - b_w ) * 4; j < w * 4; j += 4 ) {
      panel_buffer[ i * w * 4 + j ] = r;
      panel_buffer[ i * w * 4 + j + 1 ] = g;
      panel_buffer[ i * w * 4 + j + 2 ] = b;
      panel_buffer[ i * w * 4 + j + 3 ] = a;

      if ( buffer_type == BRLA_BUF_PARTIAL ) {
        int sub_x = j - vx;
        int sub_y = i - vy;
        if ( sub_x >= 0 && sub_y >= 0 ) {
          sub_buffer[ sub_y * vw * 4 + sub_x ] = r;
          sub_buffer[ sub_y * vw * 4 + sub_x + 1 ] = g;
          sub_buffer[ sub_y * vw * 4 + sub_x + 2 ] = b;
          sub_buffer[ sub_y * vw * 4 + sub_x + 3 ] = a;
        }
      }
    }
  }
}

/**
 * Outline an area within the GUI panel, given an
 * interior border thickness.
 */
void gui_panel::draw_outline( int x, int y,
                              int w, int h,
                              int o, v4 color ) {
  int ao = o;
  if ( ao > w / 2 ) { ao = w / 2; }
  if ( ao > h / 2 ) { ao = h / 2; }
  unsigned char r = color.v[ 0 ];
  unsigned char g = color.v[ 1 ];
  unsigned char b = color.v[ 2 ];
  unsigned char a = color.v[ 3 ];

  fill( x, y, w, o, r, g, b, a );
  fill( x, y, o, h, r, g, b, a );
  fill( x + w - o, y, o, h, r, g, b, a );
  fill( x, y + h - o, w, o, r, g, b, a );
}

/**
 * Wrapper method to draw text at default scale (1)
 * and default color (black).
 */
void gui_panel::text( int t_x, int t_y, string t, string font ) {
  text( t_x, t_y, t, font, v4( 0, 0, 0, 255 ), 1 );
}

/**
 * Wrapper method to draw text at default scale (1)
 * with a given color.
 */
void gui_panel::text( int t_x, int t_y,
                      string t, string font,
                      v4 color ) {
  text( t_x, t_y, t, font, color, 1 );
}

/**
 * Draw text to the GUI panel, given a string and a key
 * to a font atlas in the main 'texture_manager' object.
 * If the 'alpha' channel of the color vector is 0,
 * then this method will use whatever color is present
 * in the font atlas texture instead of a predefined color.
 * Sorry, that's a little hack-y, but you wouldn't see
 * text with 0 opacity.
 */
void gui_panel::text( int t_x, int t_y,
                      string t, string font,
                      v4 color, int scale ) {
  int f_y = flip_y( t_y, h );
  int cur_x = t_x;
  int cur_y = f_y;
  if ( cur_x >= w || cur_y >= h ) {
    log( "[WARN ] (gui_panel::text) Bad x or y: "
         "X/W: %i/%i, Y/H: %i/%i\n",
         cur_x, w, cur_y, h );
    return;
  }

  atlas_px_range cur_char;
  texture* font_tex = g->t_man->get( font );
  if ( ( font_tex->capabilities & BRLA_TEX_CAP_ATLAS ) !=
       BRLA_TEX_CAP_ATLAS ) {
    log( "[WARN ] Font texture does not have "
         "the correct flags set\n" );
    return;
  }

  // Draw characters until they're out of panel, wrapping to
  // the next line when necessary and possible.
  for ( int i = 0; i < t.size(); ++i ) {
    auto cur_char_iter = font_tex->tex_atlas.find( t.substr( i, 1 ) );
    if ( cur_char_iter != font_tex->tex_atlas.end() ) {
      cur_char = cur_char_iter->second;
    }
    else {
      log( "[WARN ] Couldn't find character '%s'\n",
           t.substr( i, 1 ).c_str() );
    }

    // Paint the glyph.
    // TODO: improve wrapping, maybe by word instead of char.
    for ( int j = 0; j < cur_char.h * scale; ++j ) {
      int c_y = cur_y - j;
      for ( int k = 0; k < cur_char.w * scale; ++k ) {
        int c_x = k + cur_x;
        if ( cur_x + cur_char.w > w ) {
          cur_y += cur_char.h*scale;
          cur_x = t_x;
          c_x = k + cur_x;
          c_y = j + cur_y;
          if ( cur_y + cur_char.h * scale > h ) {
            return;
          }
        }
        else {
          int base_ind =  c_x * 4 + c_y * w * 4;
          int tex_ind = ( ( cur_char.x + k / scale ) * 4 ) +
                        ( cur_char.y + j / scale ) *
                        font_tex->tex_x * 4;
          if ( font_tex->tex_buffer[ tex_ind + 3 ] != 0 ) {
            if ( color.v[ 3 ] != 0 ) {
              panel_buffer[ base_ind ] = color.v[ 0 ];
              panel_buffer[ base_ind + 1 ] = color.v[ 1 ];
              panel_buffer[ base_ind + 2 ] = color.v[ 2 ];
              panel_buffer[ base_ind + 3 ] = color.v[ 3 ];
            }
            else {
              panel_buffer[ base_ind ] =
                font_tex->tex_buffer[ tex_ind ];
              panel_buffer[ base_ind + 1 ] =
                font_tex->tex_buffer[ tex_ind + 1 ];
              panel_buffer[ base_ind + 2 ] =
                font_tex->tex_buffer[ tex_ind + 2 ];
              panel_buffer[ base_ind + 3 ] =
                font_tex->tex_buffer[ tex_ind + 3 ];
            }

            if ( buffer_type == BRLA_BUF_PARTIAL ) {
              int sub_x = c_x;
              int sub_y = c_y;
              // I think this should be right...
              if ( x < parent->vx ) {
                sub_x = sub_x - ( parent->vx - x );
              }
              if ( y < parent->vy ) {
                sub_y = sub_y - ( parent->vy - y );
              }
              if ( sub_x >= 0 && sub_y >= 0 ) {
                if ( color.v[ 3 ] != 0 ) {
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 ] =
                    color.v[ 0 ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 1 ] =
                    color.v[ 1 ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 2 ] =
                    color.v[ 2 ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 3 ] =
                    color.v[ 3 ];
                }
                else {
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 ] =
                    font_tex->tex_buffer[ tex_ind ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 1 ] =
                    font_tex->tex_buffer[ tex_ind + 1 ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 2 ] =
                    font_tex->tex_buffer[ tex_ind + 2 ];
                  sub_buffer[ ( sub_y * vw + sub_x ) * 4 + 3 ] =
                    font_tex->tex_buffer[ tex_ind + 3 ];
                }
              }
            }
          }
        }
      }
    }
    cur_x += cur_char.w * scale;
  }
}

/**
 * Turn the GUI panel into a clickable button,
 * and set its on-click function.
 */
void gui_panel::make_button(
  function<void( gui_panel* me )> click_func ) {
  flags |= BRLA_GUI_CAP_BUTTON;
  clicked = click_func;
}

/**
 * Turn the GUI panel into a text entry area.
 */
void gui_panel::make_text_entry() {
  flags |= BRLA_GUI_CAP_TEXTBOX;
}

/**
 * Helper method to draw a crosshairs
 * over the center of the GUI panel.
 * TODO: Comment this method.
 */
void gui_panel::draw_crosshair() {
  int center_x = g->g_win_w / 2.0f;
  int center_y = g->g_win_h / 2.0f;
  for ( int ix = -5; ix < 7; ++ix ) {
    int ix_ind = ( center_x + ix ) * 4;
    for ( int jy = -5; jy < 7; ++jy ) {
      int jy_ind = ( center_y + jy ) * g->g_win_w * 4;
      if ( ix == -1 || ix == 2 ) {
        if ( jy > 2 || jy < -1 ) {
          panel_buffer[ ix_ind + jy_ind ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
        else {
          panel_buffer[ ix_ind + jy_ind ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 255.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
      }
      else if ( ix == 0 || ix == 1 ) {
        if ( jy == 6 || jy == -5 ) {
          panel_buffer[ ix_ind + jy_ind ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
        else {
          panel_buffer[ ix_ind + jy_ind ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 255.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
      }
      else if ( ix == -2 || ix == -3 || ix == -4 ||
                ix == 3 ||ix == 4 || ix == 5 ) {
        if ( jy == 2 || jy == -1 ) {
          panel_buffer[ ix_ind + jy_ind ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
        else if ( jy == 1 || jy == 0 ) {
          panel_buffer[ ix_ind + jy_ind ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 255.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 196.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
      }
      else if ( ix == -5 || ix == 6 ) {
        if ( jy < 3 && jy > -2 ) {
          panel_buffer[ ix_ind + jy_ind ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 1 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 2 ] = 0.0f;
          panel_buffer[ ix_ind + jy_ind + 3 ] = 255.0f;
        }
      }
    }
  }
}

/**
 * Draw a 'hover-over' label describing the object
 * being looked at. Mostly used in the 'level editor'
 * mode, but also useful for objects that can be used.
 * This method assumes that the GUI panel covers the
 * entire window; TODO: Add a check for that?
 */
void gui_panel::draw_targeted_label( unity* target ) {
  // Find the center of the window and empty the panel's buffer.
  int center_x = g->g_win_w / 2.0f;
  int center_y = g->g_win_h / 2.0f;
  empty_gui_buffer();

  // Return early if the target object doesn't exist.
  if ( !target ) { return; }

  // Add the name of the object and its type to the label string.
  string label_str = target->name;
  if ( label_str == "" ) {
    label_str = target->type;
    if ( label_str == "" ) { return; }
  }

  // If the object can be used, set a purple background color.
  if ( target->use_script ) {
    fill( center_x + 10, center_y + 10,
          label_str.length() * 8 + 8, 24, 128, 0, 128, 255 );
  }
  // If the object is only there for debugging purposes,
  // like lighting indicators, set an orange background color.
  else if ( target->type == "u_light_ind" ) {
    fill( center_x + 10, center_y + 10,
          label_str.length() * 8 + 8, 24, 196, 128, 0, 255 );
  }
  // Set a black background color for normal objects.
  else {
    fill( center_x + 10, center_y + 10,
          label_str.length() * 8 + 8, 24, 0, 0, 0, 255 );
  }
  // Draw an outline around the label box.
  // TODO: Use 'draw_outline' instead of 'fill'.
  fill( center_x + 10, center_y + 10,
        2, 24, 255, 255, 255, 255 );
  fill( center_x + 16 + label_str.length() * 8, center_y + 10,
        2, 24, 255, 255, 255, 255 );
  fill( center_x + 10, center_y + 10,
        label_str.length() * 8 + 8, 2, 255, 255, 255, 255 );
  fill( center_x + 10, center_y + 34,
        label_str.length() * 8 + 8, 2, 255, 255, 255, 255 );
  // Draw the actual label string.
  text( center_x + 14, center_y + 14,
        label_str, g->f_mono, v4( 255, 255, 255, 255 ) );
}

/**
 * Clear out all child GUI panels by deleting them and
 * emptying the array that keeps track of them.
 */
void gui_panel::clear_all() {
  for ( int i = 0; i < children.size(); ++i ) {
    delete children[ i ];
    children[ i ] = 0;
  }
  children.clear();
}

/**
 * Helper method to load a 'pause screen'.
 * For now, just draw a box overlay with a 'PAUSED' text label.
 */
void gui_panel::load_pause_screen() {
  int edge_x = 100;
  int edge_y = 60;
  int w = g->g_win_w-edge_x * 2;
  int h = g->g_win_h-edge_y * 2;

  gui_panel* pause_screen_panel = new gui_panel( "P_overlay_panel",
                                                 BRLA_GUI_X_LEFT,
                                                 BRLA_GUI_Y_TOP,
                                                 edge_x,
                                                 edge_y,
                                                 w,
                                                 h,
                                                 this,
                                                 p_gui );
  pause_screen_panel->fill( 0, 0, w, h, 196, 196, 196, 196 );
  pause_screen_panel->outline( 2, 0, 0, 0, 255 );

  int label_w = 56;
  gui_panel* pause_screen_text_outline =
    new gui_panel( "P_overlay_panel_label_outline",
                   BRLA_GUI_X_LEFT,
                   BRLA_GUI_Y_TOP,
                   ( g->g_win_w / 2 ) - ( label_w / 2 ),
                   edge_y,
                   label_w,
                   30,
                   pause_screen_panel,
                   p_gui );
  pause_screen_text_outline->fill( 0, 0, label_w, 30,
                                   255, 255, 255, 255 );
  pause_screen_text_outline->outline( 2, 0, 0, 0, 255 );
  pause_screen_text_outline->text( 4, 7, "PAUSED", g->f_mono );
}

/**
 * Helper method to load a 'book screen'. This is a quick
 * shortcut for displaying text in-game, and a way to
 * demonstrate an example game script.
 * TODO: Comment this method.
 */
void gui_panel::load_book_screen( const char* text ) {
  int sw = 640;
  int sh = 480;
  int x_l = ( g->g_win_w / 2 ) - ( sw / 2 );
  int y_t = ( g->g_win_h / 2 ) - ( sh / 2 );

  gui_panel* read_screen_panel = new gui_panel( "R_overlay_panel",
                                                BRLA_GUI_X_LEFT,
                                                BRLA_GUI_Y_TOP,
                                                x_l,
                                                y_t,
                                                sw,
                                                sh,
                                                this,
                                                p_gui);
  read_screen_panel->fill( 0, 0, sw, sh, 32, 64, 196, 196 );
  read_screen_panel->outline( 5, 64, 64, 64, 196 );

  gui_panel* read_screen_top_bar =
    new gui_panel( "R_overlay_top_bar",
                   BRLA_GUI_X_LEFT,
                   BRLA_GUI_Y_TOP,
                   x_l,
                   y_t,
                   sw,
                   30,
                   read_screen_panel,
                   p_gui );
  read_screen_top_bar->fill( 0, 0, sw, 30, 32, 96, 196, 196 );
  read_screen_top_bar->outline( 2, 64, 64, 64, 196 );

  gui_panel* read_screen_bottom_bar =
    new gui_panel( "R_overlay_bottom_bar",
                   BRLA_GUI_X_LEFT,
                   BRLA_GUI_Y_TOP,
                   x_l + 5,
                   y_t + sh - 35,
                   sw - 10,
                   30,
                   read_screen_panel,
                   p_gui );
  read_screen_bottom_bar->fill( 0, 0, sw - 10, 30,
                                32, 96, 196, 196 );
  read_screen_bottom_bar->outline( 2, 64, 64, 64, 196 );

  gui_panel* read_screen_close_button =
    new gui_panel( "R_overlay_close",
                   BRLA_GUI_X_RIGHT,
                   BRLA_GUI_Y_TOP,
                   x_l + 5,
                   y_t + 5,
                   20,
                   20,
                   read_screen_top_bar,
                   p_gui );
  read_screen_close_button->fill( 0, 0, 20, 20, 196, 196, 196, 196 );
  read_screen_close_button->outline( 2, 0, 0, 0, 196 );
  read_screen_close_button->text( 6, 2, "X", g->f_mono );
  function<void( gui_panel* me )> clicky = []( gui_panel* me ) {
    me->p_gui->root->clear_all();
    g->paused = false;
  };
  read_screen_close_button->make_button( clicky );

  gui_panel* read_screen_text_area =
    new gui_panel( "R_text_contents",
                   BRLA_GUI_X_LEFT,
                   BRLA_GUI_Y_TOP,
                   x_l + 15,
                   y_t + 40,
                   sw - 30,
                   sh - 80,
                   read_screen_panel,
                   p_gui );
  read_screen_text_area->text( 0, 0, text, g->f_mono );
}

/**
 * 'GUI object' constructor. Takes X / Y extents
 * for the maximum width / height of the GUI panel tree.
 */
gui_t::gui_t( int x, int y ) {
  // Set the current width / height according to the input args.
  cur_w = x;
  cur_h = y;

  // Set the RGBA buffer size and create it.
  int buf_size = cur_w * cur_h * 4;
  gui_buffer = new unsigned char[ buf_size ];
  // Clear the buffer.
  empty_gui_buffer();

  // Setup the OpenGL VAO and VBO.
  float gui_plane_pts[ 12 ] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
  };
  glGenBuffers( 1, &gui_pos_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, gui_pos_vbo );
  glBufferData( GL_ARRAY_BUFFER,
                sizeof( gui_plane_pts ),
                gui_plane_pts,
                GL_STATIC_DRAW );
  glGenVertexArrays( 1, &gui_vao );
  glBindVertexArray( gui_vao );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );

  // Setup the OpenGL texture object.
  gui_tex_index = g->t_man->num_textures;
  glGenTextures( 1, &gui_tex );
  // Draw the initial texture value.
  draw_to_texture();
}

/**
 * 'GUI object' destructor. Delete the GUI panel tree,
 * texture buffer, OpenGL texture object, and VAO / VBO.
 */
gui_t::~gui_t() {
  if ( root ) { delete root; }
  if ( gui_buffer ) { delete gui_buffer; }
  if ( gui_tex ) { glDeleteTextures( 1, &gui_tex ); }
  if ( gui_pos_vbo ) { glDeleteBuffers( 1, &gui_pos_vbo ); }
  if ( gui_vao ) { glDeleteBuffers( 1, &gui_vao ); }
}

/**
 * Resize the GUI object, probably in response to a window
 * resize or something like that. The X / Y arguments
 * represent the new width / height.
 */
void gui_t::resize( int r_x, int r_y ) {
  // Update the size values.
  int dx = r_x - cur_w;
  int dy = r_y - cur_h;
  cur_w = r_x;
  cur_h = r_y;

  // Delete and re-allocate the texture buffer for the new
  // interface's width / height.
  delete gui_buffer;
  int new_gui_size = cur_w * cur_h * 4;
  gui_buffer = new unsigned char[ new_gui_size ];
  memset( gui_buffer, 0, new_gui_size );

  // Call the recursive 'resize' method on the root GUI panel.
  if ( !root ) { return; }
  root->resize( dx, dy );
  // Also re-allocate the root GUI panel's texture buffer.
  delete root->panel_buffer;
  root->panel_buffer = new unsigned char[ cur_w * cur_h * 4 ];
  memset( root->panel_buffer, 0, new_gui_size );
  // Update the root GUI panel's width/height values.
  root->w = cur_w;
  root->h = cur_h;

  // Mark the GUI as having changed since the last frame.
  updated = true;
}

/**
 * Process a mouse click in the GUI.
 * Return true if the mouse click was 'used' by any
 * child GUI element, otherwise return false.
 */
bool gui_t::mouse_click( int m_x, int m_y ) {
  if ( root ) {
    // If the click is handled by a child GUI panel,
    // then mark the GUI as needing an update and return true.
    if ( root->on_click( m_x, m_y ) ) {
      updated = true;
      return true;
    }
  }
  // If a text box panel was previously selected,
  // re-draw that panel to remove the text cursor.
  if ( selected && ( selected->flags & BRLA_GUI_CAP_TEXTBOX ) ) {
    selected->empty_gui_buffer();
    selected->text(0, 0, selected->text_contents, g->f_mono);
  }
  // Un-select the currently-selected GUI panel.
  selected = 0;
  // Clear the map of pressed keyboard keys. (TODO: remove this?)
  g->pressed_keys.clear();
  // The GUI did not use the mouse click, so return false.
  return false;
}

/**
 * Process a key press sent to this GUI object.
 * This method just defers to the 'on_key' method of
 * the selected GUI panel, if any.
 */
void gui_t::key_press( const char c ) {
  if ( selected ) {
    selected->on_key( c );
    updated = true;
  }
}

/** Helper method to clear the GUI object's texture buffer. */
void gui_t::empty_gui_buffer() {
  memset( gui_buffer, 0, cur_w * cur_h * 4 );
}

/**
 * Helper method to activate and bind the GUI's OpenGL texture,
 * and draw the GUI panel tree to the texture buffer.
 */
void gui_t::update_gui_buffer() {
  // Set the OpenGL texture IDs.
  glActiveTexture( GL_TEXTURE0 + gui_tex_index );
  glBindTexture( GL_TEXTURE_2D, gui_tex );
  if ( root ) {
    // Draw a label describing the object that is currently
    // under the crosshairs. TODO: Should this only happen
    // in the 'level editor' mode?
    if ( g->looking_at_unity ) {
      root->draw_targeted_label( g->looking_at_unity );
    }
    else {
      root->empty_gui_buffer();
    }
    // Draw a small crosshairs to highlight the window's center.
    root->draw_crosshair();
    // Write / buffer the GUI texture data.
    root->write_to_buffer( gui_tex, gui_tex_index );
  }
}

/**
 * Helper method to move the cursor left
 * in the selected text entry panel, if any.
 */
void gui_t::cursor_L() {
  if ( selected && selected->flags & BRLA_GUI_CAP_TEXTBOX ) {
    if ( selected->tex_cursor_ind > 0 ) {
      --selected->tex_cursor_ind;
    }
    selected->empty_gui_buffer();
    selected->fill( selected->tex_cursor_ind * 8, 0, 1, 16,
                    0, 0, 0, 255 );
    selected->text( 0, 0, selected->text_contents, g->f_mono );
  }
}

/**
 * Helper method to move the cursor right
 * in the selected text entry panel, if any.
 */
void gui_t::cursor_R() {
  if ( selected && selected->flags & BRLA_GUI_CAP_TEXTBOX ) {
    if ( selected->tex_cursor_ind <
         selected->text_contents.length() ) {
      ++selected->tex_cursor_ind;
    }
    selected->empty_gui_buffer();
    selected->fill( selected->tex_cursor_ind*8, 0, 1, 16,
                    0, 0, 0, 255 );
    selected->text( 0, 0, selected->text_contents, g->f_mono );
  }
}

/**
 * 'Level editor' mode helper method: populate settings
 * with those of the currently-selected game object, if any.
 */
void gui_t::update_selected() {
  unity* selection = g->selected_unity;
  if ( g->editor && root ) {
    if ( !selection ) {
      get_gui_panel( "cur_sel_pos_x" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_pos_y" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_pos_z" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_rot_x" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_rot_y" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_rot_z" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_rot_t" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_scale_x" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_scale_y" )->empty_gui_buffer();
      get_gui_panel( "cur_sel_scale_z" )->empty_gui_buffer();
      write_to_panel( "cur_sel_name", "__Global__" );
    }
    else {
      v3 s_c = selection->cur_center;
      write_to_panel( "cur_sel_pos_x", to_string( s_c.v[ 0 ] ) );
      write_to_panel( "cur_sel_pos_y", to_string( s_c.v[ 1 ] ) );
      write_to_panel( "cur_sel_pos_z", to_string( s_c.v[ 2 ] ) );
      quat s_q = selection->rot;
      v4 s_r = v4( s_q.r[ 1 ], s_q.r[ 2 ], s_q.r[ 3 ], s_q.r[ 0 ] );
      write_to_panel( "cur_sel_rot_x", to_string( s_r.v[ 0 ] ) );
      write_to_panel( "cur_sel_rot_y", to_string( s_r.v[ 1 ] ) );
      write_to_panel( "cur_sel_rot_z", to_string( s_r.v[ 2 ] ) );
      write_to_panel( "cur_sel_rot_t", to_string( s_r.v[ 3 ] ) );
      v3 s_s = selection->cur_scale;
      write_to_panel( "cur_sel_scale_x", to_string( s_s.v[ 0 ] ) );
      write_to_panel( "cur_sel_scale_y", to_string( s_s.v[ 1 ] ) );
      write_to_panel( "cur_sel_scale_z", to_string( s_s.v[ 2 ] ) );
      write_to_panel( "cur_sel_name", selection->name );
    }

    if ( !g->selected_light ) {
      get_gui_panel( "cur_light_pos_x" )->empty_gui_buffer();
      get_gui_panel( "cur_light_pos_y" )->empty_gui_buffer();
      get_gui_panel( "cur_light_pos_z" )->empty_gui_buffer();
      write_to_panel( "cur_light_amb", "A: " );
      write_to_panel( "cur_light_diff", "D: " );
      write_to_panel( "cur_light_spec", "S: " );
      write_to_panel( "cur_light_exp_falloff", "E      F " );
      get_gui_panel( "cur_light_spot_cur_x" )->empty_gui_buffer();
      get_gui_panel( "cur_light_spot_cur_y" )->empty_gui_buffer();
      get_gui_panel( "cur_light_spot_cur_z" )->empty_gui_buffer();
      get_gui_panel( "cur_light_spot_cur_angle" )->empty_gui_buffer();
    }
    else {
      v4 l_c = g->selected_light->pos;
      write_to_panel( "cur_light_pos_x", to_string( l_c.v[ 0 ] ) );
      write_to_panel( "cur_light_pos_y", to_string( l_c.v[ 1 ] ) );
      write_to_panel( "cur_light_pos_z", to_string( l_c.v[ 2 ] ) );
      char buf[ 50 ];
      v3 l_a = g->selected_light->a;
      v3 l_d = g->selected_light->d;
      v3 l_s = g->selected_light->s;
      // TODO: snprintf?
      sprintf( buf, "A: %.2f %.2f %.2f",
               l_a.v[ 0 ], l_a.v[ 1 ], l_a.v[ 2 ] );
      string amb_str = buf;
      sprintf( buf, "D: %.2f %.2f %.2f",
               l_d.v[ 0 ], l_d.v[ 1 ], l_d.v[ 2 ] );
      string diff_str = buf;
      sprintf( buf, "S: %.2f %.2f %.2f",
               l_s.v[ 0 ], l_s.v[ 1 ], l_s.v[ 2 ] );
      string spec_str = buf;
      sprintf( buf, "E %.2f F %.2f",
               g->selected_light->specular_exp,
               g->selected_light->falloff );
      string e_f_str = buf;
      write_to_panel( "cur_light_amb", amb_str );
      write_to_panel( "cur_light_diff", diff_str );
      write_to_panel( "cur_light_spec", spec_str );
      write_to_panel( "cur_light_exp_falloff", e_f_str );
      sprintf( buf, "%.2f", g->selected_light->spot_rads );
      string spot_str = buf;
      get_gui_panel( "cur_light_spot_cur_angle" )->
        empty_gui_buffer();
      write_to_panel( "cur_light_spot_cur_angle", spot_str );
      v3 s_d = g->selected_light->dir;
      sprintf( buf, "%.2f", s_d.v[ 0 ] );
      string s_x_str = buf;
      sprintf( buf, "%.2f", s_d.v[ 1 ] );
      string s_y_str = buf;
      sprintf( buf, "%.2f", s_d.v[ 2 ] );
      string s_z_str = buf;
      write_to_panel( "cur_light_spot_cur_x", s_x_str );
      write_to_panel( "cur_light_spot_cur_y", s_y_str );
      write_to_panel( "cur_light_spot_cur_z", s_z_str );
    }
  }
}

/**
 * 'Level editor' mode helper method to clear and
 * repopulate the 'scripts' GUI panels to reflect
 * the currently-selected script on the currently-selected
 * game object, if any.
 */
void gui_t::update_script_subgui() {
  gui_panel* subgui_area =
    get_gui_panel( "cur_scripts_type_dep_area" );
  if ( !subgui_area ) { return; }

  script* cur_script = 0;
  if ( g->selected_unity ) {
    if ( g->selected_unity->scripts.size() > cur_script_page_index &&
         g->selected_unity->scripts[ cur_script_page_index ] ) {
      cur_script =
        g->selected_unity->scripts[ cur_script_page_index ];
    }
  }
  else {
    if ( g->g_scripts.size() > cur_script_page_index &&
         g->g_scripts[ cur_script_page_index ] ) {
      cur_script = g->g_scripts[ cur_script_page_index ];
    }
  }

  // Check any script-specific values.
  check_script_subgui_values( cur_script );

  // Clear out the previous contents of the GUI panel.
  for ( int i = 0; i < subgui_area->children.size(); ++i ) {
    if ( subgui_area->children[ i ] ) {
      delete subgui_area->children[ i ];
      subgui_area->children[ i ] = 0;
    }
  }
  subgui_area->children.clear();

  // Return early if no script is selected.
  if ( !cur_script || cur_script->type == "" ) {
    get_gui_panel( "cur_scripts_on_use_check" )->z_index = 0;
    return;
  }
  // Draw the 'on-use' checkbox if appropriate.
  if ( g->selected_unity &&
       cur_script == g->selected_unity->use_script ) {
    get_gui_panel( "cur_scripts_on_use_check" )->z_index = 2;
  }
  else {
    get_gui_panel( "cur_scripts_on_use_check" )->z_index = 0;
  }
  if ( cur_script->type == "s_test_use" ) {
    append_gui( "gui/subpanels/script_readable.json", subgui_area );
  }
}

/**
 * 'Level editor' mode helper method to check that values
 * entered for the given script are valid.
 * TODO: I'm pretty sure that's what it's supposed to do...?
 */
void gui_t::check_script_subgui_values( script* cur_script ) {
  if ( !cur_script || cur_script->type == "" ) { return; }
  if ( cur_script->type == "s_test_use" ) {
    gui_panel* SR_text_fn_entry =
      get_gui_panel( "SR_text_fn_entry" );
    if ( SR_text_fn_entry ) {
      string text_contents_path = SR_text_fn_entry->text_contents;
      ( ( s_test_use* )cur_script )->
        set_text_contents_from( text_contents_path.c_str() );
    }
  }
}

/**
 * Helper method to re-generate and draw the GUI buffer to
 * its OpenGL texture object.
 */
void gui_t::draw_to_texture() {
  if ( updated ) {
    glActiveTexture( GL_TEXTURE0 + gui_tex_index );
    glBindTexture( GL_TEXTURE_2D, gui_tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, cur_w, cur_h,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, gui_buffer );

    update_selected();
    update_gui_buffer();

    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_WRAP_S,
                     GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_WRAP_T,
                     GL_CLAMP_TO_BORDER );
    //glTexParameteri( GL_TEXTURE_2D,
    //                 GL_TEXTURE_MAG_FILTER,
    //                 GL_NEAREST );
    //glTexParameteri( GL_TEXTURE_2D,
    //                 GL_TEXTURE_MIN_FILTER,
    //                 GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_MAG_FILTER,
                     GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D,
                     GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_LINEAR );
    glGenerateMipmap( GL_TEXTURE_2D );

    // TODO: Un-comment this, right?
    //updated = false;
  }
}

/**
 * Draw the GUI object to its OpenGL texture.
 */
void gui_t::draw() {
  // Swap to the 2D GUI shader program.
  g->s_man->swap_shader( g->gui_shader_key );
  // Enable alpha blending, for transparency.
  glEnable( GL_BLEND );

  // Update the GUI texture and draw to it.
  draw_to_texture();

  // Set the GUI VAO and texture sampler.
  int tex_loc = glGetUniformLocation( g->s_man->cur_shader,
                                      "texture_sampler" );
  glUniform1i( tex_loc, gui_tex_index );
  glBindVertexArray( gui_vao );
  // Draw the GUI texture.
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glBindVertexArray( gui_vao );
  // Done; disable alpha blending.
  glDisable( GL_BLEND );
}

/**
 * Helper method to load and prepare the 'level editor' mode's GUI.
 */
void gui_t::load_editor_gui() {
  load_gui( "gui/editor.json" );

  // Add button actions.
  function<void( gui_panel* me )> clicky;

  gui_panel* add_unity_button = get_gui_panel( "add_unity_button" );
  clicky = []( gui_panel* me ) {
    camera* a_cam = g->c_man->active_camera;
    if ( !a_cam ) { return; }
    float offset = 0.5f;
    v4 c_fwd = a_cam->cam_rot.row( 2 ) * -1.0f;
    v3 c_pos = v3(-a_cam->cam_pos.v[ 0 ] + c_fwd.v[ 0 ] * offset,
            -a_cam->cam_pos.v[ 1 ] + c_fwd.v[ 1 ] * offset,
            -a_cam->cam_pos.v[ 2 ] + c_fwd.v[ 2 ] * offset);
    string unity_type =
      me->p_gui->get_gui_panel( "add_unity_textbox" )->text_contents;
    g->u_man->add_unity( unity_type, c_pos );
  };
  add_unity_button->make_button(clicky);

  gui_panel* clear_selection_button =
    get_gui_panel( "clear_selection_button" );
  clicky = []( gui_panel* me ) {
    g->selected_unity = 0;
    g->selected_light = 0;
    g->picked_body = 0;
    me->p_gui->updated = true;
  };
  clear_selection_button->make_button( clicky );

  gui_panel* cur_sel_pos_x_set_outline =
    get_gui_panel( "cur_sel_pos_x_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_pos_x =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_pos_x_entry" )->
          text_contents );
    if ( std::isnan( new_pos_x ) ) { printf( "p_x nan\n" ); return; }
    float pos_x_dif =
      ( new_pos_x - g->selected_unity->cur_center.v[ 0 ] );
    g->selected_unity->move( pos_x_dif, 0.0, 0.0 );
  };
  cur_sel_pos_x_set_outline->make_button( clicky );

  gui_panel* cur_sel_pos_y_set_outline =
    get_gui_panel( "cur_sel_pos_y_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_pos_y =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_pos_y_entry" )->
          text_contents );
    if ( std::isnan( new_pos_y ) ) { printf( "p_y nan\n" ); return; }
    float pos_y_dif =
      ( new_pos_y - g->selected_unity->cur_center.v[ 1 ] );
    g->selected_unity->move( 0.0, pos_y_dif, 0.0 );
  };
  cur_sel_pos_y_set_outline->make_button( clicky );

  gui_panel* cur_sel_pos_z_set_outline =
    get_gui_panel( "cur_sel_pos_z_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_pos_z =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_pos_z_entry" )->
          text_contents );
    if ( std::isnan( new_pos_z ) ) { printf( "p_z nan\n" ); return; }
    float pos_z_dif =
      ( new_pos_z - g->selected_unity->cur_center.v[ 2 ] );
    g->selected_unity->move( 0.0, 0.0, pos_z_dif );
  };
  cur_sel_pos_z_set_outline->make_button(clicky);

  // Clicking the X / Y / Z rotation labels will snap
  // the selected object to rotate along the clicked axis.
  gui_panel* cur_sel_rot_x_label =
    get_gui_panel( "cur_sel_rot_x_label" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    v3 new_axis = v3( 1.0f, 0.0f, 0.0f );
    quat cur_rot = g->selected_unity->rot;
    quat new_rot_quat = set_quat( 0.0f, new_axis );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_x_label->make_button( clicky );

  gui_panel* cur_sel_rot_y_label =
    get_gui_panel( "cur_sel_rot_y_label" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    v3 new_axis = v3( 0.0f, 1.0f, 0.0f );
    quat cur_rot = g->selected_unity->rot;
    quat new_rot_quat = set_quat( 0.0f, new_axis );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_y_label->make_button( clicky );

  gui_panel* cur_sel_rot_z_label =
    get_gui_panel( "cur_sel_rot_z_label" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    v3 new_axis = v3( 0.0f, 0.0f, 1.0f );
    quat cur_rot = g->selected_unity->rot;
    quat new_rot_quat = set_quat( 0.0f, new_axis );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_z_label->make_button( clicky );

  // Clicking the 'T' rotation label rotates 'by' instead of 'to'.
  gui_panel* cur_sel_rot_t_label =
    get_gui_panel( "cur_sel_rot_t_label" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float ax =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_x_entry" )->
          text_contents );
    if ( std::isnan( ax ) ) { printf( "r_x nan\n" ); return; }
    float ay =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_y_entry" )->
          text_contents );
    if ( std::isnan( ay ) ) { printf( "r_y nan\n" ); return; }
    float az =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_z_entry" )->
          text_contents );
    if ( std::isnan( az ) ) { printf( "r_z nan\n" ); return; }
    float dt =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_t_entry" )->
          text_contents );
    if ( std::isnan( dt ) ) { printf( "r_t nan\n" ); return; }
    quat rot_by = set_quat( dt, ax, ay, az );
    g->selected_unity->rotate_by( rot_by );
  };
  cur_sel_rot_t_label->make_button( clicky );

  gui_panel* cur_sel_rot_x_set_outline =
    get_gui_panel( "cur_sel_rot_x_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_rot_x =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_x_entry" )->
          text_contents );
    if ( std::isnan( new_rot_x ) ) { printf( "r_x nan\n" ); return; }
    quat cur_rot = g->selected_unity->rot;
    v3 new_axis = normalize( 
      v3( new_rot_x, cur_rot.r[ 2 ], cur_rot.r[ 3 ] ) );
    quat new_rot_quat = quat( cur_rot.r[0],
                              new_axis.v[0],
                              new_axis.v[1],
                              new_axis.v[2] );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_x_set_outline->make_button( clicky );

  gui_panel* cur_sel_rot_y_set_outline =
    get_gui_panel( "cur_sel_rot_y_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_rot_y =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_y_entry" )->
          text_contents );
    if ( std::isnan( new_rot_y ) ) { printf( "r_y nan\n" ); return; }
    quat cur_rot = g->selected_unity->rot;
    v3 new_axis = normalize(
      v3( cur_rot.r[ 1 ], new_rot_y, cur_rot.r[ 3 ] ) );
    quat new_rot_quat = quat( cur_rot.r[0],
                              new_axis.v[0],
                              new_axis.v[1],
                              new_axis.v[2] );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_y_set_outline->make_button( clicky );

  gui_panel* cur_sel_rot_z_set_outline =
    get_gui_panel( "cur_sel_rot_z_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_rot_z =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_z_entry" )->
          text_contents );
    if ( std::isnan( new_rot_z ) ) { printf( "r_z nan\n" ); return; }
    quat cur_rot = g->selected_unity->rot;
    v3 new_axis = normalize(
      v3( cur_rot.r[ 1 ], cur_rot.r[ 2 ], new_rot_z ) );
    quat new_rot_quat = quat( cur_rot.r[ 0 ],
                              new_axis.v[ 0 ],
                              new_axis.v[ 1 ],
                              new_axis.v[ 2 ] );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_z_set_outline->make_button( clicky );

  gui_panel* cur_sel_rot_t_set_outline =
    get_gui_panel( "cur_sel_rot_t_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_rot_t =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_rot_t_entry" )->
          text_contents );
    if ( std::isnan( new_rot_t ) ) { printf( "r_t nan\n" ); return; }
    quat cur_rot = g->selected_unity->rot;
    quat new_rot_quat = quat( new_rot_t,
                              cur_rot.r[ 1 ],
                              cur_rot.r[ 2 ],
                              cur_rot.r[ 3 ] );
    g->selected_unity->set_rotation( new_rot_quat );
  };
  cur_sel_rot_t_set_outline->make_button( clicky );

  gui_panel* cur_sel_scale_x_set_outline =
    get_gui_panel( "cur_sel_scale_x_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_scale_x =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_scale_x_entry" )->
          text_contents );
    if ( std::isnan( new_scale_x ) ) {
      printf( "s_x nan\n" );
      return;
    }
    v3 new_scale = g->selected_unity->cur_scale;
    new_scale.v[ 0 ] = new_scale_x;
    g->selected_unity->scale( new_scale );
  };
  cur_sel_scale_x_set_outline->make_button( clicky );

  gui_panel* cur_sel_scale_y_set_outline =
    get_gui_panel( "cur_sel_scale_y_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_scale_y =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_scale_y_entry" )->
          text_contents );
    if ( std::isnan( new_scale_y ) ) {
      printf( "s_y nan\n" );
      return;
    }
    v3 new_scale = g->selected_unity->cur_scale;
    new_scale.v[ 1 ] = new_scale_y;
    g->selected_unity->scale( new_scale );
  };
  cur_sel_scale_y_set_outline->make_button( clicky );

  gui_panel* cur_sel_scale_z_set_outline =
    get_gui_panel( "cur_sel_scale_z_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    float new_scale_z =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_sel_scale_z_entry" )->
          text_contents );
    if ( std::isnan( new_scale_z ) ) {
      printf( "s_z nan\n" );
      return;
    }
    v3 new_scale = g->selected_unity->cur_scale;
    new_scale.v[ 2 ] = new_scale_z;
    g->selected_unity->scale( new_scale );
  };
  cur_sel_scale_z_set_outline->make_button( clicky );

  gui_panel* cur_sel_name_set_outline =
    get_gui_panel( "cur_sel_name_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    g->selected_unity->name =
      me->p_gui->get_gui_panel( "cur_sel_name_text_box" )->
        text_contents;
  };
  cur_sel_name_set_outline->make_button( clicky );

  gui_panel* cur_sel_delete_outline =
    get_gui_panel( "cur_sel_delete_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_unity ) { return; }
    g->u_man->evict_unity( g->selected_unity );
  };
  cur_sel_delete_outline->make_button( clicky );

  // Light panel.
  gui_panel* cur_light_pos_x_set_outline =
    get_gui_panel( "cur_light_pos_x_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_pos_x =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_pos_x_entry" )->
          text_contents );
    if ( std::isnan( new_pos_x ) ) { printf( "l_x nan\n" ); return; }
    g->selected_light->pos.v[ 0 ] = new_pos_x;
    if ( g->selected_light->indicator ) {
      float pos_x_dif =
        new_pos_x - g->selected_light->indicator->cur_center.v[ 0 ];
      g->selected_light->indicator->move( pos_x_dif, 0.0, 0.0 );
    }
  };
  cur_light_pos_x_set_outline->make_button( clicky );

  gui_panel* cur_light_pos_y_set_outline =
    get_gui_panel( "cur_light_pos_y_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_pos_y =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_pos_y_entry" )->
          text_contents );
    if ( std::isnan( new_pos_y ) ) { printf( "l_y nan\n" ); return; }
    g->selected_light->pos.v[ 1 ] = new_pos_y;
    if ( g->selected_light->indicator ) {
      float pos_y_dif =
        new_pos_y - g->selected_light->indicator->cur_center.v[ 1 ];
      g->selected_light->indicator->move( 0.0, pos_y_dif, 0.0 );
    }
  };
  cur_light_pos_y_set_outline->make_button( clicky );

  gui_panel* cur_light_pos_z_set_outline =
    get_gui_panel( "cur_light_pos_z_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_pos_z =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_pos_z_entry" )->
          text_contents );
    if ( std::isnan( new_pos_z ) ) { printf( "l_z nan\n" ); return; }
    g->selected_light->pos.v[ 2 ] = new_pos_z;
    if ( g->selected_light->indicator ) {
      float pos_z_dif =
        new_pos_z - g->selected_light->indicator->cur_center.v[ 2 ];
      g->selected_light->indicator->move( 0.0, 0.0, pos_z_dif );
    }
  };
  cur_light_pos_z_set_outline->make_button( clicky );

  gui_panel* cur_light_amb_set_outline =
    get_gui_panel( "cur_light_amb_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_amb_r =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_R_amb_entry" )->
          text_contents );
    if ( std::isnan( new_amb_r ) ) { return; }
    float new_amb_g =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_G_amb_entry" )->
          text_contents );
    if ( std::isnan( new_amb_g ) ) { return; }
    float new_amb_b =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_B_amb_entry" )->
          text_contents );
    if ( std::isnan( new_amb_b ) ) { return; }
    v4 new_amb = v4( new_amb_r, new_amb_g, new_amb_b, 1.0f );
    g->selected_light->a = new_amb;
  };
  cur_light_amb_set_outline->make_button( clicky );

  gui_panel* cur_light_diff_set_outline =
    get_gui_panel( "cur_light_diff_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_diff_r =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_R_diff_entry" )->
          text_contents );
    if ( std::isnan( new_diff_r ) ) { return; }
    float new_diff_g =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_G_diff_entry" )->
          text_contents );
    if ( std::isnan( new_diff_g ) ) { return; }
    float new_diff_b =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_B_diff_entry" )->
          text_contents );
    if ( std::isnan( new_diff_b ) ) { return; }
    v4 new_diff = v4( new_diff_r, new_diff_g, new_diff_b, 1.0f );
    g->selected_light->d = new_diff;
  };
  cur_light_diff_set_outline->make_button( clicky );

  gui_panel* cur_light_spec_set_outline =
    get_gui_panel( "cur_light_spec_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_spec_r =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_R_spec_entry" )->
          text_contents );
    if ( std::isnan( new_spec_r ) ) { return; }
    float new_spec_g =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_G_spec_entry" )->
          text_contents );
    if ( std::isnan( new_spec_g ) ) { return; }
    float new_spec_b =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_B_spec_entry" )->
          text_contents );
    if ( std::isnan( new_spec_b ) ) { return; }
    v4 new_spec = v4( new_spec_r, new_spec_g, new_spec_b, 1.0f );
    g->selected_light->s = new_spec;
  };
  cur_light_spec_set_outline->make_button( clicky );

  gui_panel* cur_light_spec_exp_set_outline =
    get_gui_panel( "cur_light_spec_exp_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_spec_exp =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_spec_exp_entry" )->
          text_contents );
    if ( std::isnan( new_spec_exp ) ) { return; }
    g->selected_light->specular_exp = new_spec_exp;
  };
  cur_light_spec_exp_set_outline->make_button( clicky );

  gui_panel* cur_light_falloff_set_outline =
    get_gui_panel( "cur_light_falloff_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_falloff =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_falloff_entry" )->
          text_contents );
    if ( std::isnan( new_falloff ) ) { return; }
    g->selected_light->falloff = new_falloff;
  };
  cur_light_falloff_set_outline->make_button( clicky );

  gui_panel* cur_light_add_outline =
    get_gui_panel( "cur_light_add_outline" );
  clicky = []( gui_panel* me ) {
    camera* a_cam = g->c_man->active_camera;
    if ( !a_cam ) { return; }
    float offset = 0.5f;
    v4 c_fwd = a_cam->cam_rot.row( 2 ) * -1.0f;
    v4 pl_pos = v4( -a_cam->cam_pos.v[ 0 ] + c_fwd.v[ 0 ] * offset,
                    -a_cam->cam_pos.v[ 1 ] + c_fwd.v[ 1 ] * offset,
                    -a_cam->cam_pos.v[ 2 ] + c_fwd.v[ 2 ] * offset,
                    1.0f);
    phong_light* pl = new phong_light( pl_pos,
                                       v4( 0.1f, 0.1f, 0.1f, 1.0f ),
                                       v4( 0, 0, 0, 1 ),
                                       v4( 0, 0, 0, 1 ) );
    pl->indicator = g->u_man->add_unity( "u_light_ind",
                                         v3( pl_pos ) );
    g->l_man->add_phong_light( pl );
    g->selected_light = pl;
  };
  cur_light_add_outline->make_button( clicky );

  gui_panel* cur_light_del_outline =
    get_gui_panel( "cur_light_del_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    g->l_man->evict_phong_light( g->selected_light );
    g->selected_light = 0;
    g->g_man->update_selected();
  };
  cur_light_del_outline->make_button( clicky );

  gui_panel* cur_light_type_checkbox_pt =
    get_gui_panel( "cur_light_type_checkbox_pt" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    me->p_gui->get_gui_panel( "cur_light_type_check_pt" )->
      z_index = 2;
    me->p_gui->get_gui_panel( "cur_light_type_check_spot" )->
      z_index = 0;
    g->selected_light->type = BRLA_LIGHT_PHONG_POINT;
  };
  cur_light_type_checkbox_pt->make_button( clicky );

  gui_panel* cur_light_type_checkbox_spot =
    get_gui_panel( "cur_light_type_checkbox_spot" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    me->p_gui->get_gui_panel( "cur_light_type_check_pt" )->
      z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_type_check_spot" )->
      z_index = 2;
    g->selected_light->type = BRLA_LIGHT_PHONG_SPOT;
  };
  cur_light_type_checkbox_spot->make_button( clicky );

  gui_panel* cur_light_spot_dir_x_set_outline =
    get_gui_panel( "cur_light_spot_dir_x_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_dir_x =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_spot_dir_x_entry" )->
          text_contents );
    if ( std::isnan( new_dir_x ) ) { return; }
    g->selected_light->dir.v[ 0 ] = new_dir_x;
  };
  cur_light_spot_dir_x_set_outline->make_button( clicky );

  gui_panel* cur_light_spot_dir_y_set_outline =
    get_gui_panel( "cur_light_spot_dir_y_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_dir_y =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_spot_dir_y_entry" )->
          text_contents );
    if ( std::isnan( new_dir_y ) ) { return; }
    g->selected_light->dir.v[ 1 ] = new_dir_y;
  };
  cur_light_spot_dir_y_set_outline->make_button( clicky );

  gui_panel* cur_light_spot_dir_z_set_outline =
    get_gui_panel( "cur_light_spot_dir_z_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_dir_z =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_spot_dir_z_entry" )->
          text_contents );
    if ( std::isnan( new_dir_z ) ) { return; }
    g->selected_light->dir.v[ 2 ] = new_dir_z;
  };
  cur_light_spot_dir_z_set_outline->make_button( clicky );

  gui_panel* cur_light_spot_angle_set_outline =
    get_gui_panel( "cur_light_spot_angle_set_outline" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    float new_angle =
      str_to_f(
        me->p_gui->get_gui_panel( "cur_light_spot_angle_entry" )->
          text_contents );
    if ( std::isnan( new_angle ) ) { return; }
    g->selected_light->spot_rads = ang_to_rad( new_angle );
  };
  cur_light_spot_angle_set_outline->make_button( clicky );

  gui_panel* cur_light_shadow_caster_checkbox =
    get_gui_panel( "cur_light_shadow_caster_checkbox" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    phong_light* li = g->selected_light;
    if ( !li->cast_shadows ) {
      li->cast_shadows = true;
      li->make_shadow_caster();
      me->p_gui->get_gui_panel( "cur_light_shadow_caster_check" )->
        z_index = 2;
    }
    else {
      li->cast_shadows = false;
      me->p_gui->get_gui_panel( "cur_light_shadow_caster_check" )->
        z_index = 0;
    }
  };
  cur_light_shadow_caster_checkbox->make_button( clicky );

  gui_panel* cur_light_shadow_caster_check =
    get_gui_panel( "cur_light_shadow_caster_check" );
  clicky = []( gui_panel* me ) {
    if ( !g->selected_light ) { return; }
    phong_light* li = g->selected_light;
    if ( li->cast_shadows ) {
      li->cast_shadows = false;
      me->p_gui->get_gui_panel( "cur_light_shadow_caster_check" )->
        z_index = 0;
    }
  };
  cur_light_shadow_caster_check->make_button( clicky );

  gui_panel* file_subpanel_save_outline =
    get_gui_panel( "file_subpanel_save_outline" );
  clicky = []( gui_panel* me ) {
    json saved_scene;
    json unities = json::array();
    function<void( unity* u )> f =
      [ &unities = unities ]( unity* u ) {
      json saved_unity;
      saved_unity[ "type" ] = u->type;
      saved_unity[ "name" ] = u->name;
      v3 u_center = u->cur_center;
      saved_unity[ "pos_x" ] = u_center.v[ 0 ];
      saved_unity[ "pos_y" ] = u_center.v[ 1 ];
      saved_unity[ "pos_z" ] = u_center.v[ 2 ];
      quat u_rot = u->rot;
      saved_unity[ "rot_x" ] = u_rot.r[ 1 ];
      saved_unity[ "rot_y" ] = u_rot.r[ 2 ];
      saved_unity[ "rot_z" ] = u_rot.r[ 3 ];
      saved_unity[ "rot_t" ] = u_rot.r[ 0 ];
      v3 u_scale = u->cur_scale;
      saved_unity[ "scale_x" ] = u_scale.v[ 0 ];
      saved_unity[ "scale_y" ] = u_scale.v[ 1 ];
      saved_unity[ "scale_z" ] = u_scale.v[ 2 ];

      json u_scripts = json::array();
      for ( int j = 0; j < u->scripts.size(); ++j ) {
        json u_script;
        u_script[ "type" ] = u->scripts[ j ]->type;
        if ( u->scripts[ j ]->type == "s_test_use" ) {
          s_test_use* st = ( s_test_use* )u->scripts[ j ];
          u_script[ "txt" ] = st->my_text;
        }
        if ( u->scripts[ j ] == u->use_script ) {
          u_script[ "on_use" ] = true;
        }
        u_scripts.push_back( u_script );
      }
      saved_unity[ "scripts" ] = u_scripts;

      unities.push_back( saved_unity );
    };
    g->u_man->for_each( f, true );
    saved_scene[ "unities" ] = unities;

    // Scripts.
    json global_scripts = json::array();
    for ( int i = 0; i < g->g_scripts.size(); ++i ) {
      json g_script;
      g_script[ "type" ] = g->g_scripts[ i ]->type;
      global_scripts.push_back( g_script );
    }
    saved_scene[ "scripts" ] = global_scripts;

    // Lights.
    json phong_lights = json::array();
    for ( int i = 0; i < g->l_man->phong_lights.size(); ++i ) {
      json p_light;
      phong_light* p_l = g->l_man->phong_lights[ i ];
      if ( p_l ) {
        p_light[ "l_type" ] = p_l->type;
        p_light[ "p_x" ] = p_l->pos.v[ 0 ];
        p_light[ "p_y" ] = p_l->pos.v[ 1 ];
        p_light[ "p_z" ] = p_l->pos.v[ 2 ];
        p_light[ "p_a" ] = p_l->pos.v[ 3 ];
        p_light[ "a_r" ] = p_l->a.v[ 0 ];
        p_light[ "a_g" ] = p_l->a.v[ 1 ];
        p_light[ "a_b" ] = p_l->a.v[ 2 ];
        p_light[ "a_a" ] = p_l->a.v[ 3 ];
        p_light[ "d_r" ] = p_l->d.v[ 0 ];
        p_light[ "d_g" ] = p_l->d.v[ 1 ];
        p_light[ "d_b" ] = p_l->d.v[ 2 ];
        p_light[ "d_a" ] = p_l->d.v[ 3 ];
        p_light[ "s_r" ] = p_l->s.v[ 0 ];
        p_light[ "s_g" ] = p_l->s.v[ 1 ];
        p_light[ "s_b" ] = p_l->s.v[ 2 ];
        p_light[ "s_a" ] = p_l->s.v[ 3 ];
        p_light[ "s_e" ] = p_l->specular_exp;
        p_light[ "f" ] = p_l->falloff;
        p_light[ "sp_dir_x" ] = p_l->dir.v[ 0 ];
        p_light[ "sp_dir_y" ] = p_l->dir.v[ 1 ];
        p_light[ "sp_dir_z" ] = p_l->dir.v[ 2 ];
        p_light[ "sp_dir_t" ] = p_l->spot_rads;
        phong_lights.push_back( p_light );
      }
    }
    saved_scene[ "phong_lights" ] = phong_lights;

    string save_filename =
      me->p_gui->get_gui_panel( "file_subpanel_entry" )->
      text_contents;
    if ( save_filename == "" ) {
      printf("Can't save - no filename\n");
      return;
    }
    save_filename = "save/" + save_filename;
    string saved_scene_dump = saved_scene.dump( 2 );
    write_to_file( saved_scene_dump.c_str(), save_filename.c_str() );
  };
  file_subpanel_save_outline->make_button( clicky );

  gui_panel* file_subpanel_load_outline =
    get_gui_panel( "file_subpanel_load_outline" );
  clicky = []( gui_panel* me ) {
    string load_filename =
      me->p_gui->get_gui_panel( "file_subpanel_entry" )->
      text_contents;
    if ( load_filename == "" ) {
      printf("Can't load - no filename\n");
      return;
    }
    load_filename = "save/" + load_filename;
    g->reload_file( load_filename );
  };
  file_subpanel_load_outline->make_button( clicky );

  // Right pane tab switchers.
  gui_panel* cur_sel_switcher = get_gui_panel( "cur_sel_switcher" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_sel_subpanel" )->z_index = 1;
    me->p_gui->get_gui_panel( "cur_scripts_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel2" )->z_index = 0;
  };
  cur_sel_switcher->make_button( clicky );

  gui_panel* cur_scripts_switcher =
    get_gui_panel( "cur_scripts_switcher" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_sel_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_scripts_subpanel" )->z_index = 1;
    me->p_gui->get_gui_panel( "cur_light_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel2" )->z_index = 0;
  };
  cur_scripts_switcher->make_button( clicky );

  gui_panel* cur_light_switcher =
    get_gui_panel( "cur_light_switcher" );
  clicky = [](gui_panel* me) {
    me->p_gui->get_gui_panel( "cur_sel_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_scripts_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel" )->z_index = 1;
    me->p_gui->get_gui_panel( "cur_light_subpanel2" )->z_index = 0;
  };
  cur_light_switcher->make_button( clicky );

  gui_panel* cur_light_switcher2 =
    get_gui_panel( "cur_light_switcher2" );
  clicky = [](gui_panel* me) {
    me->p_gui->get_gui_panel( "cur_sel_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_scripts_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel" )->z_index = 0;
    me->p_gui->get_gui_panel( "cur_light_subpanel2" )->z_index = 1;
  };
  cur_light_switcher2->make_button( clicky );

  // Scripts page buttons.
  // TODO: Ughhhh this is so boring and forever-taking...
  gui_panel* cur_scripts_page_0 =
    get_gui_panel( "cur_scripts_page_0" );
  clicky = []( gui_panel* me ) {
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 0;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_0->make_button( clicky );

  gui_panel* cur_scripts_page_1 =
    get_gui_panel( "cur_scripts_page_1" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 1;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_1->make_button( clicky );

  gui_panel* cur_scripts_page_2 =
    get_gui_panel( "cur_scripts_page_2" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 2;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_2->make_button( clicky );

  gui_panel* cur_scripts_page_3 =
    get_gui_panel( "cur_scripts_page_3" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 3;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_3->make_button( clicky );

  gui_panel* cur_scripts_page_4 =
    get_gui_panel( "cur_scripts_page_4" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 4;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_4->make_button( clicky );

  gui_panel* cur_scripts_page_5 =
    get_gui_panel( "cur_scripts_page_5" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 5;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_5->make_button( clicky );

  gui_panel* cur_scripts_page_6 =
    get_gui_panel( "cur_scripts_page_6" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 6;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_6->make_button( clicky );

  gui_panel* cur_scripts_page_7 =
    get_gui_panel( "cur_scripts_page_7" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 7;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_7->make_button( clicky );

  gui_panel* cur_scripts_page_8 =
    get_gui_panel( "cur_scripts_page_8" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_9" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->cur_script_page_index = 8;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_8->make_button( clicky );

  gui_panel* cur_scripts_page_9 =
    get_gui_panel( "cur_scripts_page_9" );
  clicky = []( gui_panel* me ) {
    me->p_gui->get_gui_panel( "cur_scripts_page_0" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_1" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_2" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_3" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_4" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_5" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_6" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_7" )->
      outline( 1, 255, 255, 255, 255 );
    me->p_gui->get_gui_panel( "cur_scripts_page_8" )->
      outline( 1, 255, 255, 255, 255 );
    me->outline( 1, 0, 255, 0, 255 );
    me->p_gui->cur_script_page_index = 9;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_page_9->make_button( clicky );

  gui_panel* cur_scripts_type_set_outline =
    get_gui_panel( "cur_scripts_type_set_outline" );
  clicky = []( gui_panel* me ) {
    g->add_script_to_selected(
      me->p_gui->get_gui_panel( "cur_scripts_type_entry" )->
        text_contents );
    me->p_gui->update_script_subgui();
  };
  cur_scripts_type_set_outline->make_button( clicky );

  gui_panel* cur_scripts_on_use_outline =
    get_gui_panel( "cur_scripts_on_use_outline" );
  clicky = []( gui_panel* me ) {
    int cur_ind = me->p_gui->cur_script_page_index;
    script* cur_script = 0;
    if ( g->selected_unity ) {
      if ( g->selected_unity->scripts.size() > cur_ind &&
           g->selected_unity->scripts[ cur_ind ] ) {
        cur_script = g->selected_unity->scripts[ cur_ind ];
      }
    }
    if ( !cur_script ) { return; }
    g->selected_unity->use_script = cur_script;
    me->p_gui->update_script_subgui();
  };
  cur_scripts_on_use_outline->make_button( clicky );

  gui_panel* cur_scripts_apply_outline =
    get_gui_panel( "cur_scripts_apply_outline" );
  clicky = []( gui_panel* me ) {
    me->p_gui->update_script_subgui();
  };
  cur_scripts_apply_outline->make_button( clicky );

  gui_panel* cur_scripts_delete_selected_outline =
    get_gui_panel( "cur_scripts_delete_selected_outline" );
  clicky = []( gui_panel* me ) {
    script* cur_script = 0;
    int cur_ind = me->p_gui->cur_script_page_index;
    if ( g->selected_unity &&
         g->selected_unity->scripts.size() > cur_ind ) {
      cur_script = g->selected_unity->scripts[ cur_ind ];
      if ( cur_script == g->selected_unity->use_script ) {
        g->selected_unity->use_script = 0;
      }
      delete cur_script;
      g->selected_unity->scripts.erase(
        g->selected_unity->scripts.begin() + cur_ind );
    }
    else if ( !g->selected_unity && g->g_scripts.size() > cur_ind ) {
      cur_script = g->g_scripts[ cur_ind ];
      delete cur_script;
      g->g_scripts.erase( g->g_scripts.begin() + cur_ind );
    }
    if ( !cur_script ) { return; }

    me->p_gui->update_script_subgui();
  };
  cur_scripts_delete_selected_outline->make_button( clicky );
}

/** Load a GUI tree from a file with the given path. */
void gui_t::load_gui( string fn ) {
  string loaded_file = read_from_file( fn.c_str() );
  json j = json::parse( loaded_file );

  root = new gui_panel( "root", BRLA_GUI_X_LEFT, BRLA_GUI_Y_TOP,
                        0, 0, cur_w, cur_h, 0, this );

  for ( int i = 0; i < ( int )j[ "panels" ].size(); ++i ) {
    load_gui_panel( j[ "panels" ][ i ], root );
  }
}

/**
 * Load a GUI tree from a file with the given path,
 * and append it to the given parent GUI panel. But
 * be careful; panel names should be unique, so it's
 * not a good idea to use this to create many similar
 * GUIs from the same file.
 */
void gui_t::append_gui( const char* p_fn, gui_panel* p_parent ) {
  string loaded_file = read_from_file( p_fn );
  json j = json::parse( loaded_file );

  for ( int i = 0; i < ( int )j[ "panels" ].size(); ++i ) {
    int x_o = p_parent->vx;
    int y_o = p_parent->vy;
    load_gui_panel( j[ "panels" ][ i ], p_parent, x_o, y_o, true );
  }
}

/**
 * Wrapper method to load a GUI panel from JSON at coordinate
 * (0, 0) with no offset.
 */
void gui_t::load_gui_panel( json p, gui_panel* parent ) {
  load_gui_panel( p, parent, 0, 0, false );
}

/**
 * Populate a GUI panel from a JSON struct describing it.
 * TODO: Comment this method.
 */
void gui_t::load_gui_panel( json p, gui_panel* parent,
                            int x_offset, int y_offset,
                            bool use_offset ) {
  string n = p[ "name" ];
  int p_x_anch = p[ "x_anchor" ];
  int p_y_anch = p[ "y_anchor" ];
  int p_x = p[ "x" ];
  int p_y = p[ "y" ];
  int p_w = p[ "w" ];
  int p_h = p[ "h" ];
  if ( use_offset ) {
    if ( p_x_anch == BRLA_GUI_X_RIGHT ) {
      p_x += ( g->g_win_w - x_offset ) - parent->vw;
    }
    else {
      p_x += x_offset;
    }
    if ( p_y_anch == BRLA_GUI_Y_BOTTOM ) {
      p_y += ( g->g_win_h - y_offset ) - parent->vh;
    }
    else {
      p_y += y_offset;
    }
  }

  gui_panel* new_panel = new gui_panel( p[ "name" ],
                                        p_x_anch,
                                        p_y_anch,
                                        p_x,
                                        p_y,
                                        p_w,
                                        p_h,
                                        parent,
                                        this );

  if ( p.find( "z" ) != p.end() ) {
    new_panel->z_index = p[ "z" ];
  }

  if ( p.find( "fill" ) != p.end() ) {
    json j_f = p[ "fill" ];
    new_panel->fill( j_f[ "x" ], j_f[ "y" ],
                     j_f[ "w" ], j_f[ "h" ],
                     j_f[ "r" ], j_f[ "g" ],
                     j_f[ "b" ], j_f[ "a" ] );
  }
  if ( p.find( "outline" ) != p.end() ) {
    json j_o = p[ "outline" ];
    new_panel->outline( j_o[ "o" ],
                        j_o[ "r" ],
                        j_o[ "g" ],
                        j_o[ "b" ],
                        j_o[ "a" ] );
  }
  if ( p.find( "text" ) != p.end() ) {
    json j_t = p[ "text" ];
    if ( j_t.find( "r" ) != j_t.end() &&
         j_t.find( "g" ) != j_t.end() &&
         j_t.find( "b" ) != j_t.end() &&
         j_t.find( "a" ) != j_t.end() ) {
      v4 text_col = v4( ( int )j_t[ "r" ],
                        ( int )j_t[ "g" ],
                        ( int )j_t[ "b" ],
                        ( int )j_t[ "a" ] );
      new_panel->text( j_t[ "x" ], j_t[ "y" ],
                       j_t[ "t" ], g->f_mono, text_col );
    }
    else {
      new_panel->text( j_t[ "x" ], j_t[ "y" ],
                       j_t[ "t" ], g->f_mono );
    }
  }
  else if ( p.find( "texts" ) != p.end() ) {
    for ( int i = 0; i < ( int )p[ "texts" ].size(); i++ ) {
      json j_t = p[ "texts" ][ i ];
      if ( j_t.find( "r" ) != j_t.end() &&
           j_t.find( "g" ) != j_t.end() &&
           j_t.find( "b" ) != j_t.end() &&
           j_t.find( "a" ) != j_t.end() ) {
        v4 text_col = v4( ( int )j_t[ "r" ],
                          ( int )j_t[ "g" ],
                          ( int )j_t[ "b" ],
                          ( int )j_t[ "a" ] );
        new_panel->text( j_t[ "x" ], j_t[ "y" ],
                         j_t[ "t" ], g->f_mono, text_col );
      }
      else {
        new_panel->text( j_t[ "x" ], j_t[ "y" ],
                         j_t[ "t" ], g->f_mono );
      }
    }
  }
  if ( p.find( "textarea" ) != p.end() ) {
    new_panel->make_text_entry();
  }

  if ( p.find( "panels" ) != p.end() ) {
    for ( int i = 0; i < ( int )p[ "panels" ].size(); i++ ) {
      if ( use_offset ) {
        load_gui_panel( p[ "panels" ][ i ], new_panel,
                        x_offset, y_offset, true );
      }
      else {
        load_gui_panel( p[ "panels" ][ i ], new_panel );
      }
    }
  }
}

/** Helper method to write a string of text to a GUI panel. */
void gui_t::write_to_panel( string panel_name, string panel_text ) {
  gui_panel* panel = get_gui_panel( panel_name );
  if ( panel ) {
    panel->empty_gui_buffer();
    panel->text( 0, 0, panel_text, g->f_mono );
  }
  else {
    printf( "Could not find panel %s\n", panel_name.c_str() );
  }
  updated = true;
}

/** Helper method to find a GUI panel by name. */
gui_panel* gui_t::get_gui_panel( string name ) {
  if ( root ) { return root->find_panel( name ); }
  return 0;
}

/**
 * 'GUI manager' constructor. Initialize the main GUI, and load
 * the 'level editor' GUI panels if that mode is enabled.
 */
gui_manager::gui_manager() {
  // Create the main GUI and add it to the GUI map.
  cur_gui = new gui_t( g->g_win_w, g->g_win_h );
  add_gui( "main", cur_gui );
  // Load the 'level editor' GUI if applicable.
  if ( g->editor ) { cur_gui->load_editor_gui(); }
  // Otherwise, load a blank GUI.
  else {
    cur_gui->root = new gui_panel( "root",
                                   BRLA_GUI_X_LEFT,
                                   BRLA_GUI_Y_TOP,
                                   0,
                                   0,
                                   cur_gui->cur_w,
                                   cur_gui->cur_h,
                                   0,
                                   cur_gui );
  }
}

/** 'GUI manager' destructor; delete any active GUIS. */
gui_manager::~gui_manager() {
  for ( auto iter = guis.begin(); iter != guis.end(); ++iter ) {
    if ( iter->second ) {
      delete iter->second;
      iter->second = 0;
    }
  }
}

/** 'Window resize' callback; resize the currently-active GUI. */
void gui_manager::window_resize_update() {
  if ( cur_gui ) { cur_gui->resize( g->g_win_w, g->g_win_h ); }
}

/**
 * 'Mouse click' callback; check if any panels use the click,
 * and return true if they do / false if they don't.
 */
bool gui_manager::mouse_click_update( int m_x, int m_y ) {
  // Check if the current GUI can use the mouse click.
  if ( cur_gui ) {
    return cur_gui->mouse_click( m_x, m_y );
  }

  // If not, set the mouse ray position / direction
  // in the 3D game world.
  v3 c_pos = g->c_man->active_camera->cam_pos * -1.0f;
  v3 m_dir = g->mouse_ray;
  // TODO: Is this camera switch necessary?
  g->c_man->switch_camera(g->cam_normal);

  // Return false to indicate the GUI didn't use the click.
  return false;
}

/** 'Keyboard press' GUI callback. */
void gui_manager::key_press( const char c ) {
  if ( cur_gui ) { cur_gui->key_press( c ); }
}

/** Helper method to call 'update_selected' on the current GUI. */
void gui_manager::update_selected() {
  if (cur_gui) { cur_gui->update_selected(); }
}

/** Add a GUI to the map of active ones. */
void gui_manager::add_gui( string key, gui_t* gui ) {
  evict_gui( key );
  guis[ key ] = gui;
}

/** Delete a GUI from the map of active ones, and free its memory. */
void gui_manager::evict_gui( string key ) {
  auto gui_iter = guis.find( key );
  if ( gui_iter != guis.end() ) {
    if ( gui_iter->second ) {
      delete gui_iter->second;
      gui_iter->second = 0;
      guis.erase( key );
    }
  }
}

/**
 * Retrieve a GUI object from the map of active ones, by name.
 * Return 0 if no GUI object can be found.
 */
gui_t* gui_manager::get_gui( string key ) {
  auto gui_iter = guis.find( key );
  if ( gui_iter != guis.end() ) { return gui_iter->second; }
  return 0;
}

/**
 * Swap the active GUI object to a new one with the given name.
 * If no GUI object could be found to swap to, do nothing.
 */
void gui_manager::swap_gui( string key ) {
  // Try to find the requested GUI.
  gui_t* new_gui = get_gui( key );
  if ( new_gui ) {
    // Swap GUIs if one was found.
    cur_gui = new_gui;
    // Ensure that the new GUI is sized to the window.
    window_resize_update();
  }
}

/**
 * Clear the array of available GUI objects, and free their memory.
 */
void gui_manager::clear_guis() {
  for ( auto gui_iter = guis.begin();
        gui_iter != guis.end();
        ++gui_iter ) {
    if ( gui_iter->second ) {
      delete gui_iter->second;
      gui_iter->second = 0;
    }
  }
}

/**
 * 'Update' game loop step for the GUI manager.
 * Currently does nothing; most UI interaction occurs in callbacks.
 */
void gui_manager::update() {}

/**
 * 'Draw' game loop step for the GUI manager.
 * Draw the OpenGL texture for the current GUI object.
 */
void gui_manager::draw() {
  if ( cur_gui ) { cur_gui->draw(); }
}
