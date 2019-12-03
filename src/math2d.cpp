#include "math2d.h"

/**
 * Flip a texture across its centerline, vertically.
 *   tex_data: Texture data; poitner to x * y * n bytes.
 *          x: X-dimension of the texture.
 *          y: Y-dimension of the texture.
 *          n: # of channels, i.e. 4 for RGBA.
 */
void flip_tex_V( unsigned char* tex_data, int x, int y, int n ) {
  // Use a buffer. We could swap with XOR, but it'd be awful to debug
  // and this memory is freed quickly.
  char temp_row;

  for ( int i = 0; i < y / 2; i++ ) {
    // Pointers to the top and bottom rows.
    unsigned char* top = tex_data + ( sizeof( char ) * x * i * n );
    unsigned char* bottom = tex_data +
      ( sizeof( char ) * x * ( y -( i + 1 ) ) * n );

    // Account for odd y-dimension, but images -should- have y%2==0.
    if ( top == bottom ) { break; }
    // top -> temp, bottom -> top, temp -> bottom
    for ( int j = 0; j < x * n; j++ ) {
      temp_row = *top;
      *top = *bottom;
      *bottom = temp_row;
      top++;
      bottom++;
    }
  }
}

/**
 * Helper method to flip a Y-coordinate in a texture, given
 * the texture's height. It's amazing how often things end up
 * backwards or upside-down with 3D math.
 */
int flip_y( int p_y, int h ) {
  // Return sensible values if the input is out-of-bounds.
  if      ( p_y > h )     { return 0; }
  else if ( p_y < 0 )     { return h; }
  // Calculate the y-coordinate mirrored across the 'X-axis'.
  else if ( p_y < h / 2 ) { return ( ( h - 1 ) - p_y ); }
  else if ( p_y > h / 2 ) { return ( p_y - ( p_y - h / 2 ) * 2 ); }
  // If the y-coordinate equals half the texture height, return that.
  return p_y;
}
