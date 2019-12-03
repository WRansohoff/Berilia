#include "math3d.h"

/**
 * Default (empty) 3-vector constructor.
 * Initializes all three elements to zero.
 */
v3::v3() { v[ 0 ] = v[ 1 ] = v[ 2 ] = 0; }

/** 3-vector constructor. Initialize with given X / Y / Z values. */
v3::v3( float x, float y, float z ) {
  v[ 0 ] = x;
  v[ 1 ] = y;
  v[ 2 ] = z;
}

/**
 * 3-vector copy constructor.
 * Initialize with X / Y / Z values copied from another 3-vector.
 */
v3::v3( const v3& other ) {
  v[ 0 ] = other.v[ 0 ];
  v[ 1 ] = other.v[ 1 ];
  v[ 2 ] = other.v[ 2 ];
}

/**
 * 3-vector constructor from a 4-vector.
 * Copies the first 3 elements and discards the 4th. */
v3::v3( const v4& other ) {
  v[ 0 ] = other.v[ 0 ];
  v[ 1 ] = other.v[ 1 ];
  v[ 2 ] = other.v[ 2 ];
}

/**
 * Addition operator for two 3-vectors.
 * Sum each element from each vector.
 */
v3 v3::operator+( const v3& other ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] + other.v[ 0 ];
  vec.v[ 1 ] = v[ 1 ] + other.v[ 1 ];
  vec.v[ 2 ] = v[ 2 ] + other.v[ 2 ];
  return vec;
}

/**
 * Addition operator for a 3-vector and a scalar.
 * Add the scalar to each element of the vector.
 */
v3 v3::operator+( float scalar ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] + scalar;
  vec.v[ 1 ] = v[ 1 ] + scalar;
  vec.v[ 2 ] = v[ 2 ] + scalar;
  return vec;
}

/**
 * '+=' operator for adding two 3-vectors.
 * Same logic as the addition operator.
 */
v3& v3::operator+=( const v3& other ) {
  v[ 0 ] = v[ 0 ] + other.v[ 0 ];
  v[ 1 ] = v[ 1 ] + other.v[ 1 ];
  v[ 2 ] = v[ 2 ] + other.v[ 2 ];
  return *this;
}

/**
 * '+=' operator for adding a scalar to a 3-vector.
 * Same logic as the addition operator.
 */
v3& v3::operator+=( float scalar ) {
  v[ 0 ] = v[ 0 ] + scalar;
  v[ 1 ] = v[ 1 ] + scalar;
  v[ 2 ] = v[ 2 ] + scalar;
  return *this;
}

/**
 * Subtraction operator for two 3-vectors.
 * Subtract each element from each vector.
 */
v3 v3::operator-( const v3& other ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] - other.v[ 0 ];
  vec.v[ 1 ] = v[ 1 ] - other.v[ 1 ];
  vec.v[ 2 ] = v[ 2 ] - other.v[ 2 ];
  return vec;
}

/**
 * Subtraction operator for a 3-vector and a scalar.
 * Subtract the scalar from each element of the vector.
 */
v3 v3::operator-( float scalar ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] - scalar;
  vec.v[ 1 ] = v[ 1 ] - scalar;
  vec.v[ 2 ] = v[ 2 ] - scalar;
  return vec;
}

/**
 * '-=' operator for subtracting one 3-vector from another.
 * Same logic as the subtraction operator.
 */
v3& v3::operator-=( const v3& other ) {
  v[ 0 ] = v[ 0 ] - other.v[ 0 ];
  v[ 1 ] = v[ 1 ] - other.v[ 1 ];
  v[ 2 ] = v[ 2 ] - other.v[ 2 ];
  return *this;
}

/**
 * '-=' operator for subtracting a scalar from a 3-vector.
 * Same logic as the subtracting operator.
 */
v3& v3::operator-=( float scalar ) {
  v[ 0 ] = v[ 0 ] - scalar;
  v[ 1 ] = v[ 1 ] - scalar;
  v[ 2 ] = v[ 2 ] - scalar;
  return *this;
}

/**
 * Multiplication operator for a 3-vector and a scalar.
 * Multiply each element of the vector by the scalar.
 */
v3 v3::operator*( float scalar ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] * scalar;
  vec.v[ 1 ] = v[ 1 ] * scalar;
  vec.v[ 2 ] = v[ 2 ] * scalar;
  return vec;
}

/**
 * '*=' operator for multiplying a 3-vector by a scalar.
 * Same logic as the multiplication operator.
 */
v3& v3::operator*=( float scalar ) {
  v[ 0 ] = v[ 0 ] * scalar;
  v[ 1 ] = v[ 1 ] * scalar;
  v[ 2 ] = v[ 2 ] * scalar;
  return *this;
}

/**
 * Division operator for dividing two 3-vectors.
 * Divide each element individually.
 */
v3 v3::operator/( v3 other ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] / other.v[ 0 ];
  vec.v[ 1 ] = v[ 1 ] / other.v[ 1 ];
  vec.v[ 2 ] = v[ 2 ] / other.v[ 2 ];
  return vec;
}

/**
 * Division operator for dividing a 3-vector by a scalar.
 * Divide each element by the given scalar value.
 */
v3 v3::operator/( float scalar ) {
  v3 vec;
  vec.v[ 0 ] = v[ 0 ] / scalar;
  vec.v[ 1 ] = v[ 1 ] / scalar;
  vec.v[ 2 ] = v[ 2 ] / scalar;
  return vec;
}

/**
 * '/=' operator for dividing a 3-vector by a scalar.
 * Same logic as the division operator.
 */
v3& v3::operator/=( float scalar ) {
  v[ 0 ] = v[ 0 ] / scalar;
  v[ 1 ] = v[ 1 ] / scalar;
  v[ 2 ] = v[ 2 ] / scalar;
  return *this;
}

/**
 * '/=' operator for dividing two 3-vectors.
 * Same logic as the division operator.
 */
v3& v3::operator/=( v3 other ) {
  v[ 0 ] = v[ 0 ] / other.v[ 0 ];
  v[ 1 ] = v[ 1 ] / other.v[ 1 ];
  v[ 2 ] = v[ 2 ] / other.v[ 2 ];
  return *this;
}

/**
 * '=' operator to copy one 3-vector's contents into another.
 */
v3& v3::operator=( const v3& other ) {
  v[ 0 ] = other.v[ 0 ];
  v[ 1 ] = other.v[ 1 ];
  v[ 2 ] = other.v[ 2 ];
  return *this;
}

/**
 * '==' operator to check whether one 3-vector equals another one.
 */
bool v3::operator==( const v3& other ) const {
  return v[ 0 ] == other.v[ 0 ] &&
         v[ 1 ] == other.v[ 1 ] &&
         v[ 2 ] == other.v[ 2 ];
}

/** Default (empty) 4-vector constructor to set all elements to 0. */
v4::v4() { v[ 0 ] = v[ 1 ] = v[ 2 ] = v[ 3 ] = 0; }

/** 4-vector constructor to initialize elements to given values. */
v4::v4( float x, float y, float z, float w ) {
  v[ 0 ] = x;
  v[ 1 ] = y;
  v[ 2 ] = z;
  v[ 3 ] = w;
}

/** Constructor to create a 4-vector from a 3-vector and a float. */
v4::v4( const v3& other, float w ) {
  v[ 0 ] = other.v[ 0 ];
  v[ 1 ] = other.v[ 1 ];
  v[ 2 ] = other.v[ 2 ];
  v[ 3 ] = w;
}

/**
 * Subtraction operator for two 4-vectors.
 * Subtract each element individually.
 */
v4 v4::operator-( const v4& other ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] - other.v[ 0 ];
  vec.v[ 1 ] = v[ 1 ] - other.v[ 1 ];
  vec.v[ 2 ] = v[ 2 ] - other.v[ 2 ];
  vec.v[ 3 ] = v[ 3 ] - other.v[ 3 ];
  return vec;
}

/** Subtraction operator to subtract a scalar from a 4-vector. */
v4 v4::operator-( float scalar ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] - scalar;
  vec.v[ 1 ] = v[ 1 ] - scalar;
  vec.v[ 2 ] = v[ 2 ] - scalar;
  vec.v[ 3 ] = v[ 3 ] - scalar;
  return vec;
}

/**
 * '-=' operator for subtracting two 4-vectors.
 * Same logic as the subtraction operator.
 */
v4 v4::operator-=( const v4& other ) {
  v[ 0 ] = v[ 0 ] - other.v[ 0 ];
  v[ 1 ] = v[ 1 ] - other.v[ 1 ];
  v[ 2 ] = v[ 2 ] - other.v[ 2 ];
  v[ 3 ] = v[ 3 ] - other.v[ 3 ];
  return *this;
}

/**
 * '-=' operator for subtracting a scalar from a 4-vector.
 * Same logic as the subtraction operator.
 */
v4 v4::operator-=( float scalar ) {
  v[ 0 ] = v[ 0 ] - scalar;
  v[ 1 ] = v[ 1 ] - scalar;
  v[ 2 ] = v[ 2 ] - scalar;
  v[ 3 ] = v[ 3 ] - scalar;
  return *this;
}

/**
 * Addition operator for two 4-vectors.
 * Add each element individually.
 */
v4 v4::operator+( const v4& other ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] + other.v[ 0 ];
  vec.v[ 1 ] = v[ 1 ] + other.v[ 1 ];
  vec.v[ 2 ] = v[ 2 ] + other.v[ 2 ];
  vec.v[ 3 ] = v[ 3 ] + other.v[ 3 ];
  return vec;
}

/** Addition operator for a 4-vector and a scalar value. */
v4 v4::operator+( float scalar ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] + scalar;
  vec.v[ 1 ] = v[ 1 ] + scalar;
  vec.v[ 2 ] = v[ 2 ] + scalar;
  vec.v[ 3 ] = v[ 3 ] + scalar;
  return vec;
}

/**
 * '+=' operator for two 4-vectors.
 * Same logic as the addition operator.
 */
v4 v4::operator+=( const v4& other ) {
  v[ 0 ] = v[ 0 ] + other.v[ 0 ];
  v[ 1 ] = v[ 1 ] + other.v[ 1 ];
  v[ 2 ] = v[ 2 ] + other.v[ 2 ];
  v[ 3 ] = v[ 3 ] + other.v[ 3 ];
  return *this;
}

/**
 * '+=' operator for a 4-vector and a scalar value.
 * Same logic as the addition operator.
 */
v4 v4::operator+=( float scalar ) {
  v[ 0 ] = v[ 0 ] + scalar;
  v[ 1 ] = v[ 1 ] + scalar;
  v[ 2 ] = v[ 2 ] + scalar;
  v[ 3 ] = v[ 3 ] + scalar;
  return *this;
}

/** Multiplication operator for a 4-vector and a scalar. */
v4 v4::operator*( float scalar ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] * scalar;
  vec.v[ 1 ] = v[ 1 ] * scalar;
  vec.v[ 2 ] = v[ 2 ] * scalar;
  vec.v[ 3 ] = v[ 3 ] * scalar;
  return vec;
}

/**
 * '*=' operator for a 4-vector and a scalar.
 * Same logic as the multiplication operator.
 */
v4& v4::operator*=( float scalar ) {
  v[ 0 ] = v[ 0 ] * scalar;
  v[ 1 ] = v[ 1 ] * scalar;
  v[ 2 ] = v[ 2 ] * scalar;
  v[ 3 ] = v[ 3 ] * scalar;
  return *this;
}

/** Division operator for a 4-vector and a scalar. */
v4 v4::operator/( float scalar ) {
  v4 vec;
  vec.v[ 0 ] = v[ 0 ] / scalar;
  vec.v[ 1 ] = v[ 1 ] / scalar;
  vec.v[ 2 ] = v[ 2 ] / scalar;
  vec.v[ 3 ] = v[ 3 ] / scalar;
  return vec;
}

/**
 * '/=' operator for a 4-vector and a scalar.
 * Same logic as the division operator.
 */
v4& v4::operator/=( float scalar ) {
  v[ 0 ] = v[ 0 ] / scalar;
  v[ 1 ] = v[ 1 ] / scalar;
  v[ 2 ] = v[ 2 ] / scalar;
  v[ 3 ] = v[ 3 ] / scalar;
  return *this;
}

/**
 * Default (empty) 4x4 matrix constructor.
 * Initialize each element to 0.
 */
m4::m4() {
  m[ 0 ] = m[ 1 ] = m[ 2 ] = m[ 3 ] = 0;
  m[ 4 ] = m[ 5 ] = m[ 6 ] = m[ 7 ] = 0;
  m[ 8 ] = m[ 9 ] = m[ 10 ] = m[ 11 ] = 0;
  m[ 12 ] = m[ 13 ] = m[ 14 ] = m[ 15 ] = 0;
}

/** Initialize a 4x4 matrix given 16 float values. */
m4::m4( float a, float b, float c, float d,
        float e, float f, float g, float h,
        float i, float j, float k, float l,
        float n, float o, float p, float q ) {
  m[ 0 ]  = a;
  m[ 1 ]  = b;
  m[ 2 ]  = c;
  m[ 3 ]  = d;
  m[ 4 ]  = e;
  m[ 5 ]  = f;
  m[ 6 ]  = g;
  m[ 7 ]  = h;
  m[ 8 ]  = i;
  m[ 9 ]  = j;
  m[ 10 ] = k;
  m[ 11 ] = l;
  m[ 12 ] = n;
  m[ 13 ] = o;
  m[ 14 ] = p;
  m[ 15 ] = q;
}

/**
 * Multiplication operator for a 4x4 matrix and a 4-vector.
 * Performs matrix multiplication and return a 4-vector.
 */
v4 m4::operator*(const v4& other) {
  v4 vec;
  vec.v[ 0 ] = m[ 0 ]  * other.v[ 0 ] +
               m[ 1 ]  * other.v[ 1 ] +
               m[ 2 ]  * other.v[ 2 ] +
               m[ 3 ]  * other.v[ 3 ];
  vec.v[ 1 ] = m[ 4 ]  * other.v[ 0 ] +
               m[ 5 ]  * other.v[ 1 ] +
               m[ 6 ]  * other.v[ 2 ] +
               m[ 7 ]  * other.v[ 3 ];
  vec.v[ 2 ] = m[ 8 ]  * other.v[ 0 ] +
               m[ 9 ]  * other.v[ 1 ] +
               m[ 10 ] * other.v[ 2 ] +
               m[ 11 ] * other.v[ 3 ];
  vec.v[ 3 ] = m[ 12 ] * other.v[ 0 ] +
               m[ 13 ] * other.v[ 1 ] +
               m[ 14 ] * other.v[ 2 ] +
               m[ 15 ] * other.v[ 3 ];
  return vec;
}

/** Multiplication operator for two 4x4 matrices. */
m4 m4::operator*( const m4& other ) {
  m4 mat;
  // M1 row X * M2 column Y = M3[X, Y]
  for ( int row = 0; row < 4; row++ ) {
    for ( int column = 0; column < 4; column++ ) {
      int row_base = row * 4;
      int index = row_base + column;
      float result = 0;
      for ( int subindex = 0; subindex < 4; subindex ++ ) {
        result += ( m[ row_base+subindex ] *
                    other.m[ column + ( subindex * 4 ) ] );
      }
      mat.m[ index ] = result;
    }
  }
  return mat;
}

/** Multiplication operator for a 4x4 matrix and a scalar */
m4 m4::operator*( const float scalar ) {
  m4 mat;
  for ( int i = 0; i < 16; ++i ) {
    mat.m[ i ] = m[ i ] * scalar;
  }
  return mat;
}

/**
 * '=' operator to set one 4x4 matrix's contents to
 * match another 4x4 matrix.
 */
m4& m4::operator=( const m4& other ) {
  m[ 0 ]  = other.m[ 0 ];
  m[ 1 ]  = other.m[ 1 ];
  m[ 2 ]  = other.m[ 2 ];
  m[ 3 ]  = other.m[ 3 ];
  m[ 4 ]  = other.m[ 4 ];
  m[ 5 ]  = other.m[ 5 ];
  m[ 6 ]  = other.m[ 6 ];
  m[ 7 ]  = other.m[ 7 ];
  m[ 8 ]  = other.m[ 8 ];
  m[ 9 ]  = other.m[ 9 ];
  m[ 10 ] = other.m[ 10 ];
  m[ 11 ] = other.m[ 11 ];
  m[ 12 ] = other.m[ 12 ];
  m[ 13 ] = other.m[ 13 ];
  m[ 14 ] = other.m[ 14 ];
  m[ 15 ] = other.m[ 15 ];
  return *this;
}

/** Helper method to get one row of a 4x4 matrix as a 4-vector. */
v4 m4::row( const int row ) {
  v4 vec;
  int rowInd = row * 4;
  vec.v[ 0 ] = m[ rowInd ];
  vec.v[ 1 ] = m[ rowInd + 1 ];
  vec.v[ 2 ] = m[ rowInd + 2 ];
  vec.v[ 3 ] = m[ rowInd + 3 ];
  return vec;
}

/** Helper method to get one column of a 4x4 matrix as a 4-vector. */
v4 m4::col( const int col ) {
  v4 vec;
  vec.v[ 0 ] = m[ col ];
  vec.v[ 1 ] = m[ col + 4 ];
  vec.v[ 2 ] = m[ col + 8 ];
  vec.v[ 3 ] = m[ col + 12 ];
  return vec;
}

/**
 * Helper method to get the 'translation' 3-vector from a
 * 4x4 transformation matrix. This vector represents a
 * transform's X / Y / Z position in 3D space.
 */
v3 m4::translation() { return v3( m[ 3 ], m[ 7 ], m[ 11 ] ); }

/**
 * Helper method to get the X-coordinate from
 * a 4x4 transformation matrix.
 */
float m4::t_x() { return m[ 3 ]; }

/**
 * Helper method to get the Y-coordinate from
 * a 4x4 transformation matrix.
 */
float m4::t_y() { return m[ 7 ]; }

/**
 * Helper method to get the Z-coordinate from
 * a 4x4 transformation matrix.
 */
float m4::t_z() { return m[ 11 ]; }

/**
 * Default (empty) quaternion constructor. Default to
 * "0 degrees around the X-axis" (I think.)
 */
quat::quat() {
  q[ 0 ] = q[ 2 ] = q[ 3 ] = 0;
  q[ 1 ] = 1;
  r[ 0 ] = r[ 2 ] = r[ 3 ] = 0;
  r[ 1 ] = 1;
}

/**
 * Constructor to create a quaternion from a given
 * "axis / angle" representation. This includes an
 * angle to rotate about an axis, and the X / Y / Z
 * deltas which define that axis.
 */
quat::quat( float a, float x, float y, float z ) {
  float rads = ang_to_rad( a );
  q[ 0 ] = cos( rads/2 );
  q[ 1 ] = sin( rads/2 ) * x;
  q[ 2 ] = sin( rads/2 ) * y;
  q[ 3 ] = sin( rads/2 ) * z;
  r[ 0 ] = rads;
  r[ 1 ] = x;
  r[ 2 ] = y;
  r[ 3 ] = z;
}

/**
 * Update a quaternion's "axis-angle" shadow values to match
 * its calculated values.
 */
void quat::r_u() {
  r[ 0 ] = acos( q[ 0 ] ) * 2;
  float denom = sqrt( 1.0f - ( q[ 0 ] * q[ 0 ] ) );
  if ( denom == 0 ) {
    r[ 1 ] = 0.0f;
    r[ 2 ] = 1.0f;
    r[ 3 ] = 0.0f;
  }
  else {
    r[ 1 ] = q[ 1 ] / denom;
    r[ 2 ] = q[ 2 ] / denom;
    r[ 3 ] = q[ 3 ] / denom;
  }
}

/**
 * Division operator for a quaternion and a scalar.
 * TODO: Is this used? Does it make sense?
 */
quat quat::operator/( float scalar ) {
  quat result;
  result.q[ 0 ] = q[ 0 ] / scalar;
  result.q[ 1 ] = q[ 1 ] / scalar;
  result.q[ 2 ] = q[ 2 ] / scalar;
  result.q[ 3 ] = q[ 3 ] / scalar;
  result.r_u();
  return result;
}

/**
 * '/=' operator for a quaternion and a scalar.
 * Same logic as the division operator.
 */
quat& quat::operator/=( float scalar ) {
  q[ 0 ] = q[ 0 ] / scalar;
  q[ 1 ] = q[ 1 ] / scalar;
  q[ 2 ] = q[ 2 ] / scalar;
  q[ 3 ] = q[ 3 ] / scalar;
  r_u();
  return *this;
}

/**
 * Quaternion multiplication. Multiplying X * Y means
 * to rotate by Y before rotating by X.
 */
quat quat::operator*( quat o ) {
  quat result;
  result.q[ 0 ] = q[ 0 ] * o.q[ 0 ] - q[ 1 ] * o.q[ 1 ] -
                  q[ 2 ] * o.q[ 2 ] - q[ 3 ] * o.q[ 3 ];
  result.q[ 1 ] = q[ 1 ] * o.q[ 0 ] + q[ 0 ] * o.q[ 1 ] -
                  q[ 3 ] * o.q[ 2 ] + q[ 2 ] * o.q[ 3 ];
  result.q[ 2 ] = q[ 2 ] * o.q[ 0 ] + q[ 3 ] * o.q[ 1 ] +
                  q[ 0 ] * o.q[ 2 ] - q[ 1 ] * o.q[ 3 ];
  result.q[ 3 ] = q[ 3 ] * o.q[ 0 ] - q[ 2 ] * o.q[ 1 ] +
                  q[ 1 ] * o.q[ 2 ] + q[ 0 ] * o.q[ 3 ];
  result.r_u();
  return result;
}

/** Quaternion '*=' operator. Same logic as multiplication. */
quat& quat::operator*=( quat& o ) {
  q[ 0 ] = q[ 0 ] * o.q[ 0 ] - q[ 1 ] * o.q[ 1 ] -
           q[ 2 ] * o.q[ 2 ] - q[ 3 ] * o.q[ 3 ];
  q[ 1 ] = q[ 1 ] * o.q[ 0 ] + q[ 0 ] * o.q[ 1 ] -
           q[ 3 ] * o.q[ 2 ] + q[ 2 ] * o.q[ 3 ];
  q[ 2 ] = q[ 2 ] * o.q[ 0 ] + q[ 3 ] * o.q[ 1 ] +
           q[ 0 ] * o.q[ 2 ] - q[ 1 ] * o.q[ 3 ];
  q[ 3 ] = q[ 3 ] * o.q[ 0 ] - q[ 2 ] * o.q[ 1 ] +
           q[ 1 ] * o.q[ 2 ] + q[ 0 ] * o.q[ 3 ];
  r_u();
  return *this;
}

/** '=' operator to set one quaternion's values to match another's. */
quat& quat::operator=( const quat& other ) {
  q[ 0 ] = other.q[ 0 ];
  q[ 1 ] = other.q[ 1 ];
  q[ 2 ] = other.q[ 2 ];
  q[ 3 ] = other.q[ 3 ];
  r[ 0 ] = other.r[ 0 ];
  r[ 1 ] = other.r[ 1 ];
  r[ 2 ] = other.r[ 2 ];
  r[ 3 ] = other.r[ 3 ];
  return *this;
}

/**
 * Default (empty) axis-aligned bounding box constructor.
 * Set width / height / depth to 0.
 */
aabb::aabb() {
  x_w = 0;
  y_h = 0;
  z_d = 0;
}

/**
 * Axis-aligned bounding box constructor,
 * given a width / height / depth.
 */
aabb::aabb( float x_width, float y_height, float z_depth ) {
  x_w = x_width;
  y_h = y_height;
  z_d = z_depth;
}

/** Print a 3-vector to a string. */
string print( v3 vec ) {
  return "[" + to_string( vec.v[ 0 ] ) +
         ", " + to_string( vec.v[ 1 ] ) +
         ", " + to_string( vec.v[ 2 ] ) + "]";
}

/** Print a 4-vector to a string. */
string print( v4 vec ) {
  return "[" + to_string( vec.v[ 0 ] ) +
         ", " + to_string( vec.v[ 1 ] ) +
         ", " + to_string( vec.v[ 2 ] ) +
         ", " + to_string( vec.v[ 3 ] ) + "]";
}

/** Print a 4x4 matrix to a string. */
string print( m4 mat ) {
  return "[" + to_string( mat.m[ 0 ] ) +
         ", " + to_string( mat.m[ 1 ] ) +
         ", " + to_string( mat.m[ 2 ] ) +
         ", " + to_string( mat.m[ 3 ] ) +
         "\n " + to_string( mat.m[ 4 ] ) +
         ", " + to_string( mat.m[ 5 ] ) +
         ", " + to_string( mat.m[ 6 ] ) +
         ", " + to_string( mat.m[ 7 ] ) +
         "\n " + to_string( mat.m[ 8 ] ) +
         ", " + to_string( mat.m[ 9 ] ) +
         ", " + to_string( mat.m[ 10 ] ) +
         ", " + to_string( mat.m[ 11 ] ) +
         "\n " + to_string( mat.m[ 12 ] ) +
         ", " + to_string( mat.m[ 13 ] ) +
         ", " + to_string( mat.m[ 14 ] ) +
         ", " + to_string( mat.m[ 15 ] ) + "]";
}

/** Print a quaternion to a string. */
string print( quat q ) {
  return "[" + to_string( q.q[ 0 ] ) +
         ", " + to_string( q.q[ 1 ] ) +
         ", " + to_string( q.q[ 2 ] ) +
         ", " + to_string( q.q[ 3 ] ) + "]";
}

/** Print an axis-aligned bounding box to a string. */
string print( aabb b ) {
  return "[" + to_string( b.x_w ) +
               ", " + to_string( b.y_h ) +
               ", " + to_string( b.z_d ) + "]";
}

/** Helper function to convert angles to radians. */
float ang_to_rad( float a ) { return a * ( PI / 180 ); }

/** Helper function to convert radians to angles. */
float rad_to_ang( float r ) { return r * ( 180 / PI ); }

/** Calculate the magnitude ("length") of a 3-vector. */
float magnitude( v3& vec ) {
  return ( sqrt( vec.v[ 0 ] * vec.v[ 0 ] +
                 vec.v[ 1 ] * vec.v[ 1 ] +
                 vec.v[ 2 ] * vec.v[ 2 ] ) );
}

/**
 * Calculate the square of a 3-vector's magnitude.
 * This is cheaper than calculating the magnitude, when you
 * only need to make comparisons between two vectors.
 */
float magnitude2( v3 vec ) {
  return ( vec.v[ 0 ] * vec.v[ 0 ] +
           vec.v[ 1 ] * vec.v[ 1 ] +
           vec.v[ 2 ] * vec.v[ 2 ] );
}

/**
 * Calculate a normalized version of a 3-vector.
 * Normalized vectors have a magnitude of 1. You might call them rays.
 */
v3 normalize( v3 vec ) {
  v3 norm = vec / magnitude( vec );
  return norm;
}

/** Calculate the cross product of two 3-vectors. */
v3 cross( v3& a, v3& b ) {
  v3 vec;
  vec.v[ 0 ] = ( a.v[ 1 ] * b.v[ 2 ] ) - ( a.v[ 2 ] * b.v[ 1 ] );
  vec.v[ 1 ] = ( a.v[ 2 ] * b.v[ 0 ] ) - ( a.v[ 0 ] * b.v[ 2 ] );
  vec.v[ 2 ] = ( a.v[ 0 ] * b.v[ 1 ] ) - ( a.v[ 1 ] * b.v[ 0 ] );
  return vec;
}

/** Calculate the dot product of two 3-vectors. */
float dot( v3& a, v3& b ) {
  return a.v[ 0 ] * b.v[ 0 ] +
         a.v[ 1 ] * b.v[ 1 ] +
         a.v[ 2 ] * b.v[ 2 ];
}

/** Weighted linear interpolation between two 3-vectors by a factor. */
v3 lerp( v3& vec1, v3& vec2, float prog ) {
  return vec1 + ( vec2 - vec1 ) * prog;
}

/** Find the distance between two points represented by 3-vectors. */
float distance( v3 vec1, v3 vec2 ) {
  v3 dif = vec2 - vec1;
  return magnitude( dif );
}

/**
 * Find the squared distance between two points represented by
 * 3-vectors. Again, this is faster if you only need to compare.
 */
float distance2( v3 vec1, v3 vec2 ) {
  v3 dif = vec2 - vec1;
  return magnitude2( dif );
}

/** Find the determinant of a 4x4 matrix. */
float determinant( m4 mat ) {
  return (
    mat.m[ 0 ] * mat.m[ 5 ] * mat.m[ 10 ] * mat.m[ 15 ] +
    mat.m[ 0 ] * mat.m[ 6 ] * mat.m[ 11 ] * mat.m[ 13 ] +
    mat.m[ 0 ] * mat.m[ 7 ] * mat.m[ 9 ] * mat.m[ 14 ] +
    mat.m[ 1 ] * mat.m[ 4 ] * mat.m[ 11 ] * mat.m[ 14 ] +
    mat.m[ 1 ] * mat.m[ 6 ] * mat.m[ 8 ] * mat.m[ 15 ] +
    mat.m[ 1 ] * mat.m[ 7 ] * mat.m[ 10 ] * mat.m[ 12 ] +
    mat.m[ 2 ] * mat.m[ 4 ] * mat.m[ 9 ] * mat.m[ 15 ] +
    mat.m[ 2 ] * mat.m[ 5 ] * mat.m[ 11 ] * mat.m[ 12 ] +
    mat.m[ 2 ] * mat.m[ 7 ] * mat.m[ 8 ] * mat.m[ 13 ] +
    mat.m[ 3 ] * mat.m[ 4 ] * mat.m[ 10 ] * mat.m[ 13 ] +
    mat.m[ 3 ] * mat.m[ 5 ] * mat.m[ 8 ] * mat.m[ 14 ] +
    mat.m[ 3 ] * mat.m[ 6 ] * mat.m[ 9 ] * mat.m[ 12 ] -
    mat.m[ 0 ] * mat.m[ 5 ] * mat.m[ 11 ] * mat.m[ 14 ] -
    mat.m[ 0 ] * mat.m[ 6 ] * mat.m[ 9 ] * mat.m[ 15 ] -
    mat.m[ 0 ] * mat.m[ 7 ] * mat.m[ 10 ] * mat.m[ 13 ] -
    mat.m[ 1 ] * mat.m[ 4 ] * mat.m[ 10 ] * mat.m[ 15 ] -
    mat.m[ 1 ] * mat.m[ 6 ] * mat.m[ 11 ] * mat.m[ 12 ] -
    mat.m[ 1 ] * mat.m[ 7 ] * mat.m[ 8 ] * mat.m[ 14 ] -
    mat.m[ 2 ] * mat.m[ 4 ] * mat.m[ 11 ] * mat.m[ 13 ] -
    mat.m[ 2 ] * mat.m[ 5 ] * mat.m[ 8 ] * mat.m[ 15 ] -
    mat.m[ 2 ] * mat.m[ 7 ] * mat.m[ 9 ] * mat.m[ 12 ] -
    mat.m[ 3 ] * mat.m[ 4 ] * mat.m[ 9 ] * mat.m[ 14 ] -
    mat.m[ 3 ] * mat.m[ 5 ] * mat.m[ 10 ] * mat.m[ 12 ] -
    mat.m[ 3 ] * mat.m[ 6 ] * mat.m[ 8 ] * mat.m[ 13 ] );
}

/** Return the identity 4x4 matrix. */
m4 id4() {
  return m4( 1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 );
}

/**
 * Invert a 4x4 matrix. Note that thsi is an expensive operation,
 * and I'm using an implementation that I found on StackOverflow.
 */
m4 inverse( m4 orig ) {
  m4 base_inverse = m4();
  base_inverse.m[ 0 ] = orig.m[ 5 ] * orig.m[ 10 ] * orig.m[ 15 ] -
            orig.m[ 5 ] * orig.m[ 11 ] * orig.m[ 14 ] -
            orig.m[ 9 ] * orig.m[ 6 ] * orig.m[ 15 ] +
            orig.m[ 9 ] * orig.m[ 7 ] * orig.m[ 14 ] +
            orig.m[ 13 ] * orig.m[ 6 ] * orig.m[ 11 ] -
            orig.m[ 13 ] * orig.m[ 7 ] * orig.m[ 10 ];
  base_inverse.m[ 4 ] = -orig.m[ 4 ] * orig.m[ 10 ] * orig.m[ 15 ] +
            orig.m[ 4 ] * orig.m[ 11 ] * orig.m[ 14 ] +
            orig.m[ 8 ] * orig.m[ 6 ] * orig.m[ 15 ] -
            orig.m[ 8 ] * orig.m[ 7 ] * orig.m[ 14 ] -
            orig.m[ 12 ] * orig.m[ 6 ] * orig.m[ 11 ] +
            orig.m[ 12 ] * orig.m[ 7 ] * orig.m[ 10 ];
  base_inverse.m[ 8 ] = orig.m[ 4 ] * orig.m[ 9 ] * orig.m[ 15 ] -
            orig.m[ 4 ] * orig.m[ 11 ] * orig.m[ 13 ] -
            orig.m[ 8 ] * orig.m[ 5 ] * orig.m[ 15 ] +
            orig.m[ 8 ] * orig.m[ 7 ] * orig.m[ 13 ] +
            orig.m[ 12 ] * orig.m[ 5 ] * orig.m[ 11 ] -
            orig.m[ 12 ] * orig.m[ 7 ] * orig.m[ 9 ];
  base_inverse.m[ 12 ] = -orig.m[ 4 ] * orig.m[ 9 ] * orig.m[ 14 ] +
            orig.m[ 4 ] * orig.m[ 10 ] * orig.m[ 13 ] +
            orig.m[ 8 ] * orig.m[ 5 ] * orig.m[ 14 ] -
            orig.m[ 8 ] * orig.m[ 6 ] * orig.m[ 13 ] -
            orig.m[ 12 ] * orig.m[ 5 ] * orig.m[ 10 ] +
            orig.m[ 12 ] * orig.m[ 6 ] * orig.m[ 9 ];
  base_inverse.m[ 1 ] = -orig.m[ 1 ] * orig.m[ 10 ] * orig.m[ 15 ] +
            orig.m[ 1 ] * orig.m[ 11 ] * orig.m[ 14 ] +
            orig.m[ 9 ] * orig.m[ 2 ] * orig.m[ 15 ] -
            orig.m[ 9 ] * orig.m[ 3 ] * orig.m[ 14 ] -
            orig.m[ 13 ] * orig.m[ 2 ] * orig.m[ 11 ] +
            orig.m[ 13 ] * orig.m[ 3 ] * orig.m[ 10 ];
  base_inverse.m[ 5 ] = orig.m[ 0 ] * orig.m[ 10 ] * orig.m[ 15 ] -
            orig.m[ 0 ] * orig.m[ 11 ] * orig.m[ 14 ] -
            orig.m[ 8 ] * orig.m[ 2 ] * orig.m[ 15 ] + 
            orig.m[ 8 ] * orig.m[ 3 ] * orig.m[ 14 ] +
            orig.m[ 12 ] * orig.m[ 2 ] * orig.m[ 11 ] -
            orig.m[ 12 ] * orig.m[ 3 ] * orig.m[ 10 ];
  base_inverse.m[ 9 ] = -orig.m[ 0 ] * orig.m[ 9 ] * orig.m[ 15 ] +
            orig.m[ 0 ] * orig.m[ 11 ] * orig.m[ 13 ] +
            orig.m[ 8 ] * orig.m[ 1 ] * orig.m[ 15 ] -
            orig.m[ 8 ] * orig.m[ 3 ] * orig.m[ 13 ] -
            orig.m[ 12 ] * orig.m[ 1 ] * orig.m[ 11 ] +
            orig.m[ 12 ] * orig.m[ 3 ] * orig.m[ 9 ];
  base_inverse.m[ 13 ] = orig.m[ 0 ] * orig.m[ 9 ] * orig.m[ 14 ] -
            orig.m[ 0 ] * orig.m[ 10 ] * orig.m[ 13 ] -
            orig.m[ 8 ] * orig.m[ 1 ] * orig.m[ 14 ] +
            orig.m[ 8 ] * orig.m[ 2 ] * orig.m[ 13 ] +
            orig.m[ 12 ] * orig.m[ 1 ] * orig.m[ 10 ] -
            orig.m[  12  ] * orig.m[  2  ] * orig.m[  9  ];
  base_inverse.m[ 2 ] = orig.m[ 1 ] * orig.m[ 6 ] * orig.m[ 15 ] -
            orig.m[ 1 ] * orig.m[ 7 ] * orig.m[ 14 ] -
            orig.m[ 5 ] * orig.m[ 2 ] * orig.m[ 15 ] +
            orig.m[ 5 ] * orig.m[ 3 ] * orig.m[ 14 ] +
            orig.m[ 13 ] * orig.m[ 2 ] * orig.m[ 7 ] -
            orig.m[ 13 ] * orig.m[ 3 ] * orig.m[ 6 ];
  base_inverse.m[ 6 ] = -orig.m[ 0 ] * orig.m[ 6 ] * orig.m[ 15 ] +
            orig.m[ 0 ] * orig.m[ 7 ] * orig.m[ 14 ] +
            orig.m[ 4 ] * orig.m[ 2 ] * orig.m[ 15 ] -
            orig.m[ 4 ] * orig.m[ 3 ] * orig.m[ 14 ] -
            orig.m[ 12 ] * orig.m[ 2 ] * orig.m[ 7 ] +
            orig.m[ 12 ] * orig.m[ 3 ] * orig.m[ 6 ];
  base_inverse.m[ 10 ] = orig.m[ 0 ] * orig.m[ 5 ] * orig.m[ 15 ] -
            orig.m[ 0 ] * orig.m[ 7 ] * orig.m[ 13 ] -
            orig.m[ 4 ] * orig.m[ 1 ] * orig.m[ 15 ] +
            orig.m[ 4 ] * orig.m[ 3 ] * orig.m[ 13 ] +
            orig.m[ 12 ] * orig.m[ 1 ] * orig.m[ 7 ] -
            orig.m[ 12 ] * orig.m[ 3 ] * orig.m[ 5 ];
  base_inverse.m[ 14 ] = -orig.m[ 0 ] * orig.m[ 5 ] * orig.m[ 14 ] +
            orig.m[ 0 ] * orig.m[ 6 ] * orig.m[ 13 ] +
            orig.m[ 4 ] * orig.m[ 1 ] * orig.m[ 14 ] -
            orig.m[ 4 ] * orig.m[ 2 ] * orig.m[ 13 ] -
            orig.m[ 12 ] * orig.m[ 1 ] * orig.m[ 6 ] +
            orig.m[ 12 ] * orig.m[ 2 ] * orig.m[ 5 ];
  base_inverse.m[ 3 ] = -orig.m[ 1 ] * orig.m[ 6 ] * orig.m[ 11 ] +
            orig.m[ 1 ] * orig.m[ 7 ] * orig.m[ 10 ] +
            orig.m[ 5 ] * orig.m[ 2 ] * orig.m[ 11 ] -
            orig.m[ 5 ] * orig.m[ 3 ] * orig.m[ 10 ] -
            orig.m[ 9 ] * orig.m[ 2 ] * orig.m[ 7 ] +
            orig.m[ 9 ] * orig.m[ 3 ] * orig.m[ 6 ];
  base_inverse.m[ 7 ] = orig.m[ 0 ] * orig.m[ 6 ] * orig.m[ 11 ] -
            orig.m[ 0 ] * orig.m[ 7 ] * orig.m[ 10 ] -
            orig.m[ 4 ] * orig.m[ 2 ] * orig.m[ 11 ] +
            orig.m[ 4 ] * orig.m[ 3 ] * orig.m[ 10 ] +
            orig.m[ 8 ] * orig.m[ 2 ] * orig.m[ 7 ] -
            orig.m[ 8 ] * orig.m[ 3 ] * orig.m[ 6 ];
  base_inverse.m[ 11 ] = -orig.m[ 0 ] * orig.m[ 5 ] * orig.m[ 11 ] +
            orig.m[ 0 ] * orig.m[ 7 ] * orig.m[ 9 ] +
            orig.m[ 4 ] * orig.m[ 1 ] * orig.m[ 11 ] -
            orig.m[ 4 ] * orig.m[ 3 ] * orig.m[ 9 ] -
            orig.m[ 8 ] * orig.m[ 1 ] * orig.m[ 7 ] +
            orig.m[ 8 ] * orig.m[ 3 ] * orig.m[ 5 ];
  base_inverse.m[ 15 ] = orig.m[ 0 ] * orig.m[ 5 ] * orig.m[ 10 ] -
            orig.m[ 0 ] * orig.m[ 6 ] * orig.m[ 9 ] -
            orig.m[ 4 ] * orig.m[ 1 ] * orig.m[ 10 ] +
            orig.m[ 4 ] * orig.m[ 2 ] * orig.m[ 9 ] +
            orig.m[ 8 ] * orig.m[ 1 ] * orig.m[ 6 ] -
            orig.m[ 8 ] * orig.m[ 2 ] * orig.m[ 5 ];

  // Shortcut, since we already did all that calculation.
  float det =
    orig.m[ 0 ] * base_inverse.m[ 0 ] +
    orig.m[ 1 ] * base_inverse.m[ 4 ] +
    orig.m[ 2 ] * base_inverse.m[ 8 ] +
    orig.m[ 3 ] * base_inverse.m[ 12 ];
  if ( det == 0.0 ) {
    // No inverse if the determinant is 0; fallback to identity.
    // This may not be the best idea, but we'll see what happens.
    return id4();
  }

  m4 inverse;
  det = 1.0 / det;
  for ( int i = 0; i < 16; i++ ) {
    inverse.m[ i ] = base_inverse.m[ i ] * det;
  }

  return inverse;
}

/** Transpose a 4x4 matrix. */
m4 transpose( m4 orig ) {
  return m4( orig.m[ 0 ], orig.m[ 4 ], orig.m[ 8 ],  orig.m[ 12 ],
             orig.m[ 1 ], orig.m[ 5 ], orig.m[ 9 ],  orig.m[ 13 ],
             orig.m[ 2 ], orig.m[ 6 ], orig.m[ 10 ], orig.m[ 14 ],
             orig.m[ 3 ], orig.m[ 7 ], orig.m[ 11 ], orig.m[ 15 ] );
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of degrees around the X-axis.
 */
m4 rotate_x( float degrees ) {
  float rads = ang_to_rad( degrees );
  return rotate_x_rads( rads );
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of radians around the X-axis.
 */
m4 rotate_x_rads( float rads ) {
  m4 mat( 1, 0, 0, 0,
          0, cos( rads ), -sin( rads ), 0,
          0, sin( rads ), cos( rads ), 0,
          0, 0, 0, 1);
  return mat;
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of degrees around the Y-axis.
 */
m4 rotate_y( float degrees ) {
  float rads = ang_to_rad( degrees );
  return rotate_y_rads( rads );
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of radians around the Y-axis.
 */
m4 rotate_y_rads( float rads ) {
  m4 mat( cos( rads ), 0, sin( rads ), 0,
          0, 1, 0, 0,
          -sin( rads ), 0, cos( rads ), 0,
          0, 0, 0, 1);
  return mat;
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of degrees around the Z-axis.
 */
m4 rotate_z( float degrees ) {
  float rads = ang_to_rad( degrees );
  return rotate_z_rads( rads );
}

/**
 * Helper method to create a transformation matrix which rotates
 * a 3D object by a specified number of radians around the Z-axis.
 */
m4 rotate_z_rads( float rads ) {
  m4 mat( cos( rads ), -sin( rads ), 0, 0,
          sin( rads ), cos( rads ), 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1 );
   return mat;
}

/**
 * Helper method to create a translation matrix, which
 * moves an object in 3D space.
 */
m4 translation_matrix( float x, float y, float z ) {
  m4 translation( 1, 0, 0, x,
                  0, 1, 0, y,
                  0, 0, 1, z,
                  0, 0, 0, 1 );
  return translation;
}

/**
 * Helper method to create a translation matrix
 * from a 3-vector instead of 3 floats.
 */
m4 translation_matrix( v3 t ) {
  return translation_matrix( t.v[ 0 ], t.v[ 1 ], t.v[ 2 ] );
}

/**
 * Create the rotation part of a 4x4 transformation matrix
 * from a quaternion object.
 */
m4 quaternion_to_rotation( quat q ) {
  // Start with the identity matrix.
  m4 mat = id4();
  // 1 - 2y^2 - 2z^2
  mat.m[ 0 ] = 1 - 2 * q.q[ 2 ] * q.q[ 2 ] - 2 * q.q[ 3 ] * q.q[ 3 ];
  // 2xy - 2az
  mat.m[ 1 ]  = 2 * q.q[ 1 ] * q.q[ 2 ] - 2 * q.q[ 0 ] * q.q[ 3 ];
  // 2xz + 2ay
  mat.m[ 2 ]  = 2 * q.q[ 1 ] * q.q[ 3 ] + 2 * q.q[ 0 ] * q.q[ 2 ];
  // 2xy + 2az
  mat.m[ 4 ]  = 2 * q.q[ 1 ] * q.q[ 2 ] + 2 * q.q[ 0 ] * q.q[ 3 ];
  // 1 - 2x^2 - 2z^2
  mat.m[ 5 ]  = 1 - 2 * q.q[ 1 ] * q.q[ 1 ] - 2 * q.q[ 3 ] * q.q[ 3 ];
  // 2yz - 2ax
  mat.m[ 6 ]  = 2 * q.q[ 2 ] * q.q[ 3 ] - 2 * q.q[ 0 ] * q.q[ 1 ];
  // 2xz - 2ay
  mat.m[ 8 ]  = 2 * q.q[ 1 ] * q.q[ 3 ] - 2 * q.q[ 0 ] * q.q[ 2 ];
  // 2yz + 2ax
  mat.m[ 9 ]  = 2 * q.q[ 2 ] * q.q[ 3 ] + 2 * q.q[ 0 ] * q.q[ 1 ];
  // 1 - 2x^2 - 2y^2
  mat.m[ 10 ] = 1 - 2 * q.q[ 1 ] * q.q[ 1 ] - 2 * q.q[ 2 ] * q.q[ 2 ];
  // Done; return the resulting matrix.
  return mat;
}

/**
 * Helper method to create a combined 'view matrix', given
 * separate translation and rotation matrices.
 * The logic is simply: V = R * T
 */
m4 view_matrix( m4 translation, m4 rotation ) {
  return rotation * translation;
}

/**
 * Create a perspective matrix, given the current display window's
 * aspect ratio, the desired vertical field of vision in degrees,
 * and the desired distance to its near and far clipping planes.
 * Canonically, the perspective matrix looks like this:
 * X,  0,  0,  0,
 * 0,  Y,  0,  0,
 * 0,  0,  Z,  P,
 * 0,  0, -1,  0
 * With each value being calculated as:
 *   X = 2 * ( near / ( range * aspect_ratio ) )
 *   Y = near / range
 *   Z = -( far + near ) / ( far - near )
 *   P = -( 2 * far * near ) / ( far - near )
 * And the intermediary 'range' value is calculated as:
 *   range = ( near * tan( vert_fov_radians / 2 ) )
 * TODO: Make the above look better in Doxygen.
 */
m4 perspective( float near,
                float far,
                float fov,
                float aspect_ratio ) {
  // Convert field of vision degrees to radians.
  float fov_rads = ang_to_rad( fov );
  // Calculate raw perspective values.
  float range = near * tan( fov_rads * 0.5 );
  float PX = 2 * ( near / ( range * aspect_ratio ) );
  float PY = near / range;
  float PZ = -( far + near ) / ( far - near );
  float P = -( 2 * far * near ) / ( far - near );

  // Assemble and return the perspective matrix.
  m4 persp_mat( PX, 0, 0, 0,
                0, PY, 0, 0,
                0, 0, PZ, P,
                0, 0, -1, 0);
  return persp_mat;
}

/**
 * Calculate the dot product of two quaternions.
 */
float dot( quat q1, quat q2 ) {
  return q1.q[0]*q2.q[0] + q1.q[1]*q2.q[1] + q1.q[2]*q2.q[2] + q1.q[3]*q2.q[3];
}

/**
 * Calculate the 'normal' of a quaternion. I don't quite
 * understand this math, but this is calculated by summing
 * the square of each element, and it is used to calculate
 * a quaternion's inverse. TODO: I don't think these more
 * theoretical math methods are actually used; maybe delete?
 */
float norm( quat q ) {
  return ( q.q[ 0 ] * q.q[ 0 ] +
           q.q[ 1 ] * q.q[ 1 ] +
           q.q[ 2 ] * q.q[ 2 ] +
           q.q[ 3 ] * q.q[ 3 ] );
}

/**
 * Set a quaternion's values based on an axis represented by
 * a 3-vector, and an angle (in degrees) to rotate around that axis.
 * This uses the same basic logic as the quaternion's constructor.
 */
quat set_quat( float a, v3 vec ) {
  quat q;
  float rads = ang_to_rad( a );
  q.q[ 0 ] = cos( rads / 2 );
  q.q[ 1 ] = sin( rads / 2 ) * vec.v[ 0 ];
  q.q[ 2 ] = sin( rads / 2 ) * vec.v[ 1 ];
  q.q[ 3 ] = sin( rads / 2 ) * vec.v[ 2 ];
  q.r[ 0 ] = a;
  q.r[ 1 ] = vec.v[ 0 ];
  q.r[ 2 ] = vec.v[ 1 ];
  q.r[ 3 ] = vec.v[ 2 ];
  return q;
}

/**
 * Set a quaternion's values based on an axis represented by
 * X / Y / Z values, and an angle (in degrees) to rotate around that
 * axis. This uses the same logic as the quaternion's constructor.
 */
quat set_quat( float a, float x, float y, float z ) {
  quat q;
  float rads = ang_to_rad( a );
  q.q[ 0 ] = cos( rads / 2 );
  q.q[ 1 ] = sin( rads / 2 ) * x;
  q.q[ 2 ] = sin( rads / 2 ) * y;
  q.q[ 3 ] = sin( rads / 2 ) * z;
  q.r[ 0 ] = a;
  q.r[ 1 ] = x;
  q.r[ 2 ] = y;
  q.r[ 3 ] = z;
  return q;
}

/**
 * Helper method to 'normalize' a quaternion by setting its 'normal'
 * or 'length' to the unit value of 1. TODO: I don't think this
 * method makes much geometric sense: delete it?
 */
quat normalize( quat q ) {
  float len = ( q.q[ 0 ] * q.q[ 0 ] +
                q.q[ 1 ] * q.q[ 1 ] +
                q.q[ 2 ] * q.q[ 2 ] +
                q.q[ 3 ] * q.q[ 3 ] );
  // Fuzzy matching for float values; match if it is close to 1.00f
  if ( fabs( len ) >= 1.0f && fabs( len ) <= 1.001f ) {
    return q;
  }
  // Return the quaternion divided by its scalar length.
  return q / sqrt( len );
}

/**
 * Helper method to calculate the 'conjugate' of a quaternion.
 * TODO: I forget what this method does, or at least how to
 * think about it in 3D space. Is it used? Can I delete it?
 */
quat conjugate( quat q ) {
  quat result( q.q[ 0 ], -q.q[ 1 ], -q.q[ 2 ], -q.q[ 3 ] );
  return result;
}

/**
 * Helper method to calculate the 'inverse' of a quaternion.
 * TODO: Again, I forget what the purpose of this method is.
 * Can / should it be deleted?
 */
quat inverse( quat q ) {
  return conjugate( q ) / norm( q );
}

/**
 * Helper method to perform spherical linear interpolation
 * TODO: I think this should work, but I don't have a use
 * for it yet and it is untested. And if I do end up using
 * it, I should comment it better.
 */
quat slerp( quat q1, quat q2, float prog ) {
  quat result;
  float d = dot( q1, q2 );

  // Negate one of the quats if we want to take the longer
  // rounded path.
  if ( d < 0.0f ) {
    for ( int i = 0; i < 4; i++ ) {
      q1.q[ i ] *= -1.0f;
    }
  }

  // If the two quaternions are roughly equal, return one of them.
  if ( fabs( d ) >= 1.0f && fabs( d ) <= 1.001f ) {
    return q1;
  }

  // Find the sin(arccos) without using expensive trig functions.
  float sin_o = sqrt( 1.0f - d * d );
  if ( fabs( sin_o ) < 0.001f ) {
    // Can't /0, so do linear interpolation.
    for ( int i = 0; i < 4; i++ ) {
      result.q[ i ] = ( 1.0f - prog ) * q1.q[ i ] + prog * q2.q[ i ];
    }
    return result;
  }

  float o = acos( d );
  float a = sin( ( 1.0f - prog ) * o ) / sin_o;
  float b = sin( prog * o ) / sin_o;
  for ( int i = 0; i < 4; i++ ) {
    result.q[ i ] = q1.q[ i ] * a + q2.q[ i ] * b;
  }
  return result;
}
